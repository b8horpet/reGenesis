#pragma once
#ifndef PHYSICS_SHAREDDATA_H_INCLUDED
#define PHYSICS_SHAREDDATA_H_INCLUDED

#include "Basics.h"
#include <vector>


struct Metadata
{
	enum ObjectShape
	{
		None, // undefined type
		Point,
		Line,
		Sphere,
		Box,
		PolyHedron,
	} Shape;
	Clr4d Color;
	Metadata(ObjectShape s=None) : Shape(s) {}
};

struct ObjectData
{
	Metadata Meta;
	ObjectData(Metadata::ObjectShape s=Metadata::None) : Meta(s) {}
	virtual ~ObjectData() {}
};

struct PointData : ObjectData
{
	PointData() : ObjectData(Metadata::Point) {}
	Vec3d Coord;
};

struct LineData : ObjectData
{
	LineData() : ObjectData(Metadata::Line) {}
	Vec3d Start;
	Vec3d End;
};

struct SphereData : ObjectData
{
	SphereData() : ObjectData(Metadata::Sphere) {}
	Vec3d Center;
	double Radius;
};

// axis aligned
struct BoxData : ObjectData
{
	BoxData() : ObjectData(Metadata::Box) {}
	Vec3d TopLeft;
	Vec3d BottomRight;
};

struct PolyHedronData : ObjectData
{
	typedef std::pair<size_t,size_t> IndexPair;
	PolyHedronData() : ObjectData(Metadata::PolyHedron) {}
	std::vector<Vec3d> Vertices;
	std::vector<IndexPair> Edges;
};

#endif
