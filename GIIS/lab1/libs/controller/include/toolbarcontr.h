#pragma once

#include "vars.h"
#include <QObject>
#include <QToolBar>
#include <QToolButton>
#include <QWidget>
#include <vector>

class AdvToolBar : public QToolBar {
  Q_OBJECT

public:
  explicit AdvToolBar(QWidget *parent = nullptr);
public slots:
signals:
private:
  std::vector<ToolBarObject> buttons;
};