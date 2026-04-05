#include "window.h"
#include "canvas.h"
#include "data_handler.h"
#include "debugger.h"
#include "misc.h"
#include "vars.h"
#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QDebug>
#include <QDialog>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QShortcut>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	this->setWindowTitle("Editor");
	this->resize(INIT_WIDTH, INIT_HEIGHT);
	this->setMaximumWidth(MAX_WINDOW_WIDTH);
	this->setMaximumHeight(MAX_WINDOW_HEIGHT);
	this->setMinimumWidth(MIN_WINDOW_WIDTH);
	this->setMinimumHeight(MIN_WINDOW_HEIGHT);

	// Init working area
	// The very "parent" widget
	QWidget *central = new QWidget(this);
	this->setCentralWidget(central);

	// Manage global working area
	QHBoxLayout *main_view = new QHBoxLayout(central);

	// Frame with canvas
	QFrame *border_frame = new QFrame(central);

	QScrollArea *scroll_area = new QScrollArea(border_frame);
	scroll_area->setWidgetResizable(false);

	Canvas *canvas = new Canvas(scroll_area);
	this->connect(canvas, &Canvas::clicked_px, this,
		      &MainWindow::on_clicked_px);
	scroll_area->setWidget(canvas);

	// Manage working area inside frame
	QVBoxLayout *frame_layout = new QVBoxLayout(border_frame);
	frame_layout->setContentsMargins(CELL, CELL, CELL, CELL);
	frame_layout->addWidget(scroll_area);

	main_view->addWidget(border_frame);

	this->debugger = new Debugger(canvas);
	this->data_handler = new DataHandler(this->debugger, { GType::CDA });

	// Init menu bar
	QMenu *file_menu = this->menuBar()->addMenu("Info");
	QMenu *settings_menu = this->menuBar()->addMenu("Settings");

	QAction *program_help = new QAction("Help", this);
	this->connect(program_help, &QAction::triggered, this,
		      &MainWindow::on_help);

	QAction *program_about = new QAction("About", this);
	this->connect(program_about, &QAction::triggered, this,
		      &MainWindow::on_info);

	QAction *active_figure_setting = new QAction("Set active figure", this);
	this->connect(active_figure_setting, &QAction::triggered, this,
		      &MainWindow::on_show_figures_dialog);

	QAction *grid_size_setting = new QAction("Cell size", this);
	this->connect(grid_size_setting, &QAction::triggered, this,
		      &MainWindow::on_size_update);

	QAction *grid_show_setting = new QAction("Show grid", this);
	grid_show_setting->setCheckable(true);
	grid_show_setting->setChecked(true);
	grid_show_setting->setToolTip("Enable/disable grid");
	this->connect(grid_show_setting, &QAction::toggled, canvas,
		      &Canvas::on_grid_show);

	file_menu->addAction(program_help);
	file_menu->addAction(program_about);

	settings_menu->addAction(active_figure_setting);
	settings_menu->addAction(grid_size_setting);
	settings_menu->addAction(grid_show_setting);

	// Init toolbar
	QToolBar *tool_bar = this->addToolBar("Tools");
	tool_bar->addSeparator();

	// set misc buttons
	QToolButton *clear_canvas_btn = new QToolButton(tool_bar);
	clear_canvas_btn->setText("Clear");
	clear_canvas_btn->setToolTip(
		"Clears workspace by filling it with white color");
	clear_canvas_btn->setMaximumWidth(4 * CELL);

	QToolButton *dot_btn = new QToolButton(tool_bar);
	dot_btn->setText("Dot");
	dot_btn->setToolTip("Sets dot on canvas");
	dot_btn->setMaximumWidth(4 * CELL);

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
	frl_menu->addAction("Bresenham")
		->setData(static_cast<int>(GType::Bresenham));
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
	srl_menu->addAction("Elipsis")->setData(
		static_cast<int>(GType::Elipsis));
	srl_menu->addAction("Parabola")
		->setData(static_cast<int>(GType::Parabola));
	srl_menu->addAction("Hyperbola")
		->setData(static_cast<int>(GType::Hyperbola));

	srl_btn->setMenu(srl_menu);
	srl_btn->setPopupMode(QToolButton::InstantPopup);
	//

	// button with popup, goddamn x3
	QToolButton *arl_btn = new QToolButton(tool_bar);
	arl_btn->setText("ARLine");
	arl_btn->setToolTip("Various interpolation algorithms");
	arl_btn->setMaximumWidth(4 * CELL);

	QMenu *arl_menu = new QMenu(arl_btn);
	arl_menu->addAction("Hermite")->setData(
		static_cast<int>(GType::Hermit));
	arl_menu->addAction("Bezier")->setData(static_cast<int>(GType::Bezier));
	arl_menu->addAction("BSpline")->setData(
		static_cast<int>(GType::BSpline));

	arl_btn->setMenu(arl_menu);
	arl_btn->setPopupMode(QToolButton::InstantPopup);
	//

	// button with popup, goddamn x4
	QToolButton *btn_3D = new QToolButton(tool_bar);
	btn_3D->setText("3D");
	btn_3D->setToolTip("3D figures");
	btn_3D->setMaximumWidth(4 * CELL);

	QMenu *menu_3D = new QMenu(btn_3D);
	menu_3D->addAction("Cube")->setData(static_cast<int>(GType::Cube));
	menu_3D->addAction("Tetrahedron")
		->setData(static_cast<int>(GType::Tetrahedron));

	btn_3D->setMenu(menu_3D);
	btn_3D->setPopupMode(QToolButton::InstantPopup);
	//

	// button with popup, goddamn x5
	QToolButton *btn_polygon = new QToolButton(tool_bar);
	btn_polygon->setText("Polygon");
	btn_polygon->setToolTip("Various polygon types");
	btn_polygon->setMaximumWidth(4 * CELL);

	QMenu *menu_polygon = new QMenu(btn_polygon);
	menu_polygon->addAction("Polygon")->setData(
		static_cast<int>(GType::Polygon));
	menu_polygon->addAction("Convex polygon")
		->setData(static_cast<int>(GType::ConvexPolygon));

	btn_polygon->setMenu(menu_polygon);
	btn_polygon->setPopupMode(QToolButton::InstantPopup);
	//

	// button with popup, goddamn x6
	QToolButton *btn_fill = new QToolButton(tool_bar);
	btn_fill->setText("Fill");
	btn_fill->setToolTip("Polygon filling algorithms");
	btn_fill->setMaximumWidth(4 * CELL);

	QMenu *menu_fill = new QMenu(btn_fill);

	menu_fill->addAction("Ordered Edges")
		->setData(static_cast<int>(GType::FillOrderedEdges));
	menu_fill->addAction("Active Edges")
		->setData(static_cast<int>(GType::FillActiveEdges));
	menu_fill->addAction("Seed Simple")
		->setData(static_cast<int>(GType::FillSeedSimple));
	menu_fill->addAction("Seed Scanline")
		->setData(static_cast<int>(GType::FillSeedScanline));

	btn_fill->setMenu(menu_fill);
	btn_fill->setPopupMode(QToolButton::InstantPopup);
	//

	// button with popup, goddamn x7
	QToolButton *btn_polygonalisation = new QToolButton(tool_bar);
	btn_polygonalisation->setText("PolyFill");
	btn_polygonalisation->setToolTip("Delone and Voronoi algorithms");
	btn_polygonalisation->setMaximumWidth(4 * CELL);

	QMenu *menu_polygonalisation = new QMenu(btn_polygonalisation);

	menu_polygonalisation->addAction("Delone")->setData(
		static_cast<int>(GType::Delone));
	menu_polygonalisation->addAction("Voronoi")->setData(
		static_cast<int>(GType::Voronoi));

	btn_polygonalisation->setMenu(menu_polygonalisation);
	btn_polygonalisation->setPopupMode(QToolButton::InstantPopup);
	//

	// algs buttons
	tool_bar->addWidget(dot_btn);
	this->connect(dot_btn, &QToolButton::clicked, this, [this]() {
		this->data_handler->set_figure({ GType::Dot });
	});

	tool_bar->addWidget(frl_btn);
	this->connect(frl_menu, &QMenu::triggered, this, [this](QAction *act) {
		int id = act->data().toInt();
		this->data_handler->set_figure({ static_cast<GType>(id) });
	});

	tool_bar->addWidget(srl_btn);
	this->connect(srl_menu, &QMenu::triggered, this, [this](QAction *act) {
		int id = act->data().toInt();
		this->data_handler->set_figure({ static_cast<GType>(id) });
	});

	tool_bar->addWidget(arl_btn);
	this->connect(arl_menu, &QMenu::triggered, this, [this](QAction *act) {
		int id = act->data().toInt();
		this->data_handler->set_figure({ static_cast<GType>(id) });
	});

	tool_bar->addWidget(btn_3D);
	this->connect(menu_3D, &QMenu::triggered, this, [this](QAction *act) {
		int id = act->data().toInt();
		this->data_handler->set_figure({ static_cast<GType>(id) });
	});

	tool_bar->addWidget(btn_polygon);
	this->connect(menu_polygon, &QMenu::triggered, this,
		      [this](QAction *act) {
			      int id = act->data().toInt();
			      this->data_handler->set_figure(
				      { static_cast<GType>(id) });
		      });

	tool_bar->addWidget(btn_fill);
	this->connect(menu_fill, &QMenu::triggered, this, [this](QAction *act) {
		int id = act->data().toInt();
		this->data_handler->set_figure({ static_cast<GType>(id) });
	});

	tool_bar->addWidget(btn_polygonalisation);
	this->connect(menu_polygonalisation, &QMenu::triggered, this,
		      [this](QAction *act) {
			      int id = act->data().toInt();
			      this->data_handler->set_figure(
				      { static_cast<GType>(id) });
		      });

	// misc buttons
	tool_bar->addWidget(clear_canvas_btn);
	this->connect(clear_canvas_btn, &QToolButton::clicked, canvas,
		      [this, canvas]() {
			      canvas->on_clear();
			      bool d = this->debugger->get_debug();
			      this->debugger->reset();
			      this->data_handler->reset();
			      this->debugger->set_debug(d);
		      });

	tool_bar->addWidget(debug_checkbox);
	this->connect(debug_checkbox, &QCheckBox::toggled, this,
		      [this](bool checked) {
			      if (checked) {
				      qDebug() << "Debug ON";
				      this->debugger->reset();
				      this->debugger->set_debug(true);
			      } else {
				      qDebug() << "Debug OFF";
				      this->debugger->set_debug(false);
				      this->debugger->begin_debug();
			      }
		      });

	tool_bar->addWidget(debug_step_btn);
	this->connect(debug_checkbox, &QCheckBox::toggled, debug_step_btn,
		      &QToolButton::setEnabled);
	this->connect(debug_step_btn, &QToolButton::clicked, this,
		      [this]() { this->debugger->begin_debug(); });

	tool_bar->addWidget(debug_stop_btn);
	this->connect(debug_checkbox, &QCheckBox::toggled, debug_stop_btn,
		      &QToolButton::setEnabled);
	this->connect(debug_stop_btn, &QToolButton::clicked, this, [this]() {
		bool d = this->debugger->get_debug();
		this->debugger->set_debug(false);
		this->debugger->begin_debug();
		this->debugger->set_debug(d);
	});

	// setup shortcuts

	// increase cell size
	new QShortcut(QKeySequence("+"), this, [canvas]() {
		int sz = canvas->get_px_size();
		sz++;
		canvas->set_px_size(sz);
	});

	// decrease cell size
	new QShortcut(QKeySequence("-"), this, [canvas]() {
		int sz = canvas->get_px_size();
		sz--;
		canvas->set_px_size(sz);
	});

	new QShortcut(QKeySequence("x"), this,
		      [this]() { this->data_handler->rotate_last_3D('x'); });
	new QShortcut(QKeySequence("y"), this,
		      [this]() { this->data_handler->rotate_last_3D('y'); });
	new QShortcut(QKeySequence("z"), this,
		      [this]() { this->data_handler->rotate_last_3D('z'); });
	new QShortcut(QKeySequence("u"), this,
		      [this]() { this->data_handler->scale_last_3D('u'); });
	new QShortcut(QKeySequence("d"), this,
		      [this]() { this->data_handler->scale_last_3D('d'); });
	new QShortcut(QKeySequence("p"), this,
		      [this]() { this->data_handler->perspective_last_3D(); });
	new QShortcut(QKeySequence(Qt::Key_Right), this,
		      [this]() { this->data_handler->move_last_3D('r'); });
	new QShortcut(QKeySequence(Qt::Key_Left), this,
		      [this]() { this->data_handler->move_last_3D('l'); });
	new QShortcut(QKeySequence(Qt::Key_Up), this,
		      [this]() { this->data_handler->move_last_3D('d'); });
	new QShortcut(QKeySequence(Qt::Key_Down), this,
		      [this]() { this->data_handler->move_last_3D('u'); });
	new QShortcut(QKeySequence(Qt::Key_F10), this, [this]() {
		bool d = this->debugger->get_debug();
		if (d) {
			this->debugger->begin_debug();
		}
	});
	new QShortcut(QKeySequence(Qt::Key_A), this, [this]() {
		this->data_handler->set_control_points(
			this->data_handler->get_figure().points.size());
		this->data_handler->add_point();
		this->data_handler->set_control_points(CON_POINTS);
	});
}

