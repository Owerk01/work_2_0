from metadata import DB, DB_NAME, STATS_DB

class SQLhelper:
    def __init__(self) -> None:
        self.db = DB()

    def save_message_pair(self, user_id: int, query: str, response: str) -> int:
        result = self.db.select_query(
            f"SELECT message_position FROM {DB_NAME} WHERE chat_id = ? ORDER BY message_position DESC LIMIT 1",
            (user_id,)
        )
        next_pos = result[0][0] + 1 if result else 1

        self.db.execute_query(
            f"INSERT INTO {DB_NAME} (chat_id, message_position, query, response) VALUES (?, ?, ?, ?)",
            (user_id, next_pos, query, response)
        )
        return next_pos

    def delete_chat(self, user_id: int) -> int:
        self.db.execute_query(f"DELETE FROM {DB_NAME} WHERE chat_id = ?", (user_id,))
        return self.db.crs.rowcount

    def get_all_chat_messages(self, user_id: int) -> list[tuple[int, str, str]]:
        return self.db.select_query(
            f"SELECT message_position, query, response FROM {DB_NAME} WHERE chat_id = ? ORDER BY message_position ASC",
            (user_id,)
        )

    def get_context(self, user_id: int) -> list[tuple[int, str, str]]:
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

    def update_turn(self, user_id: int, position: int, query: str = None, response: str = None) -> None:
        if query is not None:
            self.db.execute_query(
                f"UPDATE {DB_NAME} SET query = ? WHERE chat_id = ? AND message_position = ?",
                (query, user_id, position)
            )
        if response is not None:
            self.db.execute_query(
                f"UPDATE {DB_NAME} SET response = ? WHERE chat_id = ? AND message_position = ?",
                (response, user_id, position)
            )

    def delete_turn(self, user_id: int, position: int) -> None:
        self.db.execute_query(
            f"DELETE FROM {DB_NAME} WHERE chat_id = ? AND message_position = ?",
            (user_id, position)
        )

    def save_response_duration(self, duration: float) -> None:
        self.db.execute_query(
            f"INSERT INTO {STATS_DB} (answer_duration) VALUES (?)",
            (duration,)
        )

    def get_average_duration(self) -> float:
        result = self.db.select_query(f"SELECT AVG(answer_duration) FROM {STATS_DB}")
        if result and result[0][0] is not None:
            return round(result[0][0], 3)
        return 0.0

    def get_all_analysis(self) -> list[tuple]:
        return self.db.select_query(f"SELECT * FROM {STATS_DB} ORDER BY id")