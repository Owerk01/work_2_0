import sqlite3 as sql
import os

BASE_DIR = os.path.dirname(__file__)
DB_DIR = os.path.join(BASE_DIR, "DB")
DATABASE = os.path.join(DB_DIR, "mydb.db")
STATS_DB = "parsing_stats"
DB_NAME = "chat"

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
                chat_id INTEGER NOT NULL,
                message_position INTEGER NOT NULL,
                query TEXT NOT NULL,
                response TEXT NOT NULL,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            );
            """)

        self.crs.execute(
            f"CREATE INDEX IF NOT EXISTS idx_chat_pos ON {DB_NAME}(chat_id, message_position);"
        )

        self.crs.execute(f"""
            CREATE TABLE IF NOT EXISTS {STATS_DB} (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                answer_duration REAL NOT NULL,
                timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
            );
            """)
    
    def close(self) -> None:
        if self.conn:
            self.conn.close()
            self.conn = None

    def __del__(self) -> None:
        self.close()
    
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