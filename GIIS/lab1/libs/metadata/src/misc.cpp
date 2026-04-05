#include "misc.h"
#include "vars.h"
#include <cmath>
#include <string>

std::string gtype_to_string(const GType &type)
{
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
	case GType::FillOrderedEdges:
		return "FillOrderedEdges";
	case GType::FillActiveEdges:
		return "FillActiveEdges";
	case GType::FillSeedSimple:
		return "FillSeedSimple";
	case GType::FillSeedScanline:
		return "FillSeedScanline";
	case GType::Delone:
		return "Delone";
	case GType::Voronoi:
		return "Voronoi";
	case GType::Dot:
		return "Dot";
	default:
		return "Unknown";
	}
}

bool f_eq(const double &a, const double &b)
{
	return std::abs(a - b) <= E_FACTOR;
}