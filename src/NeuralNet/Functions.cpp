#include "Functions.h"

std::shared_ptr<IFunctionObject> ConstValueFunc::gConstZero
	= std::make_shared<ConstValueFunc>(0.0);
std::shared_ptr<IFunctionObject> TangentHyperbolicFunc::gOneMinusTanhSquaredFunc
	= std::make_shared<TangentHyperbolicFunc::OneMinusTanhSquaredFunc>();
