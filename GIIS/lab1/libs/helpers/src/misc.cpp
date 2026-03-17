#include "misc.h"
#include "vars.h"
#include <string>

std::string gtype_to_string(GType type) {
  switch (type) {
  case GType::CDA:
    return "CDA";
  case GType::Bresenham:
    return "Bresenham";
  case GType::Wu:
    return "Wu";
  case GType::Circle:
    return "Circle";
  case GType::Elipsis:
    return "Elipsis";
  case GType::Parabola:
    return "Parabola";
  case GType::Hyperbola:
    return "Hyperbola";
  case GType::Hermit:
    return "Hermit";
  case GType::Bezier:
    return "Bezier";
  case GType::BSpline:
    return "BSpline";
  case GType::Cube:
    return "Cube";
  case GType::Tetrahedron:
    return "Tetrahedron";
  case GType::Polygon:
    return "Polygon";
  case GType::ConvexPolygon:
    return "ConvexPolygon";
  case GType::Dot:
    return "Dot";
  default:
    return "Unknown";
  }
}