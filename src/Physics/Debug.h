#pragma once
#include <memory>
#ifndef PHYSICS_DEBUG_H_INCLUDED
#define PHYSICS_DEBUG_H_INCLUDED

#include <chrono>
#include <string>
#include "Tracy.hpp"
#include "SharedData.h"

void DEBUG_LOG(const char* fmt, ...);

struct timer_helper
{
	timer_helper(const char*);
	~timer_helper();
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
	std::string name;
};
#define MEASURE() timer_helper t##__LINE__(__FUNCTION__)

struct DebugDrawer : public Singleton<DebugDrawer>
{
	void FillRenderData(RenderData& renderData);
	void SetColor(const Clr4d& c);
	void DrawPoint(const Vec3d& p);
	void DrawLine(const Vec3d& p1, const Vec3d& p2);
	void DrawCircle(const Vec3d& p, double r);
	void DrawBox(const Vec3d& tl, const Vec3d& br);
	void Clear();
};

#endif
