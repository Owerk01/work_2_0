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
    
    def get_pos_rus(self, pos_: str) -> str:
        pos_map = {
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
        return pos_map.get(pos_.upper(), f"Неизвестно ({pos_})")


    def get_dep_rus(self, dep_: str) -> str:
        dep_map = {
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
        return dep_map.get(dep_.lower(), f"Неизвестно ({dep_})")

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

            pos = self.get_pos_rus(token.pos_)
            role = self.get_dep_rus(token.dep_)

            records.append((lemma, form, pos, role))

        if records:
            self.sql.insert_records(records)

# tests = "These are my super tests, I guess. Let's check it out! Words are: run, running, ran."
# Использование:
# prs = Parser()
# prs.parse(tests)
# print(prs.sql.select_all())

