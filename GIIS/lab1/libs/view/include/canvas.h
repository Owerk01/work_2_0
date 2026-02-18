#pragma once
#include "vars.h"
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

public slots:
  void on_size_update();
  void on_clear();
  void on_grid_show();

signals:

protected:
  void paintEvent(QPaintEvent *) override;

private:
  bool show_grid;
  int px_size;
  std::vector<Point> pixels;
};