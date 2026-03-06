import sqlite3 as sql
import os

BASE_DIR = os.path.dirname(__file__)
DB_DIR = os.path.join(BASE_DIR, "DB")
DATABASE = os.path.join(DB_DIR, "mydb.db")
DB_NAME = "parsing_stats"

class DBsql:
    def __init__(self) -> None:
        if not os.path.exists(DB_DIR):
            os.mkdir(DB_DIR)
            print(f"(?) Created folder {DB_DIR}/")
            
        self.conn = sql.connect(DATABASE)
        self.crs = self.conn.cursor()

        self.crs.execute(f"""
            CREATE TABLE IF NOT EXISTS {DB_NAME} (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                word_count INTEGER NOT NULL,
                duration REAL NOT NULL,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            );
            """)

        self.crs.execute(
            f"""
            CREATE INDEX IF NOT EXISTS idx_word_count ON {DB_NAME}(word_count);
            """
            ) 
    
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
        
