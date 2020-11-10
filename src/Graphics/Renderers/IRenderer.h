
#pragma once
#ifndef GRAPHICS_IRENDERER_H_INCLUDED
#define GRAPHICS_IRENDERER_H_INCLUDED

#include <vector>
#include <memory>
#include "../../Physics/Debug.h"
#include "../../Physics/Basics.h" // NOOOOOOOOOOOOOOOOOO!!!
#include "../../Physics/SharedData.h"

class IRenderer
{
public:
	virtual ~IRenderer() {}
	virtual void Render() = 0;
	virtual void UpdateData(const RenderData&) = 0;
	virtual void MoveOffset(const Vec2d&) {}
	virtual void ZoomScale(double) {}
	virtual void ResetView() {}
};

#endif
