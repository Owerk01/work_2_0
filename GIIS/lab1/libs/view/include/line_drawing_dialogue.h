#pragma once
#include <QComboBox>
#include <QDialog>
#include <QSpinBox>
#include <string>

class LineDialogue : public QDialog {
  Q_OBJECT

public:
  LineDialogue(QWidget *parent = nullptr);
  ~LineDialogue();

  int get_x1() const;
  int get_y1() const;
  int get_x2() const;
  int get_y2() const;
  std::string get_algorithm() const;

private:
  QSpinBox *x1, *y1, *x2, *y2;
  QComboBox *line_algorithm;
};