#pragma once
#ifndef GRAPHICS_ISURFACE_H_INCLUDED
#define GRAPHICS_ISURFACE_H_INCLUDED
#include <memory>
#include <string>
#include "../Renderers/IRenderer.h"

class ISurface
{
public:
	struct Properties
	{
		int Pos[2];
		int Size[2];
	};
	ISurface() = default;
	virtual ~ISurface() = default;
	virtual void Present() = 0;
	virtual Properties GetProperties() = 0;
	virtual bool SetProperties(Properties p) = 0;
	virtual IRenderer* GetRenderer() = 0;
};

std::unique_ptr<ISurface> CreateSurface(std::string surface_name);

/*class SurfaceInterface:
    def __init__(self,u):
        pass

    def StartRender(self):
        pass
*/

#endif
