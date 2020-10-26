#pragma once
#ifndef PHYSICS_DEBUG_H_INCLUDED
#define PHYSICS_DEBUG_H_INCLUDED

#include <chrono>
#include <string>
#include "../tracy/Tracy.hpp"

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

#endif
