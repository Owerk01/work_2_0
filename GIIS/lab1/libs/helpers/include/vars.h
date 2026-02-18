#pragma once

#include <QCheckBox>
#include <QToolButton>

#define CODE_ERROR -1
#define CELL 16
#define INIT_WIDTH 48 * CELL
#define INIT_HEIGHT 48 * CELL
#define OFFSET int(CELL / 2)
#define CANVAS_SIZE 48 * CELL

#define MIN_WINDOW_WIDTH int((OFFSET + CANVAS_SIZE) / 4)
#define MIN_WINDOW_HEIGHT MIN_WINDOW_WIDTH

#define MAX_WINDOW_WIDTH                                                       \
  CANVAS_SIZE <= MIN_WINDOW_WIDTH ? MIN_WINDOW_WIDTH : CANVAS_SIZE

#define MAX_WINDOW_HEIGHT                                                      \
  CANVAS_SIZE <= MIN_WINDOW_HEIGHT ? MIN_WINDOW_HEIGHT : CANVAS_SIZE

struct Point {
  int x = 0;
  int y = 0;
  int color = 0;
};

class ToolBarObject {
public:
  virtual ~ToolBarObject() = default;
  virtual void init() = 0;
};

class TBOButton : public ToolBarObject {
public:
  ~TBOButton() override;
  virtual void init() override;

private:
  QToolButton *btn;
};

class TBOCheckBox : public ToolBarObject {
public:
  ~TBOCheckBox() override;
  virtual void init() override;

private:
  QCheckBox *chk_box;
};
