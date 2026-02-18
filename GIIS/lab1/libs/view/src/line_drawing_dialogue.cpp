#include "line_drawing_dialogue.h"
#include "vars.h"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <iostream>
#include <qdialog.h>
#include <string>

LineDialogue::LineDialogue(QWidget *parent) : QDialog(parent) {
  setWindowTitle("Setup line drawing");

  this->x1 = new QSpinBox(this);
  this->y1 = new QSpinBox(this);
  this->x2 = new QSpinBox(this);
  this->y2 = new QSpinBox(this);

  for (auto *sb : {this->x1, this->y1, this->x2, this->y2}) {
    sb->setRange(0, CANVAS_SIZE - 1);
    sb->setValue(0);
  }

  this->line_algorithm = new QComboBox(this);
  this->line_algorithm->addItems({"CDA", "Bresenham", "Wu"});

  QFormLayout *layout = new QFormLayout(this);
  layout->addRow("x1:", this->x1);
  layout->addRow("y1:", this->y1);
  layout->addRow("x2:", this->x2);
  layout->addRow("y2:", this->y2);
  layout->addRow("Algorithm:", this->line_algorithm);

  QDialogButtonBox *btn_box = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(btn_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(btn_box, &QDialogButtonBox::rejected, this, [this]() {
    this->x1->setValue(CODE_ERROR);
    this->y1->setValue(CODE_ERROR);
    this->x2->setValue(CODE_ERROR);
    this->y2->setValue(CODE_ERROR);
    this->reject();
  });

  layout->addWidget(btn_box);
  setLayout(layout);
}

LineDialogue::~LineDialogue() {
  delete this->x1;
  delete this->y1;
  delete this->x2;
  delete this->y2;
  delete this->line_algorithm;
  std::cout << "LineDialogue out...\n";
}

int LineDialogue::get_x1() const { return x1->value(); }
int LineDialogue::get_y1() const { return y1->value(); }
int LineDialogue::get_x2() const { return x2->value(); }
int LineDialogue::get_y2() const { return y2->value(); }

std::string LineDialogue::get_algorithm() const {
  return line_algorithm->currentText().toStdString();
}