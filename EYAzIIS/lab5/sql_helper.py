from metadata import DB, DB_NAME, STATS_DB

class SQLhelper:
    def __init__(self) -> None:
        self.db = DB()
    
    def save_message_pair(self, chat_id: int, query: str, response: str) -> None:
        result = self.db.select_query(
            f"SELECT message_position FROM {DB_NAME} WHERE chat_id = ? ORDER BY message_position DESC LIMIT 1",
            (chat_id,)
        )
        next_pos = result[0][0] + 1 if result else 1

        self.db.execute_query(
            f"INSERT INTO {DB_NAME} (chat_id, message_position, query, response) VALUES (?, ?, ?, ?)",
            (chat_id, next_pos, query, response)
        )
    
    def get_all_analysis(self) -> list[tuple]:
        return self.db.select_query(f"SELECT * FROM {STATS_DB} ORDER BY id")