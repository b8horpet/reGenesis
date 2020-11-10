#include "Debug.h"
#include "SharedData.h"
#include <cstring>

#include <cstdarg>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include "Windows.h"
#endif

void DEBUG_LOG_IMPL(const char* fmt, va_list args)
{
	char buff[4096];
	memset(buff, 0, sizeof(buff));
	vsnprintf(buff, sizeof(buff), fmt, args);
#if defined _WIN32
	OutputDebugStringA(buff);
#else
	puts(buff);
#endif
}

void DEBUG_LOG(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	DEBUG_LOG_IMPL(fmt, args);
	va_end(args);
}

timer_helper::timer_helper(const char* name)
	: start(std::chrono::high_resolution_clock::now())
	, name(name)
{}

timer_helper::~timer_helper()
{
	auto stop = std::chrono::high_resolution_clock::now();
	DEBUG_LOG("%s %f ms",name.c_str(), std::chrono::duration<double, std::milli>(stop - start).count());
}

// no threading yet
RenderData g_debug_data;

void DebugDrawer::FillRenderData(RenderData& renderData)
{
	renderData.insert(renderData.end(), g_debug_data.begin(), g_debug_data.end());
}
void DebugDrawer::DrawPoint(const Vec3d& p)
{
	g_debug_data.emplace_back(std::shared_ptr<ObjectData>(new PointData(p)));
}
void DebugDrawer::DrawLine(const Vec3d& p1, const Vec3d& p2)
{
	g_debug_data.emplace_back(std::shared_ptr<ObjectData>(new LineData(p1, p2)));
}
void DebugDrawer::DrawCircle(const Vec3d& p, double r)
{
	g_debug_data.emplace_back(std::shared_ptr<ObjectData>(new SphereData(p, r)));
}
void DebugDrawer::DrawBox(const Vec3d& tl, const Vec3d& br)
{
	g_debug_data.emplace_back(std::shared_ptr<ObjectData>(new BoxData(tl, br)));
}
void DebugDrawer::Clear()
{
	g_debug_data.clear();
}
