import sqlite3 as sql
import os
from pydantic import BaseModel
from typing import List, Optional

BASE_DIR = os.path.dirname(__file__)
DB_DIR = os.path.join(BASE_DIR, "DB")
DATABASE = os.path.join(DB_DIR, "mydb.db")
STATS_DB = "parsing_stats"
DB_NAME = "analysis"

class DB:
    def __init__(self) -> None:
        if not os.path.exists(DB_DIR):
            os.mkdir(DB_DIR)
            print(f"(?) Created folder {DB_DIR}/")

        self.conn = sql.connect(DATABASE, check_same_thread=False)
        self.crs = self.conn.cursor()

        self.crs.execute(f"""
            CREATE TABLE IF NOT EXISTS {DB_NAME} (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                name VARCHAR(127) NOT NULL,
                filename VARCHAR(127) NOT NULL,
                sentence_count INTEGER NOT NULL,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            );
            """)

        self.crs.execute(
            f"""
            CREATE INDEX IF NOT EXISTS idx_sentence_count ON {DB_NAME}(sentence_count);
            """
            ) 

        self.crs.execute(f"""
            CREATE TABLE IF NOT EXISTS {STATS_DB} (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                word_count INTEGER NOT NULL,
                duration REAL NOT NULL,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            );
            """)

        self.crs.execute(
            f"""
            CREATE INDEX IF NOT EXISTS idx_word_count ON {STATS_DB}(word_count);
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
        
class TokenData(BaseModel):
    id: int
    word: str
    tag: str
    dep: str
    parent_word: str
    parent_id: int

class SentenceData(BaseModel):
    id: int
    text: str
    tokens: List[TokenData]
    constituency_tree: Optional[str] = None

class TextData(BaseModel):
    meta: dict
    sentences: List[SentenceData]