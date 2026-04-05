#include "canvas.h"
#include "vars.h"
#include <QInputDialog>
#include <QMouseEvent>
#include <QObject>
#include <QPaintEvent>
#include <QPainter>
#include <QPen>
#include <QString>
#include <QWidget>
#include <iostream>
#include <vector>

Canvas::Canvas(QWidget *parent, bool is_grid, int pixel_size)
	: QWidget(parent)
	, show_grid(is_grid)
	, px_size(pixel_size)
	, pixels({})
	, locked(false)
{
	this->set_px_size(pixel_size);
}

Canvas::~Canvas()
{
	std::cout << "Canvas out...\n";
}

int Canvas::get_px_size() const
{
	return this->px_size;
}

void Canvas::set_locked(bool lock)
{
	this->locked = lock;
}

void Canvas::set_px_size(int size)
{
	if (size >= 1 && size <= 2 * CELL) {
		this->px_size = size;
		this->setFixedSize(CANVAS_SIZE * this->px_size,
				   CANVAS_SIZE * this->px_size);
		this->update();
	}
}

void Canvas::set_show_grid(bool grid)
{
	this->show_grid = grid;
	this->update();
}

void Canvas::set_pixel(Point px)
{
	if (px.x >= 0 && px.x <= CANVAS_SIZE - 1 && px.y >= 0 &&
	    px.y <= CANVAS_SIZE - 1) {
		this->pixels.push_back(px);
		this->update();
	}
}

void Canvas::paintEvent(QPaintEvent *)
{
	QPainter p(this);
	p.fillRect(this->rect(), Qt::white);

	QPen pen(Qt::lightGray, 1, Qt::SolidLine);
	QPen pen2(Qt::black, 1, Qt::SolidLine);
	p.setPen(pen);

	if (this->px_size > 1 && this->show_grid == true) {
		int step = this->px_size;

		for (int x = 0; x <= this->width(); x += step) {
			if (x % (10 * step) == 0) {
				p.setPen(pen2);
				p.drawLine(x, 0, x, this->height());
				p.setPen(pen);
			} else {
				p.drawLine(x, 0, x, this->height());
			}
		}

		for (int y = 0; y <= this->height(); y += step) {
			if (y % (10 * step) == 0) {
				p.setPen(pen2);
				p.drawLine(0, y, this->width(), y);
				p.setPen(pen);
			} else {
				p.drawLine(0, y, this->width(), y);
			}
		}
	}

	int side = this->px_size > 1 ? this->px_size - 1 : 1;
	for (auto px : this->pixels) {
		int x = this->px_size > 1 ? px.x * this->px_size + 1 : px.x;
		int y = this->px_size > 1 ? px.y * this->px_size + 1 : px.y;
		QColor col(px.r, px.g, px.b);
		p.fillRect(x, y, side, side, col);
	}
}

void Canvas::on_clear()
{
	this->pixels.clear();
	this->update();
}

void Canvas::on_grid_show()
{
	if (this->show_grid == true) {
		this->set_show_grid(false);
	} else {
		this->set_show_grid(true);
	}
}

void Canvas::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton && this->locked == false) {
		int widgetX = event->pos().x();
		int widgetY = event->pos().y();

		int gridX = widgetX / this->px_size;
		int gridY = widgetY / this->px_size;

		if (gridX >= 0 && gridX < CANVAS_SIZE && gridY >= 0 &&
		    gridY < CANVAS_SIZE) {
			Point px(gridX, gridY, 255, 64, 64);

			this->set_pixel(px);

			emit this->clicked_px(px);
		}
	}
}