import os
from flask import Flask, request, render_template, redirect, url_for, flash
from processor import Parser, SQLhelper, CorpusHandler
from werkzeug.utils import secure_filename
from datetime import datetime

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
    filename = ""
    
    # Обработка файла или текста
    file = request.files.get('file')
    input_text = request.form.get('text', '').strip()
    
    if file and file.filename != '':
        # Загрузка файла
        filename = secure_filename(file.filename)
        filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
        file.save(filepath)
        
        handler = CorpusHandler()
        content = handler.parser.extract_text(filepath, filename)
        
    elif input_text:
        # Сохранение вручную введённого текста в файл
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        safe_name = secure_filename(name) if name else "manual"
        filename = f"manual_{safe_name}_{timestamp}.txt"
        filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
        
        try:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(input_text)
            content = input_text
        except Exception as e:
            flash(f"Ошибка сохранения текста: {e}", "error")
            return redirect(url_for('index'))
            
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

@app.route('/corpus/view/<int:text_id>')
def view_text(text_id):
    helper = SQLhelper()
    text_data = helper.get_text_by_id(text_id)
    if not text_data:
        flash("Текст не найден!", "error")
        return redirect(url_for('corpus_list'))
    return render_template('view_text.html', text=text_data)

@app.route('/corpus/delete/<int:text_id>')
def delete_text(text_id):
    helper = SQLhelper()
    try:
        # Получаем имя файла перед удалением
        text_data = helper.get_text_by_id(text_id)
        if text_data:
            filename = text_data[1]
            filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
            # Удаляем файл с диска
            if os.path.exists(filepath):
                os.remove(filepath)
        
        helper.delete_corpus_text(text_id)
        flash("Текст успешно удален из корпуса!", "success")
    except Exception as e:
        flash(f"Ошибка удаления: {e}", "error")
    return redirect(url_for('corpus_list'))

@app.route('/corpus/edit/<int:text_id>', methods=['GET', 'POST'])
def edit_text(text_id):
    helper = SQLhelper()
    text_data = helper.get_text_by_id(text_id)
    
    if not text_data:
        flash("Текст не найден!", "error")
        return redirect(url_for('corpus_list'))
    
    if request.method == 'POST':
        author = request.form.get('author', 'Unknown').strip()
        name = request.form.get('name', 'Untitled').strip()
        year = request.form.get('year', 2024)
        source = request.form.get('source', 'User Upload').strip()
        genre = request.form.get('genre', 'General').strip()
        style = request.form.get('style', 'Written').strip()
        subject_area = request.form.get('subject_area', 'General').strip()
        
        content = ""
        filename = text_data[1]  # Сохраняем старое имя файла по умолчанию
        
        file = request.files.get('file')
        input_text = request.form.get('text', '').strip()
        
        if file and file.filename != '':
            # Новый файл загружен
            filename = secure_filename(file.filename)
            filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
            file.save(filepath)
            
            handler = CorpusHandler()
            content = handler.parser.extract_text(filepath, filename)
            
        elif input_text:
            # Текст введён вручную - сохраняем в файл
            timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
            safe_name = secure_filename(name) if name else "manual"
            filename = f"manual_{safe_name}_{timestamp}.txt"
            filepath = os.path.join(app.config['UPLOAD_FOLDER'], filename)
            
            try:
                with open(filepath, 'w', encoding='utf-8') as f:
                    f.write(input_text)
                content = input_text
            except Exception as e:
                flash(f"Ошибка сохранения текста: {e}", "error")
                return render_template('edit_text.html', text=text_data)
        else:
            flash("Не был введен текст и не был прикреплен файл!", "error")
            return render_template('edit_text.html', text=text_data)

        if not content.strip():
            flash("Файл или текст не содержат слов!", "error")
            return render_template('edit_text.html', text=text_data)

        handler = CorpusHandler()
        meta = (filename, author, name, int(year), source, genre, style, subject_area)
        
        try:
            # Удаляем старый файл с диска
            old_filepath = os.path.join(app.config['UPLOAD_FOLDER'], text_data[1])
            if os.path.exists(old_filepath) and old_filepath != filepath:
                os.remove(old_filepath)
            
            word_count, duration = handler.edit_text_in_corpus(text_id, meta, content)
            flash(f"Текст успешно обновлен! Слов: {word_count}, время: {duration:.3f} сек.", "success")
        except Exception as e:
            flash(f"Ошибка обработки: {e}", "error")
            
        return redirect(url_for('corpus_list'))
    
    # GET запрос - показываем форму с заполненными данными
    return render_template('edit_text.html', text=text_data)

@app.route('/browse/', methods=['GET', 'POST'])
def browse():
    helper = SQLhelper()
    data = []
    if request.method == 'POST':
        lemma = request.form.get('lemma', '').strip() or None
        form = request.form.get('form', '').strip() or None
        pos = request.form.get('pos', '').strip() or None
        data = helper.search_words(lemma=lemma, form=form, pos=pos)
    else:
        data = helper.search_words()
    
    return render_template('browse.html', data=data)

@app.route('/lexeme/edit/<int:lexeme_id>', methods=['GET', 'POST'])
def edit_lexeme(lexeme_id):
    helper = SQLhelper()
    lexeme = helper.get_lexeme_by_id(lexeme_id)
    
    if not lexeme:
        flash("Лексема не найдена!", "error")
        return redirect(url_for('browse'))
    
    if request.method == 'POST':
        new_pos = request.form.get('part_of_speech', lexeme[4]).strip()
        new_role = request.form.get('role', lexeme[5]).strip()
        
        try:
            helper.update_lexeme_pos_role(lexeme_id, new_pos, new_role)
            flash("Лексема успешно отредактирована! Частота старой записи уменьшена, создана новая.", "success")
        except Exception as e:
            flash(f"Ошибка редактирования: {e}", "error")
        
        return redirect(url_for('browse'))
    
    return render_template('edit_lexeme.html', lexeme=lexeme)

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

if __name__ == '__main__':
    app.run(debug=True)