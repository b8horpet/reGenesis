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
		Sphere,
		Cube,
		PolyHedron,
	} Shape;
	Vec3 Color;
	Metadata(ObjectShape s=None) : Shape(s) {}
};

struct ObjectData
{
	Metadata Meta;
	ObjectData(Metadata::ObjectShape s=Metadata::None) : Meta(s) {}
	virtual ~ObjectData() {}
};

struct SphereData : ObjectData
{
	SphereData() : ObjectData(Metadata::Sphere) {}
	Vec3 Center;
	double Radius;
};

// axis aligned
struct CubeData : ObjectData
{
	CubeData() : ObjectData(Metadata::Cube) {}
	Vec3 Center;
	double Size;
};

struct PolyHedronData : ObjectData
{
	// wireframe for now
	typedef std::pair<size_t,size_t> IndexPair;
	PolyHedronData() : ObjectData(Metadata::PolyHedron) {}
	std::vector<Vec3> Vertices;
	std::vector<IndexPair> Edges;
};

#endif
