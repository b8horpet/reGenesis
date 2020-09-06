#pragma once
#ifndef PHYSICS_IGEOMETRY_H_INCLUDED
#define PHYSICS_IGEOMETRY_H_INCLUDED

#include "Object.h"

class IGeometry
{
public:
	virtual ~IGeometry(){}
	virtual void DoCollisions(std::vector<std::shared_ptr<Object>> os) = 0;
};

#endif
