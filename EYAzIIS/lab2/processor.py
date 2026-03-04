import spacy
import time
from data import DB, DB_NAME, CORPUS_DB_NAME


# Тут можешь его расширять
class SQLhelper:
    def __init__(self)->None:
        self.db = DB()

    def select_all(self) -> list[tuple]:
        return self.db.select_query(f"SELECT * from {DB_NAME}")
    
    def insert_records(self, values:list[tuple])->None:
        for v in values:
            self.db.execute_query(f"INSERT OR IGNORE INTO {DB_NAME} (lemma, form, part_of_speech, role, frequency, text_id) VALUES (?, ?, ?, ?, ?, ?)", v)
    
    def insert_text_for_corpus(self, values:tuple)->None:
        self.db.execute_query(f"INSERT OR IGNORE INTO {CORPUS_DB_NAME} (filename, author, name, year, source, genre, style, content) VALUES (?, ?, ?, ?, ?, ?, ?, ?)", values)

    def get_last_corpus_id(self, values:tuple)->int:
        result = self.db.select_query(f"SELECT id FROM {CORPUS_DB_NAME} WHERE author = ? AND name = ? AND year = ? AND source = ?", values)
        return result[0][0] if result else None
    
    def get_corp_text_by_id(self, id: int):
        result = self.db.select_query(
            f"SELECT * FROM {CORPUS_DB_NAME} WHERE id = ?",
            (id,)
        )
        return result[0] if result else None

    def delete_corpus_text(self, id:int)->None:
        self.db.execute_query(f"DELETE FROM {CORPUS_DB_NAME} WHERE id = ?", (id,))

    def get_form_count(self, id:int | None = None)->int | None:
        if id != None:
            result = self.db.select_query(f"SELECT COALESCE(SUM(frequency),0) FROM {DB_NAME} WHERE text_id = ?",
            (id,)
        )
            return result[0][0] if result else None
        else:
            result = self.db.select_query(f"SELECT COALESCE(SUM(frequency),0) FROM {DB_NAME}",())
            return result[0][0] if result else None
    
    def get_unique_count(self, id:int | None = None, field:str = "form")->int | None:
        if id != None:
            result = self.db.select_query(f"SELECT COUNT(DISTINCT {field}) FROM {DB_NAME} WHERE text_id = ?",
            (id,)
        )
            return result[0][0] if result else None
        else:
            result = self.db.select_query(f"SELECT COUNT(DISTINCT {field}) FROM {DB_NAME}",())
            return result[0][0] if result else None
    
    def get_pos_stats(self, text_id: int | None = None) -> list[tuple[str, int]]:
        if text_id != None:
            query = f"""
                SELECT part_of_speech, COALESCE(SUM(frequency), 0) AS count 
                FROM {DB_NAME} 
                WHERE text_id = ? 
                GROUP BY part_of_speech 
                ORDER BY count DESC
            """
            params = (text_id,)
        else:
            query = f"""
                SELECT part_of_speech, COALESCE(SUM(frequency), 0) AS count 
                FROM {DB_NAME} 
                GROUP BY part_of_speech 
                ORDER BY count DESC
            """
            params = ()
        
        return self.db.select_query(query, params)
    
    def get_top_forms(self, text_id: int | None = None, limit: int = 10) -> list[tuple[str, int]]:

        if text_id is not None:
            query = f"""
                SELECT form, SUM(frequency) AS total_freq 
                FROM {DB_NAME} 
                WHERE text_id = ? 
                GROUP BY form 
                ORDER BY total_freq DESC 
                LIMIT ?
            """
            params = (text_id, limit)
        else:
            query = f"""
                SELECT form, SUM(frequency) AS total_freq 
                FROM {DB_NAME} 
                GROUP BY form 
                ORDER BY total_freq DESC 
                LIMIT ?
            """
            params = (limit,)
        
        return self.db.select_query(query, params)


    def insert(self, lemma:str, form:str, pos:str, role:str)->None:
        self.db.execute_query(
            f"INSERT OR IGNORE INTO {DB_NAME} (lemma, form, part_of_speech, role) VALUES (?, ?, ?, ?)",
            (lemma, form, pos, role)
        )
    
    # По айди
    def update(self, id:int, lemma:str, form:str, pos:str, role:str)->None:
        self.db.execute_query(
            f"UPDATE {DB_NAME} SET lemma = ?, form = ?, part_of_speech = ?, role = ? WHERE id = ?",
            (lemma, form, pos, role, id)
        )

    def get_by_id(self, id: int):
        result = self.db.select_query(
            f"SELECT * FROM {DB_NAME} WHERE id = ?",
            (id,)
        )
        return result[0] if result else None

    def search(self, lemma=None, form=None, pos=None, role=None, id=None):
        conditions = []
        params = []

        if lemma:
            conditions.append("lemma LIKE ?")
            params.append(f"%{lemma}%")
        if form:
            conditions.append("form LIKE ?")
            params.append(f"%{form}%")
        if pos:
            conditions.append("part_of_speech LIKE ?")
            params.append(f"%{pos}%")
        if role:
            conditions.append("role LIKE ?")
            params.append(f"%{role}%")
        if id is not None: 
            conditions.append("id = ?")
            params.append(id)

        query = f"SELECT * FROM {DB_NAME}"
        if conditions:
            query += " WHERE " + " AND ".join(conditions)

        return self.db.select_query(query, tuple(params))

    def delete(self, id:int)->None:
        self.db.execute_query(f"DELETE FROM {DB_NAME} WHERE id = ?", (id,))

    def save_parsing_stat(self, word_count: int, duration: float) -> None:
        self.db.execute_query(
            "INSERT INTO parsing_stats (word_count, duration) VALUES (?, ?)",
            (word_count, duration)
        )

    def get_all_stats(self) -> list[tuple]:
        return self.db.select_query("SELECT word_count, duration, timestamp FROM parsing_stats ORDER BY timestamp")

