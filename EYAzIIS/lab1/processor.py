import spacy
from data import DB, DB_NAME

# Тут можешь его расширять
class SQLhelper:
    def __init__(self)->None:
        self.db = DB()

    def select_all(self) -> list[tuple]:
        return self.db.select_query(f"SELECT * from {DB_NAME}")
    
    def insert_records(self, values:list[tuple])->None:
        for v in values:
            self.db.execute_query(f"INSERT OR IGNORE INTO {DB_NAME} (lemma, form, part_of_speech, role) VALUES (?, ?, ?, ?)", v)

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


class Parser:

    def __init__(self) -> None:
        self.nlp = spacy.load("en_core_web_sm")
        self.sql = SQLhelper()
    
    def parse(self, text:str) -> None:
        if not text.strip():
            return

        doc = self.nlp(text)
        records:list[tuple] = []

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

            pos = token.pos_
            role = token.dep_.lower()

            records.append((lemma, form, pos, role))

        if records:
            self.sql.insert_records(records)

# tests = "These are my super tests, I guess. Let's check it out! Words are: run, running, ran."
# Использование:
# prs = Parser()
# prs.parse(tests)
# print(prs.sql.select_all())

