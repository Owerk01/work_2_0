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
    data = request.json
    user_id = data.get('user_id')
    user_query = data.get('query', '').strip()
    
    if not user_id or not user_query:
        return jsonify({'error': 'user_id and query required'}), 400
    
    
    messages = [
        {"role": "system", "content": HOBBY_SYSTEM_PROMPT},
        {"role": "user", "content": user_query}
    ]
    
    
    if not os.getenv('QWEN_API_KEY'):
        hobby_keywords = ['knit', 'chess', 'garden', 'photo', 'paint', 'cook', 'fish', 'hike', 'hobby', 'craft', 'diy']
        if any(kw in user_query.lower() for kw in hobby_keywords):
            bot_reply = f"Great hobby question! (Test mode) Here's a tip about {user_query[:30]}..."
        else:
            bot_reply = "I specialize in hobby topics! Ask me about knitting, chess, birdwatching, or any leisure activity"
        helper.save_message_pair(user_id, user_query, bot_reply)
        return jsonify({'response': bot_reply})
    
    start_time = time.time()
    
    try:
        client = get_client()
        
        response = client.chat.completions.create(
            model="qwen3-32b",  
            messages=messages,
            temperature=0.3,    
            top_p=0.8,
            max_tokens=500
        )
        
        bot_reply = response.choices[0].message.content.strip()
        
        
        helper.save_message_pair(user_id, user_query, bot_reply)
        
        duration = time.time() - start_time
        print(f"Response in {duration:.2f}s for user {user_id}")
        
        return jsonify({'response': bot_reply})
        
    except Exception as e:
        print(f"API Error: {type(e).__name__}: {e}")
        return jsonify({
            'error': 'Failed to get response',
            'details': str(e)[:150]  
        }), 502


@app.route('/api/chat/<int:user_id>', methods=['GET'])
def get_chat(user_id):
    messages = helper.get_all_chat_messages(user_id)
    formatted = []
    for pos, query, response in messages:
        formatted.append({'sender': 'user', 'text': query})
        formatted.append({'sender': 'system', 'text': response})
    return jsonify(formatted)


@app.route('/api/chat/<int:user_id>', methods=['DELETE'])
def delete_chat(user_id):
    helper.delete_chat(user_id)
    return jsonify({'status': 'deleted', 'chat_id': user_id})


@app.route('/api/users', methods=['POST'])
def create_user():
    import uuid
    data = request.json or {}
    user_id = data.get('user_id') or str(uuid.uuid4().int >> 64)
    return jsonify({'user_id': user_id})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)