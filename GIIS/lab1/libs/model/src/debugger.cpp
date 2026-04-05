#include "debugger.h"
#include "canvas.h"
#include "vars.h"
#include <iostream>

Debugger::Debugger(Canvas *canvas, bool debug_m, int step)
	: canvas(canvas)
	, debug_mode(debug_m)
	, step_i(step)
{
}

Debugger::~Debugger()
{
	std::cout << "Debugger out...\n";
}

void Debugger::set_debug(bool debug)
{
	this->debug_mode = debug;
}

void Debugger::set_canvas(Canvas *cnvs)
{
	this->canvas = cnvs;
}

void Debugger::set_figure(Figure fig)
{
	this->fig = fig;
}

void Debugger::reset()
{
	this->step_i = 0;
	this->canvas->set_locked(false);
	this->debug_mode = false;
	this->fig.points.clear();
}

Canvas *Debugger::get_canvas()
{
	return this->canvas;
}
bool Debugger::step()
{
	if (this->fig.points.size() >= 1) {
		this->canvas->set_pixel(this->fig.points[step_i]);
		this->step_i++;
		if (this->step_i >= this->fig.points.size() - 1) {
			return false;
		} else {
			return true;
		}
	} else {
		return false;
	}
}

void Debugger::begin_debug()
{
	if (!this->fig.points.empty()) {
		this->canvas->set_locked(true);
		if (this->debug_mode == false) {
			while (this->step()) {
			}
			this->reset();
			this->debug_mode = false;
			this->step_i = 0;
		} else {
			if (this->step()) {
			} else {
				this->reset();
				this->debug_mode = true;
			}
		}
	}
}

bool Debugger::get_debug() const
{
	return this->debug_mode;
}