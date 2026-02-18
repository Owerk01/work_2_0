#pragma once
#include "debugger.h"
#include <QMainWindow>
#include <QWidget>

class MainWindow : public QMainWindow {
public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

public slots:
  void on_info();
  void on_help();
  void on_px_size_setting();

signals:
private:
  Debugger *debugger;
};