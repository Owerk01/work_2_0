from datetime import datetime
import spacy
import time, os
from data import DB, DB_NAME, DB_DIR, STATS_DB
from data import SentenceData, TokenData, TextData
from pydantic import ValidationError
import stanza
import json

ROOT_IDX = -1

class SQLhelper:
    def __init__(self) -> None:
        self.db = DB()
    
    def save_analysis(self, name: str, filename: str, sent_count: int) -> None:
        self.db.execute_query(
            f"INSERT INTO {DB_NAME} (name, filename, sentence_count) VALUES (?, ?, ?)",
            (name, filename, sent_count)
        )
    
    def get_all_analysis(self) -> list[tuple]:
        return self.db.select_query(f"SELECT * FROM {DB_NAME} ORDER BY id")
    
    def get_analysis_filename_by_id(self, id: int) -> str | None:
        result = self.db.select_query(f"SELECT filename FROM {DB_NAME} WHERE id = ?", (id,))
        return result[0][0] if result else None
    
    def get_text_name_by_id(self, id: int) -> str | None:
        result = self.db.select_query(f"SELECT name FROM {DB_NAME} WHERE id = ?", (id,))
        return result[0][0] if result else None    
    
    def delete_analysis_by_id(self, id: int) -> None:
        self.db.execute_query(f"DELETE FROM {DB_NAME} WHERE id = ?", (id,))
    
    def update_analysis_name(self, id: int, new_name: str) -> None:
        self.db.execute_query(
            f"UPDATE {DB_NAME} SET name = ? WHERE id = ?",
            (new_name, id)
        )
    
    def update_analysis_filename(self, id: int, new_filename: str) -> None:
        self.db.execute_query(
            f"UPDATE {DB_NAME} SET filename = ? WHERE id = ?",
            (new_filename, id)
        )
    
    def update_analysis_sentence_count(self, id: int, sent_count: int) -> None:
        self.db.execute_query(
            f"UPDATE {DB_NAME} SET sentence_count = ? WHERE id = ?",
            (sent_count, id)
        )
    
    def save_parsing_stat(self, word_count: int, duration: float) -> None:
        self.db.execute_query(
            f"INSERT INTO {STATS_DB} (word_count, duration) VALUES (?, ?)",
            (word_count, duration)
        )
    
    def get_all_stats(self) -> list[tuple]:
        return self.db.select_query(f"SELECT word_count, duration, timestamp FROM {STATS_DB} ORDER BY timestamp")


class DBManager:
    def __init__(self) -> None:
        self.sql = SQLhelper()
    
    def save_to_file(self, text: TextData, name: str) -> None:
        json_data = text.model_dump_json(indent=4, exclude_none=True)
        timestmp = datetime.now().isoformat().replace('-', '').replace(':', '').replace('.', '').replace('T', '')
        filename = f"analysis_{timestmp}.json"
        
        with open(f"{DB_DIR}/{filename}", "w", encoding="utf-8") as f:
            f.write(json_data)
        print(f"(?) Saved file: {filename}")
        
        self.sql.save_analysis(name, filename, len(text.sentences))
    
    def update_existing_analysis(self, text: TextData, id: int, old_filename: str, new_name: str) -> str:
        json_data = text.model_dump_json(indent=4, exclude_none=True)
        timestmp = datetime.now().isoformat().replace('-', '').replace(':', '').replace('.', '').replace('T', '')
        new_filename = f"analysis_{timestmp}.json"
        
        with open(f"{DB_DIR}/{new_filename}", "w", encoding="utf-8") as f:
            f.write(json_data)
        print(f"(?) Updated file: {new_filename}")
        
        old_filepath = os.path.join(DB_DIR, old_filename)
        if os.path.exists(old_filepath):
            os.remove(old_filepath)
            print(f"(?) Removed old file: {old_filename}")
        
        self.sql.update_analysis_name(id, new_name)
        self.sql.update_analysis_filename(id, new_filename)
        self.sql.update_analysis_sentence_count(id, len(text.sentences))
        
        return new_filename
    
    def load_from_file(self, filepath: str) -> TextData | None:
        try:
            with open(filepath, "r", encoding="utf-8") as f:
                json_content = f.read()
            data = TextData.model_validate_json(json_content)
            return data
        except ValidationError as e:
            print(f"(!) Incorrect file structure: {e}")
            return None
        except FileNotFoundError:
            print("(!) File not found")
            return None
    
    def get_analysis_text(self, id: int) -> str | None:
        filename = self.sql.get_analysis_filename_by_id(id)
        if filename != None:
            data_obj = self.load_from_file(f"{DB_DIR}/{filename}")
            text = " ".join(s.text for s in data_obj.sentences)
            return text
        else:
            return None
    
    def delete_analysis_text(self, id: int) -> None:
        filename = self.sql.get_analysis_filename_by_id(id)
        if filename != None:
            self.sql.delete_analysis_by_id(id)
            os.remove(f"{DB_DIR}/{filename}")
            print(f"(?) Removed file {filename}")
        else:
            print("(!) File not found")
        
    def get_tree(self, id: int) -> list | None:
        filename = self.sql.get_analysis_filename_by_id(id)
        if filename != None:
            data_obj = self.load_from_file(f"{DB_DIR}/{filename}")
            sentences: list = data_obj.model_dump()["sentences"]
            trees = []
            for sent in sentences:
                trees.append(sent["tokens"])
            return trees
        else:
            return None


