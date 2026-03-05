import sqlite3 as sql
import os
BASE_DIR = os.path.dirname(__file__)
DB_DIR = os.path.join(BASE_DIR, "DB")
DATABASE = os.path.join(DB_DIR, "mydb.db")
DB_NAME = "vocabulary"
CORPUS_DB_NAME = "corpus_texts"

class DB:
    def __init__(self) -> None:
        if not os.path.exists(DB_DIR):
            os.mkdir(DB_DIR)
            print(f"(?) Created folder {DB_DIR}/")
        
        self.conn = sql.connect(DATABASE)
        self.conn.execute("PRAGMA foreign_keys = ON")
        self.crs = self.conn.cursor()
        
        self.crs.execute(f"""
            CREATE TABLE IF NOT EXISTS {CORPUS_DB_NAME} (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                filename VARCHAR(127),
                author VARCHAR(63),
                name VARCHAR(127) NOT NULL,
                year INTEGER CHECK(year BETWEEN 1800 AND 2026),
                source VARCHAR(127),
                genre VARCHAR(63),
                style VARCHAR(63),
                subject_area VARCHAR(63),
                content TEXT NOT NULL,
                created_at DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        """)
        
        self.crs.execute(f"""
            CREATE TABLE IF NOT EXISTS {DB_NAME} (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                text_id INTEGER NOT NULL,
                lemma VARCHAR(31) NOT NULL,
                form VARCHAR(31) NOT NULL,
                part_of_speech VARCHAR(31),
                role VARCHAR(31),
                frequency INTEGER DEFAULT 1,
                FOREIGN KEY (text_id) REFERENCES {CORPUS_DB_NAME}(id) ON DELETE CASCADE 
            )
        """)
        
        self.crs.execute(f"CREATE INDEX IF NOT EXISTS idx_lemma ON {DB_NAME}(lemma)")
        self.crs.execute(f"CREATE INDEX IF NOT EXISTS idx_text_id ON {DB_NAME}(text_id)")
        
        self.crs.execute("""
            CREATE TABLE IF NOT EXISTS parsing_stats (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                word_count INTEGER NOT NULL,
                duration REAL NOT NULL,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            )
        """)
        self.conn.commit()

    def __del__(self):
        if self.conn:
            self.conn.close()

    def execute_query(self, command: str, params: tuple) -> None:
        try:
            self.crs.execute(command, params)
            self.conn.commit()
        except sql.Error as e:
            print(f"(!) Error: {e}")
            self.conn.rollback()

    def select_query(self, command: str, params: tuple = ()) -> list[tuple]:
        data: list[tuple] = []
        try:
            self.crs.execute(command, params)
            data = self.crs.fetchall()
        except sql.Error as e:
            print(f"(!) Error: {e}")
        return data