#pragma once
#include <QComboBox>
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <string>
#include <unordered_map>
#include <vector>

class DynamicDialogue : public QDialog {
  Q_OBJECT

public:
  DynamicDialogue(QWidget *parent = nullptr, std::string name = "Default",
                  std::vector<std::string> spin_names = {},
                  std::string combo_box_name = "",
                  std::vector<std::string> combo_box_options = {},
                  std::vector<std::string> qline_names = {},
                  bool single = true);
  ~DynamicDialogue();

  int get_spin_by_name(std::string name) const;
  std::pair<int, int> get_line_by_name(std::string name) const;

  std::vector<std::pair<int, int>> get_lines() const;
  std::unordered_map<std::string, int> get_spin_boxes() const;
  std::string get_combo_box() const;

private:
  std::unordered_map<std::string, QSpinBox *> spin_boxes;
  std::unordered_map<std::string, QLineEdit *> qlines;
  QComboBox *combo_box;
};