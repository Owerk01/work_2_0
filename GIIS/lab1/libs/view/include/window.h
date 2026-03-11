#pragma once
#include "data_handler.h"
#include "debugger.h"
#include "vars.h"
#include <QMainWindow>
#include <QWidget>

class MainWindow : public QMainWindow {
public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

public slots:
  void show_notification(const QString &text = "Dummy text",
                         int fade_length = 1500);
  void on_info();
  void on_help();
  void on_px_size_setting();
  void on_clicked_px(Point px);

signals:
private:
  Debugger *debugger;
  DataHandler *data_handler;
};