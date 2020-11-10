#pragma once
#ifndef PHYSICS_SHAREDDATA_H_INCLUDED
#define PHYSICS_SHAREDDATA_H_INCLUDED

#include "Basics.h"
#include <vector>
#include <memory>


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
	Metadata(ObjectShape s=None)
		: Shape(s), Color{1., 1., 1., 1.} {}
};

struct ObjectData
{
	Metadata Meta;
	ObjectData(Metadata::ObjectShape s=Metadata::None) : Meta(s) {}
	virtual ~ObjectData() {}
};

struct PointData : ObjectData
{
	PointData(const Vec3d& p) 
		: ObjectData(Metadata::Point)
		, Coord{p}
	{}
	Vec3d Coord;
};

struct LineData : ObjectData
{
	LineData(const Vec3d& p1, const Vec3d& p2)
		: ObjectData(Metadata::Line)
		, Start{p1}
		, End{p2}
	{}
	Vec3d Start;
	Vec3d End;
};

struct SphereData : ObjectData
{
	SphereData(const Vec3d& p, double r)
		: ObjectData(Metadata::Sphere)
		, Center{p}
		, Radius{r}
	{}
	Vec3d Center;
	double Radius;
};

// axis aligned
struct BoxData : ObjectData
{
	BoxData(const Vec3d& p1, const Vec3d& p2)
		: ObjectData(Metadata::Box)
		, TopLeft{CompwiseMin(p1,p2)}
		, BottomRight{CompwiseMax(p1,p2)}
	{}
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

typedef std::vector<std::shared_ptr<ObjectData>> RenderData;

#endif
