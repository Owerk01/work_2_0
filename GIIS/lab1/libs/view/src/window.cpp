#include "window.h"
#include "canvas.h"
#include "data_handler.h"
#include "debugger.h"
#include "vars.h"
#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QDebug>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollArea>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>
#include <iostream>
#include <qnamespace.h>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {

  this->setWindowTitle("Editor");
  this->resize(INIT_WIDTH, INIT_HEIGHT);
  this->setMaximumWidth(MAX_WINDOW_WIDTH);
  this->setMaximumHeight(MAX_WINDOW_HEIGHT);
  this->setMinimumWidth(MIN_WINDOW_WIDTH);
  this->setMinimumHeight(MIN_WINDOW_HEIGHT);

  // Init working area
  // The very "parent" widget
  QWidget *central = new QWidget(this);
  setCentralWidget(central);

  // Manage global working area
  QHBoxLayout *main_view = new QHBoxLayout(central);

  // Frame with canvas
  QFrame *border_frame = new QFrame(central);

  QScrollArea *scroll_area = new QScrollArea(border_frame);
  scroll_area->setWidgetResizable(false);

  Canvas *canvas = new Canvas(scroll_area);
  connect(canvas, &Canvas::clicked_px, this, &MainWindow::on_clicked_px);
  scroll_area->setWidget(canvas);

  // Manage working area inside frame
  QVBoxLayout *frame_layout = new QVBoxLayout(border_frame);
  frame_layout->setContentsMargins(CELL, CELL, CELL, CELL);
  frame_layout->addWidget(scroll_area);

  main_view->addWidget(border_frame);

  this->debugger = new Debugger(canvas);
  this->data_handler = new DataHandler(this->debugger);

  // Init menu bar
  QMenu *file_menu = this->menuBar()->addMenu("Info");
  QMenu *settings_menu = this->menuBar()->addMenu("Settings");

  QAction *program_help = new QAction("Help", this);
  connect(program_help, &QAction::triggered, this, &MainWindow::on_help);

  QAction *program_about = new QAction("About", this);
  connect(program_about, &QAction::triggered, this, &MainWindow::on_info);

  QAction *grid_size_setting = new QAction("Cell size", this);
  connect(grid_size_setting, &QAction::triggered, canvas,
          &Canvas::on_size_update);

  QAction *grid_show_setting = new QAction("Show grid", this);
  grid_show_setting->setCheckable(true);
  grid_show_setting->setChecked(true);
  grid_show_setting->setToolTip("Enable/disable grid");
  connect(grid_show_setting, &QAction::toggled, canvas, &Canvas::on_grid_show);

  file_menu->addAction(program_help);
  file_menu->addAction(program_about);

  settings_menu->addAction(grid_size_setting);
  settings_menu->addAction(grid_show_setting);

  // Init toolbar
  QToolBar *tool_bar = addToolBar("Tools");
  tool_bar->addSeparator();

  QToolButton *clear_canvas_btn = new QToolButton(tool_bar);
  clear_canvas_btn->setText("Clear");
  clear_canvas_btn->setToolTip(
      "Clears workspace by filling it with white color");
  clear_canvas_btn->setMaximumWidth(4 * CELL);

  QCheckBox *debug_checkbox = new QCheckBox("Debug", tool_bar);
  debug_checkbox->setToolTip("Debug mode");
  debug_checkbox->setChecked(false);

  QToolButton *debug_step_btn = new QToolButton(tool_bar);
  debug_step_btn->setText(">");
  debug_step_btn->setEnabled(false);

  QToolButton *debug_stop_btn = new QToolButton(tool_bar);
  debug_stop_btn->setText("x");
  debug_stop_btn->setEnabled(false);

  // button with popup, goddamn
  QToolButton *frl_btn = new QToolButton(tool_bar);
  frl_btn->setText("FRLine");
  frl_btn->setToolTip("Various first rank line drawing algorithms");
  frl_btn->setMaximumWidth(4 * CELL);

  QMenu *frl_menu = new QMenu(frl_btn);
  frl_menu->addAction("CDA")->setData(static_cast<int>(GType::CDA));
  frl_menu->addAction("Bresenham")->setData(static_cast<int>(GType::Bresenham));
  frl_menu->addAction("Wu")->setData(static_cast<int>(GType::Wu));

  frl_btn->setMenu(frl_menu);
  frl_btn->setPopupMode(QToolButton::InstantPopup);
  //

  // button with popup, goddamn x2
  QToolButton *srl_btn = new QToolButton(tool_bar);
  srl_btn->setText("SRLine");
  srl_btn->setToolTip("Various second rank line drawing algorithms");
  srl_btn->setMaximumWidth(4 * CELL);

  QMenu *srl_menu = new QMenu(srl_btn);
  srl_menu->addAction("Circle")->setData(static_cast<int>(GType::Circle));
  srl_menu->addAction("Elipsis")->setData(static_cast<int>(GType::Elipsis));
  srl_menu->addAction("Parabola")->setData(static_cast<int>(GType::Parabola));
  srl_menu->addAction("Hyperbola")->setData(static_cast<int>(GType::Hyperbola));

  srl_btn->setMenu(srl_menu);
  srl_btn->setPopupMode(QToolButton::InstantPopup);
  //

  // button with popup, goddamn x3
  QToolButton *arl_btn = new QToolButton(tool_bar);
  arl_btn->setText("ARLine");
  arl_btn->setToolTip("Various interpolation algorithms");
  arl_btn->setMaximumWidth(4 * CELL);

  QMenu *arl_menu = new QMenu(arl_btn);
  arl_menu->addAction("Hermite")->setData(static_cast<int>(GType::Hermit));
  arl_menu->addAction("Bezier")->setData(static_cast<int>(GType::Bezier));
  arl_menu->addAction("BSpline")->setData(static_cast<int>(GType::BSpline));

  arl_btn->setMenu(arl_menu);
  arl_btn->setPopupMode(QToolButton::InstantPopup);
  //

  tool_bar->addWidget(frl_btn);
  connect(frl_menu, &QMenu::triggered, this, [this](QAction *act) {
    int id = act->data().toInt();
    this->show_notification("Line: start, end", 7500);
    this->data_handler->set_figure({static_cast<GType>(id)});
  });

  tool_bar->addWidget(srl_btn);
  connect(srl_menu, &QMenu::triggered, this, [this](QAction *act) {
    int id = act->data().toInt();
    this->show_notification("Circle: center, radius\n"
                            "Elipsis: center, a (x axis), b (y axis)",
                            10000);
    this->data_handler->set_figure({static_cast<GType>(id)});
  });

  tool_bar->addWidget(arl_btn);
  connect(arl_menu, &QMenu::triggered, this, [this](QAction *act) {
    int id = act->data().toInt();
    this->show_notification("Hermite: start, force1, end, force2\n"
                            "Bezier: start, magnet1, magnet2, end\n"
                            "BSpline: 8 point",
                            10000);
    this->data_handler->set_figure({static_cast<GType>(id)});
  });

  //
  tool_bar->addWidget(clear_canvas_btn);
  connect(clear_canvas_btn, &QToolButton::clicked, canvas, [this, canvas]() {
    canvas->on_clear();
    this->show_notification("Cleared canvas");
    bool d = this->debugger->get_debug();
    this->debugger->reset();
    this->data_handler->reset();
    this->debugger->set_debug(d);
  });

  tool_bar->addWidget(debug_checkbox);
  connect(debug_checkbox, &QCheckBox::toggled, this, [this](bool checked) {
    if (checked) {
      qDebug() << "Debug ON";
      this->show_notification("Debug ON");
      this->debugger->reset();
      this->debugger->set_debug(true);
    } else {
      qDebug() << "Debug OFF";
      this->show_notification("Debug OFF");
      this->debugger->set_debug(false);
      this->debugger->begin_debug();
    }
  });

  tool_bar->addWidget(debug_step_btn);
  connect(debug_checkbox, &QCheckBox::toggled, debug_step_btn,
          &QToolButton::setEnabled);
  connect(debug_step_btn, &QToolButton::clicked, this,
          [this]() { this->debugger->begin_debug(); });

  tool_bar->addWidget(debug_stop_btn);
  connect(debug_checkbox, &QCheckBox::toggled, debug_stop_btn,
          &QToolButton::setEnabled);
  connect(debug_stop_btn, &QToolButton::clicked, this, [this]() {
    bool d = this->debugger->get_debug();
    this->debugger->set_debug(false);
    this->debugger->begin_debug();
    this->debugger->set_debug(d);
  });
}