class Parser:
    def __init__(self) -> None:
        # 1. Загружаем spaCy для dependency parse
        self.nlp_spacy = spacy.load("en_core_web_sm")
        
        # 2. Загружаем stanza для constituency parse (только нужные процессоры)
        #    Модель скачается автоматически при первом вызове
        self.nlp_stanza = stanza.Pipeline(
            lang='en',
            processors='tokenize,pos,constituency',   # токенизация + дерево составляющих
            use_gpu=False,
            verbose=False
        )
        self.manager = DBManager()
        print("(✓) Модели загружены: spaCy (dep) + stanza (constituency)")

    # ---------- Вспомогательные методы для перевода тегов (остаются без изменений) ----------
    def get_tag_rus(self, tag: str) -> str:
        tag_map = {
            "$": "Символ (валюта)", "''": "Закрывающая кавычка", ",": "Запятая",
            "-LRB-": "Открывающая круглая скобка", "-RRB-": "Закрывающая круглая скобка",
            ".": "Точка (конец предложения)", ":": "Двоеточие или многоточие",
            "ADD": "Email адрес", "AFX": "Аффикс", "CC": "Сочинительный союз",
            "CD": "Количественное числительное", "DT": "Определитель (арт., указ. мест.)",
            "EX": "Конструкция 'there is/are'", "FW": "Иностранное слово",
            "HYPH": "Дефис", "IN": "Подчинительный союз или предлог",
            "JJ": "Прилагательное", "JJR": "Прилагательное, сравнительная степень",
            "JJS": "Прилагательное, превосходная степень", "LS": "Маркер списка",
            "MD": "Модальный глагол (can, will...)", "NFP": "Избыточная пунктуация",
            "NN": "Существительное, ед. ч. или неисчислимое",
            "NNP": "Имя собственное, ед. ч.", "NNPS": "Имя собственное, мн. ч.",
            "NNS": "Существительное, мн. ч.", "PDT": "Предопределитель",
            "POS": "Притяжательный аффикс ('s)", "PRP": "Личное местоимение",
            "PRP$": "Притяжательное местоимение", "RB": "Наречие",
            "RBR": "Наречие, сравнительная степень", "RBS": "Наречие, превосходная степень",
            "RP": "Частица (фразовый глагол)", "SYM": "Символ",
            "TO": "Частица 'to' (инфинитив)", "UH": "Междометие",
            "VB": "Глагол, базовая форма", "VBD": "Глагол, прошедшее время",
            "VBG": "Глагол, герундий или причастие наст. вр.",
            "VBN": "Глагол, причастие прош. вр.",
            "VBP": "Глагол, наст. вр. (кроме 3-го лица ед.ч.)",
            "VBZ": "Глагол, наст. вр., 3-е лицо ед.ч.",
            "WDT": "Wh-определитель (which, that...)",
            "WP": "Wh-местоимение (who, what...)",
            "WP$": "Wh-притяжательное местоимение (whose)",
            "WRB": "Wh-наречие (where, when...)",
            "XX": "Неизвестный тег", "_SP": "Пробел", "``": "Открывающая кавычка"
        }
        return tag_map.get(tag, f"Неизвестно ({tag})")
    
    def get_dep_rus(self, dep_: str) -> str:
        dep_map = {
            "root": "Корень предложения", "acl": "Придаточное определительное",
            "acomp": "Придаточное дополнительное (прил.)",
            "advcl": "Обстоятельственное придаточное", "advmod": "Обстоятельство",
            "agent": "Агент действия", "amod": "Определение (прилагательное)",
            "appos": "Приложение", "attr": "Атрибут", "aux": "Вспомогательный глагол",
            "auxpass": "Вспомогательный глагол пассива", "case": "Падежный маркер",
            "cc": "Сочинительный союз", "ccomp": "Придаточное дополнительное",
            "compound": "Составное слово", "conj": "Однородный член",
            "csubj": "Придаточное подлежащее", "csubjpass": "Придаточное подлежащее пассива",
            "dative": "Дательный падеж", "dep": "Зависимость по умолчанию",
            "det": "Определитель", "dobj": "Прямое дополнение", "expl": "Эксплетива",
            "intj": "Междометие", "mark": "Маркер придаточного", "meta": "Мета-информация",
            "neg": "Отрицание", "nmod": "Именная модификация",
            "npadvmod": "Именное обстоятельство", "nsubj": "Именное подлежащее",
            "nsubjpass": "Именное подлежащее пассива", "nummod": "Числовое определение",
            "oprd": "Предикативное дополнение", "parataxis": "Паратаксис",
            "pcomp": "Придаточное предлога", "pobj": "Объект предлога",
            "poss": "Притяжательное", "preconj": "Предшествующий союз",
            "predet": "Предопределитель", "prep": "Предлог", "prt": "Частица",
            "punct": "Пунктуация", "quantmod": "Модификатор квантора",
            "relcl": "Относительное придаточное", "xcomp": "Придаточное безличное"
        }
        return dep_map.get(dep_.lower(), f"Неизвестно ({dep_})")

    # ---------- Преобразование дерева составляющих из stanza в словарь ----------
    @staticmethod
    def _constituency_tree_to_dict(tree) -> dict:
        """Рекурсивно превращает объект stanza.Tree в словарь для JSON."""
        if tree.is_leaf():
            # Терминальный узел (слово)
            return {
                "label": tree.label,
                "children": []
            }
        else:
            return {
                "label": tree.label,
                "children": [Parser._constituency_tree_to_dict(child) for child in tree.children]
            }

    # ---------- Основной метод парсинга ----------
    def parse_text_only(self, text: str) -> tuple[TextData, int, float]:
        if not text.strip():
            return None, 0, 0.0

        start_time = time.time()
        
        # 1. Dependency parse через spaCy
        doc_spacy = self.nlp_spacy(text)
        
        # 2. Constituency parse через stanza (обрабатываем весь текст целиком)
        doc_stanza = self.nlp_stanza(text)
        
        sentences = []
        word_count = 0
        
        # Предполагаем, что количество предложений в spacy и stanza одинаково
        # (для английского обычно совпадает)
        for i, (sent_spacy, sent_stanza) in enumerate(zip(doc_spacy.sents, doc_stanza.sentences)):
            # ----- Сбор токенов из spacy (dependency) -----
            tokens = []
            for token in sent_spacy:
                if (token.like_url or token.like_email or token.like_num or not token.text.strip()):
                    continue
                
                form = token.text.lower().strip()
                role = self.get_dep_rus(token.dep_.lower())
                tag = self.get_tag_rus(token.tag_)
                parent_word = token.head.text
                
                if token.head == token:
                    parent_index_in_sentence = ROOT_IDX
                else:
                    parent_index_in_sentence = token.head.i
                
                token_obj = TokenData(
                    id=token.i, word=form, tag=tag, dep=role,
                    parent_word=parent_word, parent_id=parent_index_in_sentence
                )
                tokens.append(token_obj)
                word_count += 1
            
            # ----- Дерево составляющих из stanza -----
            stanza_tree = sent_stanza.constituency
            tree_dict = self._constituency_tree_to_dict(stanza_tree)
            constituency_json = json.dumps(tree_dict, ensure_ascii=False)
            
            sent_obj = SentenceData(
                id=i,
                text=sent_spacy.text,   # берём текст из spacy (обычно совпадает)
                tokens=tokens,
                constituency_tree=constituency_json
            )
            sentences.append(sent_obj)
        
        text_data = TextData(
            meta={
                "language": "English",
                "model_used": "spacy_en_core_web_sm + stanza_en",
                "processed_at": time.strftime("%Y-%m-%dT%H:%M:%S"),
                "total_sentences": len(sentences)
            },
            sentences=sentences
        )
        
        duration = time.time() - start_time
        return text_data, word_count, duration

    # ---------- Публичный метод parse (сохраняет в БД) ----------
    def parse(self, text: str, name: str) -> tuple[int, float]:
        text_data, word_count, duration = self.parse_text_only(text)
        
        if text_data is None:
            return 0, 0.0
        
        self.manager.save_to_file(text_data, name)
        self.manager.sql.save_parsing_stat(word_count, duration)
        
        return word_count, duration