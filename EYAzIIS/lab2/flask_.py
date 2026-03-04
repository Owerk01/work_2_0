import os
from flask import Flask, request, render_template, redirect, url_for, flash
from processor import Parser, SQLhelper, CorpusHandler
from striprtf.striprtf import rtf_to_text

BASE_DIR = os.path.dirname(__file__)
UPLOAD_FOLDER = os.path.join(BASE_DIR, 'uploads')

if not os.path.exists(UPLOAD_FOLDER):
    os.mkdir(UPLOAD_FOLDER)
    print(f"(?) Created folder {UPLOAD_FOLDER}/")

app = Flask(__name__)
app.secret_key = 'supersecretkey123'

@app.route('/')
def index():
    db = SQLhelper()
    return render_template('index.html')


@app.route('/parse/', methods=['POST'])
def parse() -> Response:
    input_ = request.form.get('text', '').strip()
    file = request.files.get('file')

    # Тут придется поебаться с меню, нужно заполнять форму
    filename = "" 
    author = "" 
    name = "" 
    year = 0 
    source = "" 
    genre = "" 
    style = "" 
    content = ""

    if input_:
        content = input_
    elif file and file.filename != '':
        if not file.filename.endswith(('.txt', '.rtf')):
            flash("Работа осуществляется только с файлами форматов .txt и .rtf!", "error")
            return redirect(url_for('index'))
        filepath = os.path.join(UPLOAD_FOLDER, file.filename)
        file.save(filepath)
        try:
            if file.filename.endswith('.txt'):
                with open(filepath, 'r', encoding='utf-8') as f:
                    content = f.read()
            elif file.filename.endswith('.rtf'):
                with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                    rtf_content = f.read()
                content = rtf_to_text(rtf_content)
        except Exception as e:
            flash(f"Ошибка чтения файла: {e}", "error")
            return redirect(url_for('index'))
    else:
        flash("Не был введён текст и не был прикреплён файл!", "error")
        return redirect(url_for('index'))

    if not content.strip():
        flash("Прикреплённый файл не содержит текст!", "error")
        return redirect(url_for('index'))

    # TODO: фиксим под новую модель
    corpus = CorpusHandler()
    word_count, duration = corpus.add_text_to_corpus(filename, author, name, year, source, genre, style, content)  
    flash(f"Текст успешно обработан! Слов: {word_count}, время: {duration:.3f} сек.", "success")
    return redirect(url_for('browse'))

# Тут все ок
@app.route('/stats/')
def stats():
    helper = SQLhelper()
    stats_data = helper.get_all_stats() 
    return render_template('stats.html', stats=stats_data)

# Вроде ок
@app.route('/browse/', methods=['GET', 'POST'])
def browse():
    helper = SQLhelper()
    data = []

    if request.method == 'POST':
        lemma = request.form.get('lemma', '').strip() or None
        form = request.form.get('form', '').strip() or None
        pos = request.form.get('pos', '').strip() or None
        role = request.form.get('role', '').strip() or None

        try:
            data = helper.search(lemma=lemma, form=form, pos=pos, role=role)
        except Exception as e:
            flash(f"Ошибка поиска: {e}", "error")
            data = []
    else:
        data = helper.select_all()

    return render_template('browse.html', data=data)

# Тут конечно сложно... frequency надо просто костылить =1, Предлагать еблану выбрать текст, к которому будет принадлежать слово...ArithmeticError
# Но наверно это лучше убрать, нельзя добавлять новые, можно только фиксить существующие
# Типа идея в том что у нас текст содержит слова и они связаны с текстом железно
# Поэтому можно только изменять слова, а удаление и добавление можно менять ТОЛЬКО при изменении исходного текста
# то есть хотим добавить слово -> оно должно появиться в тексте
@app.route('/add/', methods=['GET', 'POST'])
def add():
    helper = SQLhelper()
    if request.method == 'POST':
        lemma = request.form['lemma'].strip()
        form = request.form['form'].strip()
        pos = request.form['pos'].strip()
        role = request.form['role'].strip()

        if not all([lemma, form]):
            flash("Lemma и Form обязательны!", "error")
            return render_template('edit.html', entry=None)

        try:
            helper.insert(lemma=lemma, form=form, pos=pos, role=role)
            flash("Запись добавлена!", "success")
            return redirect(url_for('browse'))
        except Exception as e:
            flash(f"Что-то пошло не по плану: {e}", "error")

    return render_template('edit.html', entry=None)

# После изменения костыль frequency = 1
# Нельзя менять словоформу и лемму, только role и pos (lemma & form меняется при редактировании текста)
@app.route('/edit/<int:id>/', methods=['GET', 'POST'])
def edit(id):
    helper = SQLhelper()

    if request.method == 'POST':
        lemma = request.form['lemma'].strip()
        form = request.form['form'].strip()
        pos = request.form['pos'].strip()
        role = request.form['role'].strip()

        if not all([lemma, form]):
            flash("Lemma и Form обязательны!", "error")
            entry = helper.get_by_id(id)
            if not entry:
                flash("Запись не найдена!", "error")
                return redirect(url_for('browse'))
            return render_template('edit.html', entry=entry)

        try:
            helper.update(id=id, lemma=lemma, form=form, pos=pos, role=role)
            flash("Запись обновлена!", "success")
            return redirect(url_for('browse'))
        except Exception as e:
            flash(f"Ошибка: {e}", "error")
            entry = helper.get_by_id(id)
            return render_template('edit.html', entry=entry)

    entry = helper.get_by_id(id)
    if not entry:
        flash("Запись не найдена!", "error")
        return redirect(url_for('browse'))

    return render_template('edit.html', entry=entry)

# Наверное стоит убрать
@app.route('/delete/<int:id>/')
def delete(id):
    helper = SQLhelper()
    try:
        helper.delete(id)
        flash("Запись удалена!", "success")
    except Exception as e:
        flash(f"Ошибка удаления: {e}", "error")
    return redirect(url_for('browse'))

@app.route('/help/')
def help():
    return render_template('help.html')

# Страница со списком всех текстов и инфы про них
# добавить изменение текста (просто заполнять форму как в def parse() выше)
# подтягиваешь данные в форму, предлагаешь изменить бла бла затем:
# corpus = CorpusHandler()
# corpus.edit_corp_text(...)

if __name__ == '__main__':
    app.run(debug=True)