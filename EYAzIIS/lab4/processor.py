from datetime import datetime
import spacy
import time
import os
from data import DB, DB_NAME, DB_DIR, STATS_DB
from data import SentenceData, TokenData, TextData
from pydantic import ValidationError

ROOT_IDX = -1

class SemanticAnalyzer:
    """Семантический анализатор на основе spaCy"""
    
    def __init__(self, nlp):
        self.nlp = nlp
        
        self.semantic_classes_ru = {
            "ANIMAL": "ЖИВОТНОЕ",
            "PERSON": "ЧЕЛОВЕК",
            "OBJECT": "ОБЪЕКТ",
            "ACTION": "ДЕЙСТВИЕ",
            "PROPERTY": "СВОЙСТВО",
            "CIRCUMSTANCE": "ОБСТОЯТЕЛЬСТВО",
            "QUANTITY": "КОЛИЧЕСТВО",
            "RELATION": "ОТНОШЕНИЕ",
            "DETERMINER": "ОПРЕДЕЛИТЕЛЬ",
            "PUNCTUATION": "ПУНКТУАЦИЯ",
            "ENTITY": "СУЩНОСТЬ",
            "ORGANIZATION": "ОРГАНИЗАЦИЯ",
            "LOCATION": "ЛОКАЦИЯ",
            "TIME": "ВРЕМЯ",
            "EMOTION": "ЭМОЦИЯ",
            "OTHER": "ДРУГОЕ",
            "UNKNOWN": "НЕИЗВЕСТНО"
        }
        
        self.animal_keywords = {
            'cat', 'dog', 'bird', 'fish', 'horse', 'cow', 'pig', 'sheep',
            'lion', 'tiger', 'elephant', 'monkey', 'rabbit', 'mouse', 'rat',
            'animal', 'pet', 'creature', 'mammal', 'beast', 'wolf', 'fox'
        }
        
        self.person_keywords = {
            'man', 'woman', 'boy', 'girl', 'person', 'people', 'human',
            'child', 'adult', 'friend', 'mother', 'father', 'parent',
            'he', 'she', 'they', 'him', 'her', 'them', 'his', 'hers'
        }
    
    def get_semantic_class(self, token) -> str:
        lemma = token.lemma_.lower()
        pos = token.pos_
        tag = token.tag_
        
        if token.ent_type_:
            entity_map = {
                "PERSON": "ЧЕЛОВЕК",
                "ORG": "ОРГАНИЗАЦИЯ",
                "GPE": "ЛОКАЦИЯ",
                "LOC": "ЛОКАЦИЯ",
                "PRODUCT": "ОБЪЕКТ",
                "EVENT": "СОБЫТИЕ",
                "DATE": "ВРЕМЯ",
                "TIME": "ВРЕМЯ",
                "MONEY": "КОЛИЧЕСТВО",
                "QUANTITY": "КОЛИЧЕСТВО"
            }
            return entity_map.get(token.ent_type_, "СУЩНОСТЬ")
        
        if lemma in self.animal_keywords:
            return "ЖИВОТНОЕ"
        
        if lemma in self.person_keywords or tag in ["PRP", "PRP$"]:
            return "ЧЕЛОВЕК"
        
        pos_class_map = {
            "VERB": "ДЕЙСТВИЕ",
            "AUX": "ДЕЙСТВИЕ",
            "NOUN": "ОБЪЕКТ",
            "PROPN": "СУЩНОСТЬ",
            "PRON": "ЧЕЛОВЕК",
            "ADJ": "СВОЙСТВО",
            "ADV": "ОБСТОЯТЕЛЬСТВО",
            "ADP": "ОТНОШЕНИЕ",
            "CONJ": "ОТНОШЕНИЕ",
            "DET": "ОПРЕДЕЛИТЕЛЬ",
            "NUM": "КОЛИЧЕСТВО",
            "PUNCT": "ПУНКТУАЦИЯ",
            "PART": "ОТНОШЕНИЕ",
            "INTJ": "ЭМОЦИЯ"
        }
        
        return pos_class_map.get(pos, "ДРУГОЕ")
    
    def calculate_similarity(self, token1, token2) -> float:
        try:
            if token1.has_vector and token2.has_vector:
                return float(token1.similarity(token2))
            else:
                if token1.lemma_.lower() == token2.lemma_.lower():
                    return 1.0
                elif token1.pos_ == token2.pos_:
                    return 0.3
                else:
                    return 0.0
        except:
            return 0.0
    
    def analyze_token_semantics(self, token, root_token=None) -> dict:
        result = {
            "semantic_class": self.get_semantic_class(token),
            "entity_type": token.ent_type_ if token.ent_type_ else "",
            "is_stop_word": token.is_stop,
            "similarity_to_root": 0.0
        }
        
        if root_token:
            result["similarity_to_root"] = self.calculate_similarity(token, root_token)
        
        return result


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
        if filename is not None:
            data_obj = self.load_from_file(f"{DB_DIR}/{filename}")
            text = " ".join(s.text for s in data_obj.sentences)
            return text
        else:
            return None
    
    def delete_analysis_text(self, id: int) -> None:
        filename = self.sql.get_analysis_filename_by_id(id)
        if filename is not None:
            self.sql.delete_analysis_by_id(id)
            os.remove(f"{DB_DIR}/{filename}")
            print(f"(?) Removed file {filename}")
        else:
            print("(!) File not found")
        
    def get_tree(self, id: int) -> list | None:
        filename = self.sql.get_analysis_filename_by_id(id)
        if filename is not None:
            data_obj = self.load_from_file(f"{DB_DIR}/{filename}")
            sentences = data_obj.model_dump()["sentences"]
            trees = []
            for sent in sentences:
                trees.append(sent["tokens"])
            return trees
        else:
            return None


