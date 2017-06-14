#pragma once
#ifndef NEURALNET_COMMON_H_INCLUDED
#define NEURALNET_COMMON_H_INCLUDED

#include "Functions.h"

class INeuralObject : public std::enable_shared_from_this<INeuralObject>
{
	static double Braveness;
public:
	virtual ~INeuralObject(){}
	virtual void Activate() = 0;
	virtual void Propagate() = 0;
};

#endif
