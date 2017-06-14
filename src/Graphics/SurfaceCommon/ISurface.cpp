#include "ISurface.h"

#include "../Surfaces/Surface_SDL.h"

std::unique_ptr<ISurface> CreateSurface(std::string surface_name)
{
	std::unique_ptr<ISurface> ret=nullptr;
	try
	{
		if(surface_name == "SDL2")
			ret = std::unique_ptr<Surface_SDL>(new Surface_SDL());
	}
	catch(...) {}
	return ret;
}

// header only?

/*#author: b8horpet


class SurfaceInterface:
    def __init__(self,u):
        pass

    def StartRender(self):
        pass
*/

