#include "Debug.h"
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
	: name(name)
	, start(std::chrono::high_resolution_clock::now())
{}

timer_helper::~timer_helper()
{
	auto stop = std::chrono::high_resolution_clock::now();
	DEBUG_LOG("%s %f ms\n",name.c_str(), std::chrono::duration<double, std::milli>(stop - start).count());
}