MainWindow::~MainWindow()
{
	delete this->debugger;
	delete this->data_handler;
	std::cout << "Window out...\n";
}

void MainWindow::on_info()
{
	QMessageBox::information(
		this, "About",
		"Awesome Editor created by Owerk and Glentas (and "
		"Qwen)\n"
		"C++ and Qt6.10.2\n"
		"Version 2.3\n"
		"2026");
}

void MainWindow::on_help()
{
	QMessageBox::information(
		this, "Help",
		"1. To draw a line - set a start point and an end point\n"
		"2. To draw a circle - set a center point and a radius (relative to "
		"center point)\n"
		"3. To draw a elipsis - set a center point, a (x axis, relative to "
		"center "
		"point) and b (y axis, relative to center point)\n"
		"4. To draw a Hermit curve - set a start point, a start force, an end "
		"point and an end force\n"
		"5. To draw a Bezier curve - set a start point, the first magnet, the "
		"second magnet and an end point\n"
		"6. To draw a bspline - set 8 points\n"
		"7. To draw a cube - set the first vertex point and edge length\n"
		"8. To draw a tetrahedron - set the first vertex point and edge length\n"
		"9. To fill polygon (Ordered Edges) - set 3+ polygon vertices, press 'a' "
		"to complete\n"
		"10. To fill polygon (Active Edges) - set 3+ polygon vertices, press 'a' "
		"to complete\n"
		"11. To fill polygon (Seed Simple) - set 3+ polygon vertices + seed "
		"point, press 'a' to complete\n"
		"12. To fill polygon (Seed Scanline) - set 3+ polygon vertices + seed "
		"point, press 'a' to complete\n"
		"\n"
		"Useful shortcuts:\n"
		"'F10' - debug step\n"
		"'a' - completes polygon/fill with current control points\n"
		"'+' - increase grid size\n"
		"'-' - decrease grid size\n"
		"'x' - rotate 3D object in x axis\n"
		"'y' - rotate 3D object in y axis\n"
		"'z' - rotate 3D object in z axis\n"
		"'u' - scale 3D object up\n"
		"'d' - scale 3D object down\n"
		"'p' - perspective view\n"
		"arrows - move 3D object\n");
}