MainWindow::~MainWindow() {
  delete this->debugger;
  delete this->data_handler;
  std::cout << "Window out...\n";
}

void MainWindow::on_info() {
  QMessageBox::information(this, "About",
                           "Awesome Editor created by Owerk and Glentas (and "
                           "Qwen)\n"
                           "C++ and Qt6.10.2\n"
                           "Version 1.2\n"
                           "2026");
}

void MainWindow::on_help() {
  QMessageBox::information(this, "Help", "Nothing here yet :3");
}

void MainWindow::show_notification(const QString &text, int fade_length) {
  QLabel *notification = new QLabel(this);
  notification->setText(text);
  notification->setStyleSheet("QLabel {"
                              "  background-color: #b8e7fe;"
                              "  color: #000000;"
                              "  border: 2px solid #000000;"
                              "  border-radius: 4px;"
                              "  padding: 6px 10px;"
                              "  font-size: 10pt;"
                              "  min-width: 80px;"
                              "  text-align: center;"
                              "}");
  notification->setAlignment(Qt::AlignCenter);
  notification->setAttribute(Qt::WA_DeleteOnClose);
  notification->setWindowFlags(Qt::ToolTip);

  notification->adjustSize();

  QPoint local_pos = this->rect().topRight();
  local_pos.setX(local_pos.x() - notification->width() - 10);
  local_pos.setY(local_pos.y() + 10);

  QPoint global_pos = this->mapToGlobal(local_pos);
  notification->move(global_pos);

  notification->show();
  QTimer::singleShot(fade_length, notification, &QWidget::close);
}

void MainWindow::on_clicked_px(Point px) { this->data_handler->add_point(px); }