#include "dynamic_dia.h"
#include "vars.h"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QString>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

DynamicDialogue::DynamicDialogue(QWidget *parent, std::string name,
                                 std::vector<std::string> spin_names,
                                 std::string combo_box_name,
                                 std::vector<std::string> combo_box_options)
    : QDialog(parent) {

  setWindowTitle(QString::fromStdString(name));

  for (auto e : spin_names) {
    this->spin_boxes[e] = new QSpinBox(this);
    this->spin_boxes[e]->setRange(0, CANVAS_SIZE - 1);
    this->spin_boxes[e]->setValue(0);
  }

  this->combo_box = new QComboBox(this);
  for (auto e : combo_box_options) {
    this->combo_box->addItem(QString::fromStdString(e));
  }

  QFormLayout *layout = new QFormLayout(this);

  for (auto e : this->spin_boxes) {
    layout->addRow(QString::fromStdString(e.first), e.second);
  }

  layout->addRow(QString::fromStdString(combo_box_name), this->combo_box);

  QDialogButtonBox *btn_box = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(btn_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(btn_box, &QDialogButtonBox::rejected, this, [this]() {
    for (auto &e : this->spin_boxes) {
      e.second->setValue(CODE_ERROR);
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