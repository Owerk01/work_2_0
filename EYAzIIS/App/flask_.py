import os
from flask import Flask, request, render_template, redirect, url_for, flash
from processor import Parser, SQLhelper
from striprtf.striprtf import rtf_to_text

UPLOAD_FOLDER = 'uploads'

if not os.path.exists(UPLOAD_FOLDER):
    os.mkdir(UPLOAD_FOLDER)
    print(f"(?) Created folder {UPLOAD_FOLDER}/")

app = Flask(__name__)
app.secret_key = 'supersecretkey123'

@app.route('/')
def index():
    db = SQLhelper()
    return render_template('index.html')


@app.route('/parse', methods=['POST'])
def parse():
    input_ = request.form.get('text', '').strip()
    file = request.files.get('file')

    text = ""

    if input_:
        text = input_
    if file:
        if not file.filename.endswith(('.txt', '.rtf')):
            flash("Работа осуществляется только с файлами форматов .txt и .rtf!", "error")
            return redirect(url_for('index'))
        filepath = os.path.join(UPLOAD_FOLDER, file.filename)
        file.save(filepath)
        try:
            if file.filename.endswith('.txt'):
                with open(filepath, 'r', encoding='utf-8') as f:
                    text = f.read()
            elif file.filename.endswith('.rtf'):
                with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                    rtf_content = f.read()
                text += " " + rtf_to_text(rtf_content)
        except Exception as e:
            flash(f"Ошибка чтения файла: {e}", "error")
            return redirect(url_for('index'))
    if text == "":
        flash("Не был введён текст и не был прикреплён файл!", "error")
        return redirect(url_for('index'))

    if not text.strip():
        flash("Прикреплённый файл не содержит текст!", "error")
        return redirect(url_for('index'))

    parser = Parser()
    parser.parse(text)
    flash("Текст успешно обработан и добавлен в словарь!", "success")
    return redirect(url_for('browse'))


@app.route('/browse', methods=['GET', 'POST'])
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


@app.route('/add', methods=['GET', 'POST'])
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


@app.route('/edit/<int:id>', methods=['GET', 'POST'])
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


@app.route('/delete/<int:id>')
def delete(id):
    helper = SQLhelper()
    try:
        helper.delete(id)
        flash("Запись удалена!", "success")
    except Exception as e:
        flash(f"Ошибка удаления: {e}", "error")
    return redirect(url_for('browse'))


if __name__ == '__main__':
    app.run(debug=True)