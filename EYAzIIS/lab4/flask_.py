import os
from flask import Flask, request, render_template, redirect, url_for, flash
from processor import Parser, SQLhelper, DBManager
from data import DB_DIR
from werkzeug.utils import secure_filename
from datetime import datetime

BASE_DIR = os.path.dirname(__file__)
UPLOAD_FOLDER = os.path.join(BASE_DIR, 'uploads')
if not os.path.exists(UPLOAD_FOLDER):
    os.mkdir(UPLOAD_FOLDER)

app = Flask(__name__)
app.secret_key = 'supersecretkey123'
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER
app.config['MAX_CONTENT_LENGTH'] = 16 * 1024 * 1024

ALLOWED_EXTENSIONS = {'txt', 'rtf', 'doc', 'docx'}

def allowed_file(filename):
    return '.' in filename and filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

def extract_text_from_file(filepath, filename):
    ext = filename.rsplit('.', 1)[1].lower()
    
    if ext == 'txt':
        with open(filepath, 'r', encoding='utf-8') as f:
            return f.read()
    
    elif ext == 'rtf':
        from striprtf.striprtf import rtf_to_text
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        return rtf_to_text(content)
    
    elif ext == 'docx':
        from docx import Document
        try:
            doc = Document(filepath)
            return "\n".join(para.text for para in doc.paragraphs)
        except:
            return None
    
    elif ext == 'doc':
        try:
            import subprocess
            result = subprocess.run(
                ['antiword', filepath],
                capture_output=True,
                text=True,
                timeout=30,
                encoding='utf-8',
                errors='ignore'
            )
            if result.returncode == 0 and result.stdout.strip():
                return result.stdout
            else:
                result = subprocess.run(
                    ['antiword', '-m', 'UTF-8', filepath],
                    capture_output=True,
                    text=True,
                    timeout=30
                )
                if result.returncode == 0:
                    return result.stdout
        except FileNotFoundError:
            print("(!) antiword not installed. Install it: sudo apt install antiword")
        except Exception as e:
            print(f"(!) Error reading .doc: {e}")
        return None
    
    return None

def get_parser():
    return Parser()

def get_sql_helper():
    return SQLhelper()

def get_db_manager():
    return DBManager()

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/parse', methods=['POST'])
def parse():
    text = request.form.get('text', '').strip()
    name = request.form.get('name', '').strip()
    file = request.files.get('file')
    
    if not name:
        name = f"Анализ_{datetime.now().strftime('%Y%m%d_%H%M%S')}"
    
    if file and file.filename and allowed_file(file.filename):
        filename = secure_filename(file.filename)
        filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
        file.save(filepath)
        text = extract_text_from_file(filepath, filename)
        os.remove(filepath)
    
    if not text:
        flash('Пожалуйста, введите текст или загрузите файл!', 'error')
        return redirect(url_for('index'))
    
    try:
        parser = get_parser()
        word_count, duration = parser.parse(text, name)
        flash(f'Успешно обработано! Слов: {word_count}, Время: {duration:.3f} сек. (семантико-синтаксический анализ)', 'success')
    except Exception as e:
        flash(f'Ошибка обработки: {str(e)}', 'error')
        return redirect(url_for('index'))
    
    return redirect(url_for('browse'))

@app.route('/browse')
def browse():
    sql_helper = get_sql_helper()
    analyses = sql_helper.get_all_analysis()
    return render_template('browse.html', analyses=analyses)

@app.route('/view/<int:id>')
def view(id):
    sql_helper = get_sql_helper()
    db_manager = get_db_manager()
    
    filename = sql_helper.get_analysis_filename_by_id(id)
    if not filename:
        flash('Анализ не найден!', 'error')
        return redirect(url_for('browse'))
    
    filepath = os.path.join(DB_DIR, filename)
    text_data = db_manager.load_from_file(filepath)
    
    if not text_data:
        flash('Ошибка загрузки данных!', 'error')
        return redirect(url_for('browse'))
    
    text_name = sql_helper.get_text_name_by_id(id)
    return render_template('view.html', text_data=text_data, text_name=text_name, analysis_id=id)

@app.route('/sentence/<int:analysis_id>/<int:sentence_id>')
def sentence(analysis_id, sentence_id):
    sql_helper = get_sql_helper()
    db_manager = get_db_manager()
    
    filename = sql_helper.get_analysis_filename_by_id(analysis_id)
    if not filename:
        flash('Анализ не найден!', 'error')
        return redirect(url_for('browse'))
    
    filepath = os.path.join(DB_DIR, filename)
    text_data = db_manager.load_from_file(filepath)
    
    if not text_data or sentence_id >= len(text_data.sentences):
        flash('Предложение не найдено!', 'error')
        return redirect(url_for('view', id=analysis_id))
    
    sentence_obj = text_data.sentences[sentence_id]
    return render_template('sentence.html', sentence=sentence_obj, analysis_id=analysis_id, sentence_id=sentence_id)

@app.route('/delete/<int:id>')
def delete(id):
    db_manager = get_db_manager()
    db_manager.delete_analysis_text(id)
    flash('Анализ успешно удалён!', 'success')
    return redirect(url_for('browse'))

@app.route('/edit/<int:id>', methods=['GET', 'POST'])
def edit(id):
    sql_helper = get_sql_helper()
    db_manager = get_db_manager()
    
    filename = sql_helper.get_analysis_filename_by_id(id)
    if not filename:
        flash('Анализ не найден!', 'error')
        return redirect(url_for('browse'))
    
    text_name = sql_helper.get_text_name_by_id(id)
    text_data = db_manager.load_from_file(os.path.join(DB_DIR, filename))
    
    if not text_data:
        flash('Ошибка загрузки данных!', 'error')
        return redirect(url_for('browse'))
    
    current_text = " ".join(sent.text for sent in text_data.sentences)
    
    if request.method == 'POST':
        new_name = request.form.get('name', '').strip()
        new_text = request.form.get('text', '').strip()
        
        if not new_name:
            new_name = text_name
        if not new_text:
            flash('Текст не может быть пустым!', 'error')
            return render_template('edit.html', text_name=text_name, current_text=current_text, analysis_id=id)
        
        try:
            if new_text != current_text:
                parser = get_parser()
                text_data, word_count, duration = parser.parse_text_only(new_text)
                
                if text_data:
                    db_manager.update_existing_analysis(text_data, id, filename, new_name)
                    sql_helper.save_parsing_stat(word_count, duration)
                    flash(f'Текст обновлён! Слов: {word_count}, Время: {duration:.3f} сек.', 'success')
                else:
                    flash('Ошибка парсинга текста!', 'error')
                    return render_template('edit.html', text_name=text_name, current_text=current_text, analysis_id=id)
            else:
                if new_name != text_name:
                    sql_helper.update_analysis_name(id, new_name)
                flash('Название обновлено!', 'success')
            
        except Exception as e:
            flash(f'Ошибка при обновлении: {str(e)}', 'error')
            return render_template('edit.html', text_name=text_name, current_text=current_text, analysis_id=id)
        
        return redirect(url_for('view', id=id))
    
    return render_template('edit.html', text_name=text_name, current_text=current_text, analysis_id=id)

@app.route('/stats')
def stats():
    sql_helper = get_sql_helper()
    stats_data = sql_helper.get_all_stats()
    return render_template('stats.html', stats=stats_data)

@app.route('/help')
def help():
    return render_template('help.html')

if __name__ == '__main__':
    app.run(debug=True, port=5000, threaded=True)
    