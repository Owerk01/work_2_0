#include "dynamic_dia.h"
#include "vars.h"
#include <QDebug>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionValidator>
#include <QString>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

DynamicDialogue::DynamicDialogue(QWidget *parent, std::string name,
                                 std::vector<std::string> spin_names,
                                 std::string combo_box_name,
                                 std::vector<std::string> combo_box_options,
                                 std::vector<std::string> qline_names,
                                 bool single)
    : QDialog(parent) {

  QRegularExpression re;
  QString tips;

  if (single) {
    re.setPattern(R"(^\(\d+,\d+\)$)");
    tips = "(n,m)";
  } else {
    re.setPattern(R"(^\(\d+,\d+\)(,\(\d+,\d+\))*$)");
    tips = "(n,m),(i,j)...";
  }

  setWindowTitle(QString::fromStdString(name));
  QFormLayout *layout = new QFormLayout(this);

  for (auto e : spin_names) {
    this->spin_boxes[e] = new QSpinBox(this);
    this->spin_boxes[e]->setRange(0, CANVAS_SIZE - 1);
    this->spin_boxes[e]->setValue(0);
    layout->addRow(QString::fromStdString(e), this->spin_boxes[e]);
  }

  for (auto e : qline_names) {
    QLineEdit *edit = new QLineEdit(this);
    edit->setValidator(new QRegularExpressionValidator(re, this));
    edit->setPlaceholderText(tips);
    this->qlines[e] = edit;
    layout->addRow(QString::fromStdString(e), this->qlines[e]);
  }

  if (combo_box_name != "") {
    this->combo_box = new QComboBox(this);
    for (auto e : combo_box_options) {
      this->combo_box->addItem(QString::fromStdString(e));
    }
  }

  if (combo_box_name != "") {
    layout->addRow(QString::fromStdString(combo_box_name), this->combo_box);
  }

  QDialogButtonBox *btn_box = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(btn_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(btn_box, &QDialogButtonBox::rejected, this, [this]() {
    for (auto &e : this->spin_boxes) {
      e.second->setValue(CODE_ERROR);
    }

    for (auto &e : this->qlines) {
      e.second->setText("(" + QString::number(CODE_ERROR) + "," +
                        QString::number(CODE_ERROR) + ")");
    }
    this->reject();
  });

  layout->addWidget(btn_box);
  setLayout(layout);
}

DynamicDialogue::~DynamicDialogue() {
  std::cout << "Dynamic Dialogue out...\n";
}

std::string DynamicDialogue::get_combo_box() const {
  return this->combo_box->currentText().toStdString();
}

std::pair<int, int> DynamicDialogue::get_line_by_name(std::string name) const {
  if (this->qlines.at(name)->text() == "(-1,-1)") {
    return {-1, -1};
  }
  QRegularExpression re(R"(^\((\d+),(\d+)\)$)");
  QRegularExpressionMatch match = re.match(this->qlines.at(name)->text());
  if (!match.hasMatch()) {
    throw std::invalid_argument("Invalid point format");
  }

  int x = match.captured(1).toInt();
  int y = match.captured(2).toInt();
  return {x, y};
}

std::vector<std::pair<int, int>> DynamicDialogue::get_lines() const {
  if (this->qlines.begin()->second->text() == "(-1,-1)") {
    return {{-1, -1}};
  }
  std::vector<std::pair<int, int>> result;

  QRegularExpression re(R"(\((\d+),(\d+)\))");
  QRegularExpressionMatchIterator it =
      re.globalMatch(this->qlines.begin()->second->text());

  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    int x = match.captured(1).toInt();
    int y = match.captured(2).toInt();
    result.emplace_back(x, y);
  }

  return result;
}

int DynamicDialogue::get_spin_by_name(std::string name) const {
  return this->spin_boxes.at(name)->value();
}

std::unordered_map<std::string, int> DynamicDialogue::get_spin_boxes() const {
  std::unordered_map<std::string, int> tmp;
  for (auto e : this->spin_boxes) {
    tmp[e.first] = e.second->value();
  }
  return tmp;
}