from datetime import datetime
import spacy
import time
from data import DBsql, DB_NAME, DB_DIR
from data import SentenceData, TokenData, TextData

ROOT_IDX = -1

class SQLhelper:
    def __init__(self)->None:
        self.db = DBsql()

    def save_parsing_stat(self, word_count: int, duration: float) -> None:
        self.db.execute_query(
            f"INSERT INTO {DB_NAME} (word_count, duration) VALUES (?, ?)",
            (word_count, duration)
        )

    def get_all_stats(self) -> list[tuple]:
        return self.db.select_query(f"SELECT word_count, duration, timestamp FROM {DB_NAME} ORDER BY timestamp")

class JSONhelper:
    def __init__(self)->None:
        pass
    def save_to_file(self, text:TextData)->None:
        json_data = text.model_dump_json(indent=4, exclude_none=True)
        with open(f"{DB_DIR}/analysis_result_{datetime.now().isoformat()}.json", "w", encoding="utf-8") as f:
            f.write(json_data)
        print(f"(?) Saved file: analysis_result_{datetime.now().isoformat()}.json")


class Parser:
    def __init__(self) -> None:
        self.nlp = spacy.load("en_core_web_sm")
        self.sql = SQLhelper()
        self.pyd_json = JSONhelper()
    
    def get_tag_rus(self, tag: str) -> str:
        tag_map = {
            "$": "Символ (валюта)",
            "''": "Закрывающая кавычка",
            ",": "Запятая",
            "-LRB-": "Открывающая круглая скобка",
            "-RRB-": "Закрывающая круглая скобка",
            ".": "Точка (конец предложения)",
            ":": "Двоеточие или многоточие",
            "ADD": "Email адрес",
            "AFX": "Аффикс",
            "CC": "Сочинительный союз",
            "CD": "Количественное числительное",
            "DT": "Определитель (арт., указ. мест.)",
            "EX": "Конструкция 'there is/are'",
            "FW": "Иностранное слово",
            "HYPH": "Дефис",
            "IN": "Подчинительный союз или предлог",
            "JJ": "Прилагательное",
            "JJR": "Прилагательное, сравнительная степень",
            "JJS": "Прилагательное, превосходная степень",
            "LS": "Маркер списка",
            "MD": "Модальный глагол (can, will...)",
            "NFP": "Избыточная пунктуация",
            "NN": "Существительное, ед. ч. или неисчислимое",
            "NNP": "Имя собственное, ед. ч.",
            "NNPS": "Имя собственное, мн. ч.",
            "NNS": "Существительное, мн. ч.",
            "PDT": "Предeterminer (предшествующий определитель)",
            "POS": "Притяжательный аффикс ('s)",
            "PRP": "Личное местоимение",
            "PRP$": "Притяжательное местоимение",
            "RB": "Наречие",
            "RBR": "Наречие, сравнительная степень",
            "RBS": "Наречие, превосходная степень",
            "RP": "Частица (фразовый глагол)",
            "SYM": "Символ",
            "TO": "Частица 'to' (инфинитив)",
            "UH": "Междометие",
            "VB": "Глагол, базовая форма",
            "VBD": "Глагол, прошедшее время",
            "VBG": "Глагол, герундий или причастие наст. вр.",
            "VBN": "Глагол, причастие прош. вр.",
            "VBP": "Глагол, наст. вр. (кроме 3-го лица ед.ч.)",
            "VBZ": "Глагол, наст. вр., 3-е лицо ед.ч.",
            "WDT": "Wh-определитель (which, that...)",
            "WP": "Wh-местоимение (who, what...)",
            "WP$": "Wh-притяжательное местоимение (whose)",
            "WRB": "Wh-наречие (where, when...)",
            "XX": "Неизвестный тег",
            "_SP": "Пробел",
            "``": "Открывающая кавычка"
        }
        return tag_map.get(tag, f"Неизвестно ({tag})")
    
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

    def parse(self, text: str) -> tuple[int, float]:

        if not text.strip():
            return 0, 0.0

        start_time = time.time()
        doc = self.nlp(text)
        sentences = []
        word_count = 0

        for i,sent in enumerate(doc.sents):

            tokens = []

            for token in sent:
                if (
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

                #pos = self.get_pos_rus(token.pos_)
                role = self.get_dep_rus(token.dep_.lower())
                tag = self.get_tag_rus(token.tag_)
                parent_word = token.head.text

                # if (token == (root of sentence tree))
                if token.head == token:
                    parent_index_in_sentence = ROOT_IDX
                else:
                    parent_index_in_sentence = token.head.i - sent.start

                token_obj = TokenData(
                    id=token.i,
                    word=form,
                    lemma=lemma,
                    #pos=pos,
                    tag=tag,
                    dep=role,
                    parent_word=parent_word,
                    parent_id=parent_index_in_sentence
                )
                tokens.append(token_obj)
                word_count += 1
            
            sent_obj= SentenceData(id=i, text=sent.text, tokens=tokens)
            sentences.append(sent_obj)

        text_data = TextData(
            meta={
                "language": "English",
                "model_used": "en_core_web_sm",
                "processed_at": datetime.now().isoformat(),
                "total_sentences": len(sentences)
            },
            sentences=sentences
        )
        self.pyd_json.save_to_file(text_data)
        duration = time.time() - start_time
        self.sql.save_parsing_stat(word_count, duration)

        return word_count, duration

test = Parser()
test.parse("Very cool sentence of mine.")
