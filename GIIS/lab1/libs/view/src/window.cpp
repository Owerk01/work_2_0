#include "window.h"
#include "canvas.h"
#include "data_handler.h"
#include "debugger.h"
#include "vars.h"
#include <QAction>
#include <QBoxLayout>
#include <QCheckBox>
#include <QDebug>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QScrollArea>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("Editor");
  resize(INIT_WIDTH, INIT_HEIGHT);
  setMaximumWidth(MAX_WINDOW_WIDTH);
  setMaximumHeight(MAX_WINDOW_HEIGHT);
  setMinimumWidth(MIN_WINDOW_WIDTH);
  setMinimumHeight(MIN_WINDOW_HEIGHT);

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

  scroll_area->setWidget(canvas);

  // Manage working area inside frame
  QVBoxLayout *frame_layout = new QVBoxLayout(border_frame);
  frame_layout->setContentsMargins(CELL, CELL, CELL, CELL);
  frame_layout->addWidget(scroll_area);

  main_view->addWidget(border_frame);

  this->debugger = new Debugger(canvas);

  // Init menu bar
  QMenu *file_menu = menuBar()->addMenu("Info");
  QMenu *settings_menu = menuBar()->addMenu("Settings");

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

  QToolButton *frl_btn = new QToolButton(tool_bar);
  frl_btn->setText("FRLine");
  frl_btn->setToolTip("Various first rank line drawing algorithms");
  frl_btn->setMaximumWidth(4 * CELL);

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
  QToolButton *srl_btn = new QToolButton(tool_bar);
  srl_btn->setText("SRLine");
  srl_btn->setToolTip("Various second rank line drawing algorithms");
  srl_btn->setMaximumWidth(4 * CELL);

  QMenu *srl_menu = new QMenu(srl_btn);
  srl_menu->addAction("Circle")->setData(0);
  srl_menu->addAction("Elipsis")->setData(1);
  srl_menu->addAction("Parabola")->setData(2);
  srl_menu->addAction("Hyperbola")->setData(3);

  srl_btn->setMenu(srl_menu);
  srl_btn->setPopupMode(QToolButton::InstantPopup);
  //

  tool_bar->addWidget(frl_btn);
  connect(frl_btn, &QToolButton::clicked, this, [this, canvas]() {
    FRLDataHandler frld;
    this->debugger->set_points(frld.get_points());
    this->debugger->begin_debug();
  });

  tool_bar->addWidget(srl_btn);
  connect(srl_menu, &QMenu::triggered, this, [this](QAction *act) {
    int id = act->data().toInt();
    SRLDataHandler srld(id);
    this->debugger->set_points(srld.get_points());
    this->debugger->begin_debug();
  });

  tool_bar->addWidget(clear_canvas_btn);
  connect(clear_canvas_btn, &QToolButton::clicked, canvas, [this, canvas]() {
    canvas->on_clear();
    bool d = this->debugger->get_debug();
    this->debugger->reset();
    this->debugger->set_debug(d);
  });

  tool_bar->addWidget(debug_checkbox);
  connect(debug_checkbox, &QCheckBox::toggled, this, [this](bool checked) {
    if (checked) {
      qDebug() << "Debug ON";
      this->debugger->reset();
      this->debugger->set_debug(true);
    } else {
      qDebug() << "Debug OFF";
      this->debugger->reset();
      this->debugger->set_debug(false);
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
    this->debugger->reset();
    this->debugger->set_debug(d);
  });
}

MainWindow::~MainWindow() { delete this->debugger; }

void MainWindow::on_info() {
  QMessageBox::information(this, "About",
                           "Awesome Editor created by Owerk and Glentas (and "
                           "Qwen)\n"
                           "C++ and Qt6.10.2\n"
                           "Version 1.1\n"
                           "2026");
}

void MainWindow::on_help() {
  QMessageBox::information(this, "Help", "Nothing here yet :3");
}