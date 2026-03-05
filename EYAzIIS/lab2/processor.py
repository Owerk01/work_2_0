import spacy
import time
import re
from data import DB, DB_NAME, CORPUS_DB_NAME

try:
    from docx import Document
    HAS_DOCX = True
except ImportError:
    HAS_DOCX = False

try:
    import pdfplumber
    HAS_PDF = True
except ImportError:
    HAS_PDF = False

from striprtf.striprtf import rtf_to_text

class SQLhelper:
    def __init__(self):
        self.db = DB()

    def insert_text_for_corpus(self, values: tuple):
        self.db.execute_query(f"""
            INSERT INTO {CORPUS_DB_NAME} 
            (filename, author, name, year, source, genre, style, subject_area, content) 
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)
        """, values)

    def get_last_corpus_id(self, author, name, year, source):
        result = self.db.select_query(
            f"SELECT id FROM {CORPUS_DB_NAME} WHERE author = ? AND name = ? AND year = ? AND source = ?",
            (author, name, year, source)
        )
        return result[0][0] if result else None

    def get_all_texts(self):
        return self.db.select_query(f"SELECT id, name, author, year, subject_area, filename FROM {CORPUS_DB_NAME}")

    def get_text_by_id(self, text_id):
        res = self.db.select_query(f"SELECT * FROM {CORPUS_DB_NAME} WHERE id = ?", (text_id,))
        return res[0] if res else None

    def get_text_content(self, text_id):
        res = self.db.select_query(f"SELECT content FROM {CORPUS_DB_NAME} WHERE id = ?", (text_id,))
        return res[0][0] if res else ""

    def delete_corpus_text(self, text_id):
        self.db.execute_query(f"DELETE FROM {CORPUS_DB_NAME} WHERE id = ?", (text_id,))

    def insert_records(self, values: list[tuple]):
        for v in values:
            self.db.execute_query(f"""
                INSERT INTO {DB_NAME} (text_id, lemma, form, part_of_speech, role, frequency) 
                VALUES (?, ?, ?, ?, ?, ?)
            """, v)

    def search_words(self, lemma=None, form=None, pos=None, text_id=None):
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
        if text_id:
            conditions.append("text_id = ?")
            params.append(text_id)
        
        query = f"SELECT * FROM {DB_NAME}"
        if conditions:
            query += " WHERE " + " AND ".join(conditions)
        query += " LIMIT 100"
        return self.db.select_query(query, tuple(params))

    def get_stats(self):
        return self.db.select_query("SELECT word_count, duration, timestamp FROM parsing_stats ORDER BY timestamp DESC LIMIT 50")
    
    def get_corpus_stats(self):
        total_texts = self.db.select_query("SELECT COUNT(*) FROM corpus_texts")[0][0]
        total_words = self.db.select_query("SELECT SUM(frequency) FROM vocabulary")[0][0] or 0
        return total_texts, total_words

    def get_lexeme_by_id(self, lexeme_id):
        res = self.db.select_query(f"SELECT * FROM {DB_NAME} WHERE id = ?", (lexeme_id,))
        return res[0] if res else None

    def update_lexeme_pos_role(self, lexeme_id, new_pos, new_role):
        """
        Редактирование POS/роли:
        1. Уменьшаем частоту старой записи на 1
        2. Создаём новую запись с новыми POS/role (частота = 1)
        """
        old_entry = self.get_lexeme_by_id(lexeme_id)
        if not old_entry:
            return False
        
        text_id, lemma, form, old_pos, old_role, old_freq = old_entry[1], old_entry[2], old_entry[3], old_entry[4], old_entry[5], old_entry[6]
        
        # Уменьшаем частоту старой записи
        if old_freq > 1:
            self.db.execute_query(f"UPDATE {DB_NAME} SET frequency = ? WHERE id = ?", (old_freq - 1, lexeme_id))
        else:
            # Если частота была 1, удаляем старую запись
            self.db.execute_query(f"DELETE FROM {DB_NAME} WHERE id = ?", (lexeme_id,))
        
        # Проверяем, существует ли уже запись с такими параметрами
        existing = self.db.select_query(f"""
            SELECT id, frequency FROM {DB_NAME} 
            WHERE text_id = ? AND lemma = ? AND form = ? AND part_of_speech = ? AND role = ?
        """, (text_id, lemma, form, new_pos, new_role))
        
        if existing:
            # Увеличиваем частоту существующей записи
            self.db.execute_query(f"UPDATE {DB_NAME} SET frequency = ? WHERE id = ?", (existing[0][1] + 1, existing[0][0]))
        else:
            # Создаём новую запись
            self.db.execute_query(f"""
                INSERT INTO {DB_NAME} (text_id, lemma, form, part_of_speech, role, frequency) 
                VALUES (?, ?, ?, ?, ?, 1)
            """, (text_id, lemma, form, new_pos, new_role))
        
        return True

