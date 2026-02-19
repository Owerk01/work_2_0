import sqlite3 as sql
import os

BASE_DIR = os.path.dirname(__file__)
DB_DIR = os.path.join(BASE_DIR, "DB")
DATABASE = os.path.join(DB_DIR, "mydb.db")
DB_NAME = "vocabulary"

if not os.path.exists(DB_DIR):
    os.mkdir(DB_DIR)
    print(f"(?) Created folder {DB_DIR}/")

class DB:
    def __init__(self) -> None:
        self.conn = sql.connect(DATABASE)
        self.crs = self.conn.cursor()
        self.crs.execute(
            f"""
            CREATE TABLE IF NOT EXISTS {DB_NAME} (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            lemma VARCHAR(31) NOT NULL,
            form VARCHAR(31) NOT NULL,
            part_of_speech VARCHAR(31),
            role VARCHAR(31) 
            );
            """
            )
        self.crs.execute(
            f"""
            CREATE INDEX IF NOT EXISTS idx_lemma ON {DB_NAME}(lemma);
            """
            ) 
        self.crs.execute(
            f"""
            CREATE UNIQUE INDEX IF NOT EXISTS idx_all_unique ON {DB_NAME} (lemma, form, part_of_speech, role);
            """
            )

        self.crs.execute("""
            CREATE TABLE IF NOT EXISTS parsing_stats (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                word_count INTEGER NOT NULL,
                duration REAL NOT NULL,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            );
            """)
    
    def __del__(self)-> None:
        if self.conn:
            self.conn.close()
    
    def execute_query(self, command:str, params: tuple) -> None:
        try:
            self.crs.execute(command, params)
            self.conn.commit()

        except sql.Error as e:
            print(f"(!) Error: {e}")
            self.conn.rollback()

    def select_query(self, command: str, params: tuple = ()) -> list[tuple]:
        data: list[tuple] = []
        if not command.strip().upper().startswith("SELECT"):
            print("Incorrect 'SELECT' query")
            return data

        try:
            self.crs.execute(command, params)
            data = self.crs.fetchall()
        except sql.Error as e:
            print(f"(!) Error: {e}")
            self.conn.rollback()

        return data
        
