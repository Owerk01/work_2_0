from datetime import datetime
import spacy
import time, os
from data import DB, DB_NAME, DB_DIR, STATS_DB
from data import SentenceData, TokenData, TextData
from pydantic import ValidationError


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
        self.nlp = spacy.load("en_core_web_sm")
        self.manager = DBManager()
    
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
            "acl": "Придаточное прилагательное (adjectival clause)",
            "acomp": "Прилагательное дополнение (adjectival complement)",
            "advcl": "Модификатор придаточного обстоятельственного предложения (adverbial clause modifier)",
            "advmod": "Наречное определение (adverbial modifier)",
            "agent": "Агент (agent)",
            "amod": "Атрибутивное определение (adjectival modifier)",
            "appos": "Приложение (appositional modifier)",
            "attr": "Атрибут (attribute)",
            "aux": "Вспомогательный глагол (auxiliary)",
            "auxpass": "Вспомогательный глагол пассива (auxiliary passive)",
            "case": "Падежный маркер (case)",
            "cc": "Сочинительный союз (coordinating conjunction)",
            "ccomp": "Придаточное дополнение (clausal complement)",
            "compound": "Составное слово (compound)",
            "conj": "Однородный член / соединение (conjunct)",
            "cop": "Копула (copula)",
            "csubj": "Придаточное- подлежащее (clausal subject)",
            "csubjpass": "Придаточное- подлежащее (пассив) (clausal subject passive)",
            "dative": "Дательный (dative)",
            "dep": "Неуточнённая зависимость (dep)",
            "det": "Определитель (determiner)",
            "dobj": "Прямое дополнение (direct object)",
            "expl": "Ругательство (expletive)",
            "intj": "Междометие (interjection)",
            "mark": "Маркер (marker)",
            "meta": "Мета-модификатор (meta)",
            "neg": "Отрицание (negation modifier)",
            "nn": "Именной компонент (noun compound modifier)",
            "nounmod": "Модификатор имени (modifier of nominal)",
            "npmod": "Имя как обстоятельство (noun phrase as adverbial modifier)",
            "nsubj": "Именное подлежащее (nominal subject)",
            "nsubjpass": "Именное подлежащее (пассив) (nominal subject passive)",
            "nummod": "Числовой модификатор (numeric modifier)",
            "oprd": "Предикативное дополнение (object predicate)",
            "obj": "Объект (object)",
            "obl": "Косвенный член / облик (oblique nominal)",
            "parataxis": "Паратаксис (parataxis)",
            "pcomp": "Дополнение предлога (complement of preposition)",
            "pobj": "Объект предлога (object of preposition)",
            "poss": "Притяжательное (possession modifier)",
            "preconj": "Предшествующий союз (pre-correlative conjunction)",
            "prep": "Предложный модификатор / предлог (prepositional modifier)",
            "prt": "Частица (particle)",
            "punct": "Пунктуация (punctuation)",
            "quantmod": "Модификатор квантора (modifier of quantifier)",
            "relcl": "Относительное придаточное (relative clause modifier)",
            "root": "Корень (root)",
            "xcomp": "Открытое придаточное (open clausal complement)"
        }
        return dep_map.get((dep_ or "").lower(), f"Неизвестно ({dep_})")
    
    def parse_text_only(self, text: str) -> tuple[TextData, int, float]:
        if not text.strip():
            return None, 0, 0.0
        
        start_time = time.time()
        doc = self.nlp(text)
        sentences = []
        word_count = 0
        
        for i, sent in enumerate(doc.sents):
            tokens = []
            
            for token in sent:
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
            
            sent_obj = SentenceData(id=i, text=sent.text, tokens=tokens)
            sentences.append(sent_obj)
        
        text_data = TextData(
            meta={
                "language": "English", "model_used": "en_core_web_sm",
                "processed_at": datetime.now().isoformat(),
                "total_sentences": len(sentences)
            },
            sentences=sentences
        )
        
        duration = time.time() - start_time
        return text_data, word_count, duration
    
    def parse(self, text: str, name: str) -> tuple[int, float]:
        text_data, word_count, duration = self.parse_text_only(text)
        
        if text_data is None:
            return 0, 0.0
        
        self.manager.save_to_file(text_data, name)
        self.manager.sql.save_parsing_stat(word_count, duration)
        
        return word_count, duration