class Parser:

    def __init__(self) -> None:
        self.nlp = spacy.load("en_core_web_sm")
        self.sql = SQLhelper()
        self.pos_map = {
            "ADJ": "Прилагательное",
            "ADP": "Предлог",
            "ADV": "Наречие",
            "AUX": "Вспомогательный глагол",
            "CCONJ": "Сочинительный союз",
            "DET": "Определитель",
            "INTJ": "Междометие",
            "NOUN": "Существительное",
            "NUM": "Числительное",
            "PART": "Частица",
            "PRON": "Местоимение",
            "PROPN": "Имя собственное",
            "PUNCT": "Пунктуация",
            "SCONJ": "Подчинительный союз",
            "SYM": "Символ",
            "VERB": "Глагол",
            "X": "Другое",
            "SPACE": "Пробел"
        }

        self.dep_map = {
        "root": "Корень предложения",
        "acl": "Придаточное определительное",
        "acomp": "Придаточное дополнительное (прил.)",
        "advcl": "Обстоятельственное придаточное",
        "advmod": "Обстоятельство",
        "agent": "Агент действия",
        "amod": "Определение (прилагательное)",
        "appos": "Приложение",
        "attr": "Атрибут",
        "aux": "Вспомогательный глагол",
        "auxpass": "Вспомогательный глагол пассива",
        "case": "Падежный маркер",
        "cc": "Сочинительный союз",
        "ccomp": "Придаточное дополнительное",
        "compound": "Составное слово",
        "conj": "Однородный член",
        "csubj": "Придаточное подлежащее",
        "csubjpass": "Придаточное подлежащее пассива",
        "dative": "Дательный падеж",
        "dep": "Зависимость по умолчанию",
        "det": "Определитель",
        "dobj": "Прямое дополнение",
        "expl": "Эксплетива",
        "intj": "Междометие",
        "mark": "Маркер придаточного",
        "meta": "Мета-информация",
        "neg": "Отрицание",
        "nmod": "Именная модификация",
        "npadvmod": "Именное обстоятельство",
        "nsubj": "Именное подлежащее",
        "nsubjpass": "Именное подлежащее пассива",
        "nummod": "Числовое определение",
        "oprd": "Предикативное дополнение",
        "parataxis": "Паратаксис",
        "pcomp": "Придаточное предлога",
        "pobj": "Объект предлога",
        "poss": "Притяжательное",
        "preconj": "Предшествующий союз",
        "predet": "Предопределитель",
        "prep": "Предлог",
        "prt": "Частица",
        "punct": "Пунктуация",
        "quantmod": "Модификатор квантора",
        "relcl": "Относительное придаточное",
        "xcomp": "Придаточное безличное"
        }
    
    def get_pos_rus(self, pos_: str) -> str:
        return self.pos_map.get(pos_.upper(), f"Неизвестно ({pos_})")


    def get_dep_rus(self, dep_: str) -> str:
        return self.dep_map.get(dep_.lower(), f"Неизвестно ({dep_})")

    def parse(self, text:str, text_id:int) -> tuple[int, float]:

        if not text.strip():
            return 0, 0.0

        start_time = time.time()
        doc = self.nlp(text)
        records: list[tuple] = []
        forms_frequency:dict[tuple,int] = {}

        for token in doc:
            if (
                token.is_punct or
                token.is_space or
                token.is_bracket or
                token.like_url or
                token.like_email or
                token.like_num or
                not token.text.strip()
            ):
                continue

            form = token.text.lower().strip()
            lemma = token.lemma_.lower().strip()

            if lemma == "-pron-":
                lemma = form

            if not form.isalpha() or not lemma.isalpha():
                continue

            if len(form) > 31 or len(lemma) > 31:
                continue

            pos = self.get_pos_rus(token.pos_)
            role = self.get_dep_rus(token.dep_)
            forms_frequency[(lemma, form, pos, role)] += 1
            records.append((lemma, form, pos, role, forms_frequency[(lemma, form, pos, role)], text_id))

        word_count = len(records)

        if records:
            self.sql.insert_records(records)

        duration = time.time() - start_time

        # Сохраняем статистику
        self.sql.save_parsing_stat(word_count, duration)

        return word_count, duration

