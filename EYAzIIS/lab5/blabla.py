import sqlite3
import shutil
import os

DB_PATH = "EYAzIIS/lab5/DB/mydb.db"

# 1. Автоматический бэкап
if os.path.exists(DB_PATH):
    shutil.copy(DB_PATH, DB_PATH + ".bak")
    print("💾 Создан бэкап: mydb.db.bak")

conn = sqlite3.connect(DB_PATH)
try:
    conn.executescript("""
        -- 2. Удаляем индекс, который ссылается на chat_id
        DROP INDEX IF EXISTS idx_chat_pos;
        
        -- 3. Удаляем колонку
        ALTER TABLE chat DROP COLUMN chat_id;
        
        -- 4. Создаём новый индекс только по позиции сообщения
        CREATE INDEX IF NOT EXISTS idx_msg_pos ON chat(message_position);
    """)
    conn.close()
    print("✅ Колонка `chat_id` успешно удалена, индекс обновлён!")
except Exception as e:
    print(f"❌ Ошибка: {e}")
    conn.rollback()
    conn.close()