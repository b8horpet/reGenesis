#include "../SurfaceCommon/ISurface.h"
#include "Surface_SDL.h" // TODO FIX INCLUDES ASAP!!!
#include "../Renderers/OpenGL.h"

#include <SDL.h>
#include <SDL_opengl.h>
#include <cstdio>
#include <cstdlib>

static SDL_Window* window = nullptr;
static SDL_GLContext context = nullptr;


constexpr int window_size = 1000;

extern bool g_Paused;

Surface_SDL::Surface_SDL()
: Renderer(nullptr)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr,"SDL could not initialize, error='%s'\n",SDL_GetError());
		throw "SDL init error";
	}
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	//SDL_Renderer* renderer = nullptr;
	window = SDL_CreateWindow("reGenesis",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			window_size,
			window_size,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
//	if(!SDL_CreateWindowAndRenderer(window_size,window_size,0,&window,&renderer))
//	{
//		fprintf(stderr,"failed to create window, or renderer %s\n", SDL_GetError());
//		return 1;
//	}
	if(!window)
	{
		fprintf(stderr, "SDL window creation failed, error='%s'\n",SDL_GetError());
		throw "SDL window creation error";
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
		throw "SDL GL context creation error";
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
	bool should_exit = false;
	while(SDL_PollEvent(&e))
	{
		switch(e.type)
		{
		case SDL_QUIT:
		{
			should_exit = true;
		} break;
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			const SDL_KeyboardEvent& k = e.key;
			if(k.state == SDL_PRESSED)
			{
				switch(k.keysym.sym)
				{
				case SDLK_ESCAPE:
				{
					should_exit = true;
				} break;
				case SDLK_f:
				{
					if(k.repeat == 0)
					{
						fullscreen = !fullscreen;
						SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN/*_DESKTOP*/ : 0);
						// handle centered?
					}
				} break;
				case SDLK_w:
				case SDLK_UP:
					Renderer->MoveOffset(Vec2{0.,1.});
					break;
				case SDLK_s:
				case SDLK_DOWN:
					Renderer->MoveOffset(Vec2{0.,-1.});
					break;
				case SDLK_a:
				case SDLK_LEFT:
					Renderer->MoveOffset(Vec2{-1.,0.});
					break;
				case SDLK_d:
				case SDLK_RIGHT:
					Renderer->MoveOffset(Vec2{1.,0.});
					break;
				case SDLK_KP_PLUS:
					Renderer->ZoomScale(.75);
					break;
				case SDLK_KP_MINUS:
					Renderer->ZoomScale(1.0/.75);
					break;
				case SDLK_HOME:
					Renderer->ResetView();
					break;
				case SDLK_SPACE:
				{
					if(k.repeat == 0)
					{
						g_Paused = !g_Paused;
					}
				} break;
				}
			}
		} break;
		}
	}
	if(should_exit)
	{
		SDL_DestroyWindow(window);
		SDL_Quit();
		::exit(1);
	}
}

void Surface_SDL::SaveBitmap(const std::string& path)
{
	SDL_Surface *sshot = SDL_CreateRGBSurface(0, window_size, window_size, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
	SDL_RenderReadPixels(SDL_GetRenderer(window), NULL, SDL_PIXELFORMAT_ARGB8888, sshot->pixels, sshot->pitch);
	SDL_SaveBMP(sshot, path.c_str());
	SDL_FreeSurface(sshot);
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