class CorpusHandler:
    def __init__(self)->None:
        self.parser = Parser()
    
    def add_text_to_corpus(self, filename:str, author:str, name:str, year:int,source:str, genre:str, style:str, content:str)->tuple[int, float]:
        if self.parser.sql.get_last_corpus_id((author, name, year, source)) == None:
            self.parser.sql.insert_text_for_corpus((filename, author, name, year, source, genre, style, content))
            last_id = self.parser.sql.get_last_corpus_id((author, name, year, source))
            return self.parser.parse(content, last_id)
    
    def delete_corp(self, id:int)->None:
        if self.parser.sql.get_corp_text_by_id(id) != None:
            self.parser.sql.delete_corpus_text(id)

    def edit_corp_text(self, id:int, filename:str, author:str, name:str, year:int,source:str, genre:str, style:str, content:str)->tuple[int, float]:
        if self.parser.sql.get_corp_text_by_id(id) != None:
            self.parser.sql.delete_corpus_text(id)
            return self.add_text_to_corpus(filename, author, name, year, source, genre, style, content)
    
    # you can get stats for specific text ID or for all of them
    def get_corpus_stats(self, id:int = None) -> dict:
        stats={}
        stats["form_count"] = self.parser.sql.get_form_count(id)
        stats["unique_form_count"] = self.parser.sql.get_unique_count(id, "form")
        stats["unique_lemma_count"] = self.parser.sql.get_unique_count(id, "lemma")
        stats["pos_freq"] = self.parser.sql.get_pos_stats(id)
        stats["top_form"] = self.parser.sql.get_top_forms(id)

        return stats
    
    # TODO
    def concrodance(self, word:str)->dict[str, list[str]]:
        # Файл: часть предложений с переданным словом в них
        matches: dict[str, list[str]]= {}

        return matches


    
            
    
    
    
            