class Parser:
    def __init__(self) -> None:
        try:
            self.nlp = spacy.load("en_core_web_md")
        except OSError:
            print("(!) Installing en_core_web_md model...")
            import os
            os.system('python -m spacy download en_core_web_md')
            self.nlp = spacy.load("en_core_web_md")
        
        self.manager = DBManager()
        self.semantic_analyzer = SemanticAnalyzer(self.nlp)
    
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
    
    def parse_text_only(self, text: str) -> tuple[TextData, int, float]:
        """Парсит текст без сохранения в БД (для редактирования)"""
        if not text.strip():
            return None, 0, 0.0
        
        start_time = time.time()
        doc = self.nlp(text)
        sentences = []
        word_count = 0
        
        for i, sent in enumerate(doc.sents):
            tokens = []
            
            root_token = None
            for token in sent:
                if token.dep_ == "root":
                    root_token = token
                    break
            
            for token in sent:
                if (token.like_url or token.like_email or 
                    token.like_num or not token.text.strip()):
                    continue
                
                form = token.text.lower().strip()
                lemma = token.lemma_.lower()
                role = self.get_dep_rus(token.dep_.lower())
                tag = self.get_tag_rus(token.tag_)
                parent_word = token.head.text
                
                token_index_in_sentence = token.i - sent.start

                if token.head == token:
                    parent_index_in_sentence = ROOT_IDX
                else:
                    parent_index_in_sentence = token.head.i - sent.start
                
                semantic_info = self.semantic_analyzer.analyze_token_semantics(token, root_token)
                
                token_obj = TokenData(
                    id=token_index_in_sentence,
                    word=form,
                    lemma=lemma,
                    tag=tag,
                    dep=role,
                    parent_word=parent_word,
                    parent_id=parent_index_in_sentence,
                    semantic_class=semantic_info["semantic_class"],
                    entity_type=semantic_info["entity_type"],
                    similarity_to_root=semantic_info["similarity_to_root"],
                    is_stop_word=semantic_info["is_stop_word"]
                )
                tokens.append(token_obj)
                word_count += 1
            
            sent_obj = SentenceData(id=i, text=sent.text, tokens=tokens)
            sentences.append(sent_obj)
        
        text_data = TextData(
            meta={
                "language": "English",
                "model_used": "en_core_web_md",
                "processed_at": datetime.now().isoformat(),
                "total_sentences": len(sentences),
                "semantic_analysis": True
            },
            sentences=sentences
        )
        
        duration = time.time() - start_time
        return text_data, word_count, duration
    
    def parse(self, text: str, name: str) -> tuple[int, float]:
        """Парсит текст и сохраняет в БД"""
        text_data, word_count, duration = self.parse_text_only(text)
        
        if text_data is None:
            return 0, 0.0
        
        self.manager.save_to_file(text_data, name)
        self.manager.sql.save_parsing_stat(word_count, duration)
        
        return word_count, duration