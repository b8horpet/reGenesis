
#pragma once
#ifndef GRAPHICS_IRENDERER_H_INCLUDED
#define GRAPHICS_IRENDERER_H_INCLUDED

#include <vector>
#include <memory>
#include "../../Physics/Basics.h" // NOOOOOOOOOOOOOOOOOO!!!
#include "../../Physics/SharedData.h"

class IRenderer
{
public:
	virtual ~IRenderer() {}
	virtual void Render() = 0;
	virtual void UpdateData(std::vector<std::shared_ptr<ObjectData>>) = 0;
protected:
};

#endif
