import sqlite3 as sql
import os

DIR = "DB"
DATABASE = "mydb.db"
PATH = DIR + "/"+ DATABASE
DB_NAME = "vocabulary"

if not os.path.exists(DIR):
    os.mkdir(DIR)
    print(f"(?) Created folder {DIR}/")

class DB:
    def __init__(self) -> None:
        self.conn = sql.connect(PATH)
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
        