class Parser:
    def __init__(self) -> None:
        try:
            self.nlp = spacy.load("en_core_web_sm")
        except OSError:
            print("(!) Model 'en_core_web_sm' not found.")
            self.nlp = None
            
        self.sql = SQLhelper()
        
        self.pos_map = {
            "ADJ": "Прилагательное", "ADP": "Предлог", "ADV": "Наречие",
            "AUX": "Вспомогательный глагол", "CCONJ": "Сочинительный союз", "DET": "Определитель",
            "INTJ": "Междометие", "NOUN": "Существительное", "NUM": "Числительное",
            "PART": "Частица", "PRON": "Местоимение", "PROPN": "Имя собственное",
            "PUNCT": "Пунктуация", "SCONJ": "Подчинительный союз", "SYM": "Символ",
            "VERB": "Глагол", "X": "Другое"
        }
        
        self.dep_map = {
            "root": "Корень предложения", "acl": "Придаточное определительное",
            "advcl": "Обстоятельственное придаточное", "advmod": "Обстоятельство",
            "amod": "Определение (прилагательное)", "appos": "Приложение",
            "aux": "Вспомогательный глагол", "case": "Падежный маркер",
            "cc": "Сочинительный союз", "ccomp": "Придаточное дополнительное",
            "compound": "Составное слово", "conj": "Однородный член",
            "csubj": "Придаточное подлежащее", "det": "Определитель",
            "dobj": "Прямое дополнение", "expl": "Эксплетива",
            "mark": "Маркер придаточного", "neg": "Отрицание",
            "nmod": "Именная модификация", "nsubj": "Именное подлежащее",
            "nummod": "Числовое определение", "parataxis": "Паратаксис",
            "prep": "Предлог", "punct": "Пунктуация",
            "relcl": "Относительное придаточное", "xcomp": "Придаточное безличное"
        }

    def extract_text(self, file_path, filename):
        content = ""
        if filename.endswith('.txt'):
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
        elif filename.endswith('.rtf'):
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = rtf_to_text(f.read())
        elif filename.endswith('.docx') and HAS_DOCX:
            doc = Document(file_path)
            content = "\n".join([p.text for p in doc.paragraphs])
        elif filename.endswith('.pdf') and HAS_PDF:
            with pdfplumber.open(file_path) as pdf:
                content = "\n".join([page.extract_text() for page in pdf.pages if page.extract_text()])
        return content

    def parse(self, text: str, text_id: int):
        if not self.nlp or not text.strip():
            return 0, 0.0

        start_time = time.time()
        doc = self.nlp(text)
        records = []
        freq_dict = {} 

        for token in doc:
            if token.is_punct or token.is_space or token.like_url or token.like_num:
                continue
            
            form = token.text.lower()
            lemma = token.lemma_.lower()
            if not form.isalpha(): continue
            
            pos = self.pos_map.get(token.pos_, token.pos_)
            role = self.dep_map.get(token.dep_, token.dep_)
            
            key = (lemma, form, pos, role)
            freq_dict[key] = freq_dict.get(key, 0) + 1

        for (lemma, form, pos, role), freq in freq_dict.items():
            records.append((text_id, lemma, form, pos, role, freq))

        if records:
            self.sql.insert_records(records)
        
        duration = time.time() - start_time
        word_count = sum([r[5] for r in records])
        self.sql.db.execute_query("INSERT INTO parsing_stats (word_count, duration) VALUES (?, ?)", (word_count, duration))
        return word_count, duration

    def get_concordance(self, text_id, word_form, window=50):
        content = self.sql.get_text_content(text_id)
        if not content:
            return []
        
        matches = []
        pattern = r'\b' + re.escape(word_form) + r'\b'
        
        for match in re.finditer(pattern, content, re.IGNORECASE):
            start = max(0, match.start() - window)
            end = min(len(content), match.end() + window)
            
            prefix = "..." + content[start:match.start()] if start > 0 else content[start:match.start()]
            suffix = content[match.end():end] + "..." if end < len(content) else content[match.end():end]
            
            matches.append({
                "prefix": prefix.replace("\n", " "),
                "match": match.group(),
                "suffix": suffix.replace("\n", " ")
            })
            if len(matches) >= 20:
                break
        return matches

class CorpusHandler:
    def __init__(self):
        self.parser = Parser()
        self.sql = SQLhelper()

    def add_text_to_corpus(self, meta, content):
        self.sql.insert_text_for_corpus((*meta, content))
        text_id = self.sql.get_last_corpus_id(meta[1], meta[2], meta[3], meta[4])
        if text_id:
            return self.parser.parse(content, text_id)
        return 0, 0

    def edit_text_in_corpus(self, text_id, meta, content):
        """
        Редактирование текста: удаляем старый текст и его лексемы, добавляем новый
        """
        # Удаляем старый текст (CASCADE удалит связанные лексемы)
        self.sql.delete_corpus_text(text_id)
        
        # Добавляем новый текст
        return self.add_text_to_corpus(meta, content)