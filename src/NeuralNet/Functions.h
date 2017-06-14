#pragma once
#ifndef NEURALNET_FUNCTIONS_H_INCLUDED
#define NEURALNET_FUNCTIONS_H_INCLUDED

#include <cmath>
#include <memory>

class IFunctionObject : public std::enable_shared_from_this<IFunctionObject>
{
public:
    virtual double operator()(double) = 0;

	virtual std::shared_ptr<IFunctionObject> Differentiate()
	{
		return std::shared_ptr<IFunctionObject>(nullptr);
	}
};

class ConstValueFunc : public IFunctionObject
{
	friend class ConstValueRefFunc;
	static std::shared_ptr<IFunctionObject> gConstZero;
	const double mValue;
public:
	ConstValueFunc(double val) : mValue(val) {}
	virtual double operator()(double) override
	{
		return mValue;
	}
	virtual std::shared_ptr<IFunctionObject> Differentiate() override
	{
		return gConstZero;
	}
};

class ConstValueRefFunc : public IFunctionObject
{
	const std::shared_ptr<double> mValue;
public:
	ConstValueRefFunc(std::shared_ptr<double> ref) : mValue(ref) {}
	virtual double operator()(double) override
	{
		if(mValue)
			return *mValue;
		return 0.0;
	}
	virtual std::shared_ptr<IFunctionObject> Differentiate() override
	{
		return ConstValueFunc::gConstZero;
	}
};

class LinearFunc :  public IFunctionObject
{
	double mCoeff;
public:
	LinearFunc(double coeff=1.0) : mCoeff(coeff) {}
	virtual double operator()(double x) override
	{
		return mCoeff*x;
	}

	virtual std::shared_ptr<IFunctionObject> Differentiate() override
	{
		return std::make_shared<ConstValueFunc>(mCoeff);
	}
};

class SlabFunc : public IFunctionObject
{
public:
	virtual double operator()(double x) override
	{
		if(x>0.0)
			return 1.0;
		return -1.0;
	}
};

class LinearSlabFunc : public IFunctionObject
{
	const double mCoeff;
public:
	LinearSlabFunc(double coeff=1.0) : mCoeff(coeff) {}
	virtual double operator()(double x) override
	{
		const double v=x*mCoeff;
		if(v >= 1.0)
			return 1.0;
		if(v <= -1.0)
			return -1.0;
		return v;
	}
};


class TangentHyperbolicFunc : public IFunctionObject
{
	class OneMinusTanhSquaredFunc : public IFunctionObject
	{
	public: 
		double operator()(double x) override
		{
			const double t=tanh(x);
			return 1.0-t*t;
		}
	};
	static std::shared_ptr<IFunctionObject> gOneMinusTanhSquaredFunc;
public:
	double operator()(double x) override
	{
		return tanh(x);
	}
	virtual std::shared_ptr<IFunctionObject> Differentiate() override
	{
		return gOneMinusTanhSquaredFunc;
	}
};

// todo
/*
class Polinomial(FunctionObjectInterface):
    def __init__(self,n: int, a: float = 1.0):
        self.n = n
        self.a = a

    def __call__(self, x: float, *args, **kwargs):
        return self.a * ( x**self.n )

    def __str__(self):
        return "f(x)="+str(self.a)+"*x^"+str(self.n)

    def __hash__(self):
        return hash(self.__str__())

    def __eq__(self, other):
        #for now
        return isinstance(other,type(self)) and (self.n,self.a) == (other.n,other.a)

    def Differentiate(self):
        return Polinomial(self.n-1,self.a*self.n)
*/

#endif
