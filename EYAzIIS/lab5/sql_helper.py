from metadata import DB, DB_NAME, STATS_DB

class SQLhelper:
    def __init__(self) -> None:
        self.db = DB()
    
    def save_message_pair(self, user_id: int, query: str, response: str) -> None:
        """Сохраняет пару вопрос-ответ для конкретного пользователя"""
        result = self.db.select_query(
            f"SELECT message_position FROM {DB_NAME} WHERE chat_id = ? ORDER BY message_position DESC LIMIT 1",
            (user_id,)
        )
        next_pos = result[0][0] + 1 if result else 1

        self.db.execute_query(
            f"INSERT INTO {DB_NAME} (chat_id, message_position, query, response) VALUES (?, ?, ?, ?)",
            (user_id, next_pos, query, response)
        )
    
    def delete_chat(self, user_id: int) -> int:
        """Удаляет весь чат пользователя, возвращает кол-во удалённых строк"""
        self.db.execute_query(f"DELETE FROM {DB_NAME} WHERE chat_id = ?", (user_id,))
        return self.db.crs.rowcount
    
    def get_all_chat_messages(self, user_id: int) -> list[tuple[int, str, str]]:
        """Возвращает историю чата пользователя в хронологическом порядке"""
        return self.db.select_query(
            f"SELECT message_position, query, response FROM {DB_NAME} WHERE chat_id = ? ORDER BY message_position ASC",
            (user_id,)
        )

    def get_context(self, user_id: int) -> list[tuple[int, str, str]]:
        """Последние 50 сообщений для контекста (если понадобится для RAG)"""
        return self.db.select_query(
            f"""
            SELECT message_position, query, response
            FROM (
                SELECT message_position, query, response
                FROM {DB_NAME}
                WHERE chat_id = ?
                ORDER BY message_position DESC
                LIMIT 50
            ) AS recent
            ORDER BY message_position ASC
            """,
            (user_id,)
        )
    
    def get_all_analysis(self) -> list[tuple]:
        return self.db.select_query(f"SELECT * FROM {STATS_DB} ORDER BY id")