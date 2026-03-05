import os
from flask import Flask, request, render_template, redirect, url_for, flash
from processor import Parser, SQLhelper, CorpusHandler
from werkzeug.utils import secure_filename

BASE_DIR = os.path.dirname(__file__)
UPLOAD_FOLDER = os.path.join(BASE_DIR, 'uploads')
if not os.path.exists(UPLOAD_FOLDER):
    os.mkdir(UPLOAD_FOLDER)

app = Flask(__name__)
app.secret_key = 'supersecretkey123'
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/parse/', methods=['POST'])
def parse():
    # Сбор метаданных из формы
    author = request.form.get('author', 'Unknown').strip()
    name = request.form.get('name', 'Untitled').strip()
    year = request.form.get('year', 2024)
    source = request.form.get('source', 'User Upload').strip()
    genre = request.form.get('genre', 'General').strip()
    style = request.form.get('style', 'Written').strip()
    subject_area = request.form.get('subject_area', 'General').strip()
    
    content = ""
    filename = "manual_input.txt"
    
    # Обработка файла или текста
    file = request.files.get('file')
    input_text = request.form.get('text', '').strip()
    
    if file and file.filename != '':
        filename = secure_filename(file.filename)
        filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
        file.save(filepath)
        
        handler = CorpusHandler()
        content = handler.parser.extract_text(filepath, filename)
        # Удаляем файл после обработки для экономии места (опционально)
        # os.remove(filepath) 
    elif input_text:
        content = input_text
    else:
        flash("Не был введен текст и не был прикреплен файл!", "error")
        return redirect(url_for('index'))

    if not content.strip():
        flash("Файл или текст не содержат слов!", "error")
        return redirect(url_for('index'))

    handler = CorpusHandler()
    meta = (filename, author, name, int(year), source, genre, style, subject_area)
    
    try:
        word_count, duration = handler.add_text_to_corpus(meta, content)
        flash(f"Текст успешно добавлен в корпус! Слов: {word_count}, время: {duration:.3f} сек.", "success")
    except Exception as e:
        flash(f"Ошибка обработки: {e}", "error")
        
    return redirect(url_for('corpus_list'))

@app.route('/corpus/')
def corpus_list():
    helper = SQLhelper()
    texts = helper.get_all_texts()
    return render_template('corpus_list.html', texts=texts)

@app.route('/browse/', methods=['GET', 'POST'])
def browse():
    helper = SQLhelper()
    data = []
    if request.method == 'POST':
        lemma = request.form.get('lemma', '').strip() or None
        form = request.form.get('form', '').strip() or None
        pos = request.form.get('pos', '').strip() or None
        # Поиск по всему корпусу или конкретному тексту можно добавить позже
        data = helper.search_words(lemma=lemma, form=form, pos=pos)
    else:
        data = helper.search_words() # Первые 100 записей
    
    return render_template('browse.html', data=data)

@app.route('/concordance/<int:text_id>/<word>')
def concordance(text_id, word):
    handler = CorpusHandler()
    matches = handler.parser.get_concordance(text_id, word)
    return render_template('concordance.html', matches=matches, word=word, text_id=text_id)

@app.route('/stats/')
def stats():
    helper = SQLhelper()
    stats_data = helper.get_stats()
    corp_stats = helper.get_corpus_stats()
    return render_template('stats.html', stats=stats_data, corp_stats=corp_stats)

@app.route('/help/')
def help():
    return render_template('help.html')

# Ручное редактирование слов отключено согласно требованиям целостности корпуса
# @app.route('/add/') и /delete/ удалены или скрыты

if __name__ == '__main__':
    app.run(debug=True)