// connecting user clicks, in order to calculate when to launch algorithms
void MainWindow::on_clicked_px(Point px)
{
	this->data_handler->add_point(px);
}

void MainWindow::on_size_update()
{
	bool ok;
	QString *max_size = new QString();
	max_size->assign("Size (1 - " + std::to_string(2 * CELL) + "):");

	int n = QInputDialog::getInt(this, "Size of a grid cell", *max_size, 1,
				     1, 2 * CELL, 1, &ok);
	if (ok) {
		this->debugger->get_canvas()->set_px_size(n);
	}
}

void MainWindow::on_show_figures_dialog()
{
	QDialog *dialog = new QDialog(this);
	dialog->setWindowTitle("Список фигур");
	dialog->resize(300, 400);

	QVBoxLayout *layout = new QVBoxLayout(dialog);

	QListWidget *listWidget = new QListWidget();
	listWidget->setSelectionMode(QAbstractItemView::SingleSelection);

	for (const auto &fig : this->data_handler->get_figures()) {
		QString itemText = QString("%1 %2")
					   .arg(gtype_to_string(fig.fig_type))
					   .arg(fig.id);
		listWidget->addItem(itemText);
	}

	layout->addWidget(listWidget);

	QPushButton *okButton = new QPushButton("OK");
	connect(okButton, &QPushButton::clicked, dialog, &QDialog::accept);
	layout->addWidget(okButton);

	dialog->setLayout(layout);

	if (dialog->exec() == QDialog::Accepted) {
		int row = listWidget->currentRow();
		if (row >= 0 &&
		    row < this->data_handler->get_figures().size()) {
			this->data_handler->set_current_active_idx(row);
			this->data_handler->set_prev_active_idx(row);
		}
	}

	delete dialog;
}