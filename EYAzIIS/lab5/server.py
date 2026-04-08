import os
import time
from flask import Flask, request, jsonify, send_from_directory
from dotenv import load_dotenv
from openai import OpenAI
from sql_helper import SQLhelper
from prompt import HOBBY_SYSTEM_PROMPT

load_dotenv()
app = Flask(__name__, static_folder='.', static_url_path='')
helper = SQLhelper()

@app.route('/')
def index():
    return send_from_directory('.', 'index.html')

@app.route('/style.css')
def css():
    return send_from_directory('.', 'style.css', mimetype='text/css')

@app.route('/script.js')
def js():
    return send_from_directory('.', 'script.js', mimetype='application/javascript')

def get_client():
    api_key = os.getenv('QWEN_API_KEY')
    base_url = os.getenv('QWEN_API_URL', 'https://api.scitely.com/v1')
    return OpenAI(api_key=api_key, base_url=base_url)

@app.route('/api/chat', methods=['POST'])
def chat():
    data = request.json or {}
    user_query = data.get('query', '').strip()
    if not user_query:
        return jsonify({'error': 'query required'}), 400

    messages = [
        {"role": "system", "content": HOBBY_SYSTEM_PROMPT},
        {"role": "user", "content": user_query}
    ]

    start_time = time.time()

    if not os.getenv('QWEN_API_KEY'):

        bot_reply = "API key is not accessible"
    else:
        try:
            client = get_client()
            response = client.chat.completions.create(
                model="qwen3-32b",
                messages=messages,
                temperature=0.2,
                max_tokens=500
            )
            bot_reply = response.choices[0].message.content.strip()
        except Exception as e:
            print(f"API Error: {type(e).__name__}: {e}")
            return jsonify({
                'error': 'Failed to get response',
                'details': str(e)[:150]
            }), 502

    duration = time.time() - start_time
    pos = helper.save_message_pair(user_query, bot_reply)
    
    helper.save_response_duration(duration)
    avg_duration = helper.get_average_duration()
    print(f"Current: {duration:.2f}s | Avg: {avg_duration:.3f}s")

    return jsonify({'response': bot_reply, 'position': pos})

@app.route('/api/chat', methods=['GET'])
def get_chat():
    messages = helper.get_all_chat_messages()
    formatted = []
    for pos, query, response in messages:
        formatted.append({'sender': 'user', 'text': query, 'position': pos})
        formatted.append({'sender': 'system', 'text': response, 'position': pos})
    return jsonify(formatted)

@app.route('/api/chat', methods=['DELETE'])
def delete_chat():
    helper.delete_chat()
    return jsonify({'status': 'deleted'})

@app.route('/api/chat/message/<int:position>', methods=['PATCH'])
def edit_message(position):
    data = request.json or {}
    role = data.get('role')
    new_text = data.get('text', '').strip()
    if role == 'user':
        helper.update_turn(position, query=new_text)
    elif role == 'system':
        helper.update_turn(position, response=new_text)
    return jsonify({'status': 'updated'})

@app.route('/api/chat/message/<int:position>', methods=['DELETE'])
def delete_message(position):
    helper.delete_turn(position)
    return jsonify({'status': 'deleted'})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)