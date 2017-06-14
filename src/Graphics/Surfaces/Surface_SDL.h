#pragma once
#ifndef GRAPHICS_SURFACE_SDL_H_INCLUDED
#define GRAPHICS_SURFACE_SDL_H_INCLUDED

#include "../SurfaceCommon/ISurface.h"

class Surface_SDL : public ISurface
{
public:
	Surface_SDL();
	virtual ~Surface_SDL() override;
	virtual void Present() override;
	virtual ISurface::Properties GetProperties() override;
	virtual bool SetProperties(ISurface::Properties p) override;
	virtual IRenderer* GetRenderer() override;
private:
	bool InitGL();
	void Render();
	void Update();
	int MainLoop(int argc, char** argv);
	void HandleInput();
	std::unique_ptr<IRenderer> Renderer;
};

#endif
