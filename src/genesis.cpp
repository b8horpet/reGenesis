#include <stdio.h>
#include "Physics/World.h"
#include "Creature/Creature.h"
#include "Graphics/SurfaceCommon/ISurface.h"
//hack:
//#include "Graphics/Surfaces/Surface_SDL.h"
#include "Graphics/Renderers/IRenderer.h"

#include <cmath>

extern bool g_Paused;

int main(int argc, char** argv)
{
	ZoneScopedN("main");
	World theWorld(argc > 1 ? argv[1] : "");
	{
	ZoneScopedN("init")
	for(int i=0; i<5; ++i)
		for(int j=0; j<=i; ++j)
			theWorld.AddObject(std::make_shared<Sphere>(Vec3d{double(j*2)-double(i),-double(i*2)*sqrt(3.0)/2.0,0}));
	}
	//auto c=std::make_shared<Sphere>(Vec3d{10,10,0});
	auto c = std::make_shared<Creature>(Vec3d{ 10,10,0 });
	c->mVelocity=Vec3d{-.6,-.72,0};
	theWorld.AddObject(c);
	auto theSurface=CreateSurface("SDL2");
	printf("reGenesis\n");
	// g_Paused = true;
	for(int i=0; i<10000;)
	{
		ZoneScopedN("loop");
		// MEASURE();
		theWorld.Activate();
		if(!g_Paused) i++;
		if(theSurface)
		{
			ZoneScopedN("render");
			IRenderer* Renderer=theSurface->GetRenderer();
			if(Renderer)
			{
				Renderer->UpdateData(theWorld.GetRenderData());
			}
			theSurface->Present();
			FrameMark;
		}
	}
	return 0;
}
