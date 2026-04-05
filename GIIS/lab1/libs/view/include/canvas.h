#pragma once
#include "vars.h"
#include <QMouseEvent>
#include <QObject>
#include <QPaintEvent>
#include <QWidget>
#include <vector>

class Canvas : public QWidget {
	Q_OBJECT
    public:
	explicit Canvas(QWidget *parent = nullptr, bool is_grid = true,
			int pixel_size = CELL);
	~Canvas();

	void set_px_size(int size);
	void set_show_grid(bool grid);
	void set_pixel(Point px);
	void set_locked(bool lock);

	int get_px_size() const;

    public slots:
	void on_clear();
	void on_grid_show();

    signals:
	void clicked_px(Point pt);

    protected:
	void paintEvent(QPaintEvent *) override;
	void mousePressEvent(QMouseEvent *event) override;

    private:
	bool show_grid;
	bool locked;
	int px_size;
	std::vector<Point> pixels;
};