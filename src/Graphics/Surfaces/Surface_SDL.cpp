#include "../SurfaceCommon/ISurface.h"
#include "Surface_SDL.h" // TODO FIX INCLUDES ASAP!!!
#include "../Renderers/OpenGL.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <cstdio>
#include <cstdlib>

static SDL_Window* window = nullptr;
static SDL_GLContext context = nullptr;


Surface_SDL::Surface_SDL()
: Renderer(nullptr)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr,"SDL could not initialize, error='%s'\n",SDL_GetError());
		throw 1;
	}
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	//SDL_Renderer* renderer = nullptr;
	window = SDL_CreateWindow("reGenesis",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			400,
			400,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
//	if(!SDL_CreateWindowAndRenderer(400,400,0,&window,&renderer))
//	{
//		fprintf(stderr,"failed to create window, or renderer %s\n", SDL_GetError());
//		return 1;
//	}
	if(!window)
	{
		fprintf(stderr, "SDL window creation failed, error='%s'\n",SDL_GetError());
		throw 1;
	}
//	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
//	if(!renderer)
//	{
//		fprintf(stderr, "SDL renderer creation failed, error='%s'\n",SDL_GetError());
//		return 1;
//	}
	context = SDL_GL_CreateContext(window);
	if(!context)
	{
		fprintf(stderr, "SDL GL context creation failed, error='%s'\n",SDL_GetError());
		SDL_DestroyWindow(window);
		throw 1;
	}

	Renderer.reset(new Render_OpenGL());
	
	//SDL_GL_SetSwapInterval(1);
	SDL_GL_SetSwapInterval(0);
	((Render_OpenGL*)Renderer.get())->InitGL();

	printf("Surface SDL2\n");
}

Surface_SDL::~Surface_SDL()
{
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Surface_SDL::Present()
{
	ZoneScoped;
	//MEASURE();
	Renderer->Render();
	{
	ZoneScopedN("SDL_GL_SwapWindow");
	// timer_helper("SDL_GL_SwapWindow");
	SDL_GL_SwapWindow(window);
	}
	HandleInput();
	//MainLoop(0,nullptr);
}

ISurface::Properties Surface_SDL::GetProperties()
{
	return {{0,0},{0,0}};
}

bool Surface_SDL::SetProperties(ISurface::Properties /*p*/)
{
	return false;
}

IRenderer* Surface_SDL::GetRenderer()
{
	ZoneScoped;
	//MEASURE();
	return Renderer.get();
}

void Surface_SDL::HandleInput()
{
	ZoneScoped;
	//MEASURE();
	SDL_Event e;
	while(SDL_PollEvent(&e))
	{
		if(e.type == SDL_QUIT)
		{
			SDL_DestroyWindow(window);
			SDL_Quit();
			::exit(1);
		}
	}
}

// ---------- o ----------

/*int Surface_SDL::MainLoop(int argc, char** argv)
{
	//SDL_Point circle[41];
	for(int i=0; i<40; ++i)
	{
		double rad=2.0*PI/40.0*double(i);
		circle[i].x=0.1*cos(rad);
		circle[i].y=0.1*sin(rad);
	}
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr,"SDL could not initialize, error='%s'\n",SDL_GetError());
		return 1;
	}
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_Window* window = nullptr;
	//SDL_Renderer* renderer = nullptr;
	SDL_GLContext context = nullptr;
	window = SDL_CreateWindow("try",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			400,
			400,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
//	if(!SDL_CreateWindowAndRenderer(400,400,0,&window,&renderer))
//	{
//		fprintf(stderr,"failed to create window, or renderer %s\n", SDL_GetError());
//		return 1;
//	}
	if(!window)
	{
		fprintf(stderr, "SDL window creation failed, error='%s'\n",SDL_GetError());
		return 1;
	}
//	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
//	if(!renderer)
//	{
//		fprintf(stderr, "SDL renderer creation failed, error='%s'\n",SDL_GetError());
//		return 1;
//	}
	context = SDL_GL_CreateContext(window);
	if(!context)
	{
		fprintf(stderr, "SDL GL context creation failed, error='%s'\n",SDL_GetError());
		SDL_DestroyWindow(window);
		return 1;
	}
	
	SDL_GL_SetSwapInterval(1);
	InitGL();

	//SDL_Surface* surface = SDL_GetWindowSurface(window);
	//SDL_FillRect(surface,nullptr,SDL_MapRGB(surface->format,0xff,0xff,0xff));
	SDL_Event e;
	unsigned char c=0xff;
	while(1)
	{
//		SDL_SetRenderDrawColor(renderer,0,0,0,SDL_ALPHA_OPAQUE);
//		SDL_RenderClear(renderer);
//		SDL_SetRenderDrawColor(renderer,c,0x7f+c,0xff-c,SDL_ALPHA_OPAQUE);
//		SDL_RenderDrawLines(renderer,circle,41);
//		SDL_RenderPresent(renderer);
		Update();
		Render();
		SDL_GL_SwapWindow(window);
		c--;
		//SDL_UpdateWindowSurface(window);
		while(SDL_PollEvent(&e))
		{
			if(e.type == SDL_QUIT)
			{
				SDL_DestroyWindow(window);
				SDL_Quit();
				return 0;
			}
		}
	}
	return 0;
}
*/
