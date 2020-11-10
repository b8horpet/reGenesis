#pragma once
#ifndef PHYSICS_BASICS_H_INCLUDED
#define PHYSICS_BASICS_H_INCLUDED

#include "b8.hpp"
using namespace b8;

/*
template<typename T>
T clamp(T in, T low, T high)
{
	return std::min(std::max(in, low), high);
}

template<int D, typename DataType=double>
Vec<D, DataType> clamp(const Vec<D, DataType>& v, DataType low, DataType high)
{
	Vec<D, DataType> ret;
	for(int i=0; i<D; ++i)
	{
		ret[i]=clamp(v[i], low, high);
	}
	return ret;
}
*/

#endif