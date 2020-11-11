#define B8_UTIL_IMPL
#include "b8.hpp"
#undef B8_UTIL_IMPL

#include "Physics/World.h"
#include "Graphics/SurfaceCommon/ISurface.h"
#include "Graphics/Renderers/IRenderer.h"

#include <cstdio>
#include <memory>
#include <string>

#include <random>

std::random_device rd;
std::mt19937 gen(rd());

constexpr double Earth_R = 10.;
const Vec3d grav_center{0, -70. - Earth_R , 0};

class Boid : public Sphere
{
public:
    Boid(Vec3d p=Vec3d())
        : Sphere(p)
    {
        mFrictionCoeffs[0] = 0.001;
        mFrictionCoeffs[1] = 0.0001;
        mFrictionCoeffs.push_back(0.0001);
    }
    virtual void Logic(const std::vector<std::shared_ptr<Object>>& objs) override
    {
        mColor = {1,1,1,1};
        Sphere::Logic(objs);
    }
    virtual std::shared_ptr<PhysEffect> DoCollision(const Object* other) override
    {
        auto ret = Sphere::DoCollision(other);
        ret->dV *= .2;
        return ret;
    }
    virtual void DoEffect(std::shared_ptr<PhysEffect> e) override
    {
        mColor = {1,0,0,1};
        Sphere::DoEffect(e);
    }
};

class Void : public Sphere
{
public:
    Void(Vec3d p=Vec3d())
        : Sphere(p)
    {
        mRadius = Earth_R;
        mMass = mRadius*mRadius;
        mColor = {.5,.5,0,.5};
    }
    virtual void DoEffect(std::shared_ptr<PhysEffect> e) override
    {}
    virtual void Physics(double dT) override
    {}
};

class GravWorld : public World
{
    using World::World;
    virtual void Physics(double dT) override
    {
        for(auto o : mObjects)
        {
            std::shared_ptr<Sphere> s=std::dynamic_pointer_cast<Sphere>(o);
            const Vec3d dp = grav_center - s->mPosition;
            s->mAcceleration = dp * Earth_R * Earth_R / std::pow(dp.Length(),3);
        }
        World::Physics(dT);
    }
};

long N = 0;

extern bool g_Paused;

int main(int argc, char* argv[])
{
    ZoneScopedN("main");
    if(argc < 2)
    {
        fprintf(stderr, "usage: %s <number of boids>\n", argv[0]);
        return 1;
    }
    try
    {
        N = std::stol(argv[1]);
    }
    catch(...)
    {
        fprintf(stderr, "cannot convert '%s' to integer\n", argv[1]);
        return 1;
    }
    N = std::min(std::max(N, 1l), 1000l);
    printf("N= %ld\n", N);
    GravWorld theWorld(argc > 2 ? argv[2] : "");
    std::unique_ptr<ISurface> theSurface;
    {
    ZoneScopedN("init");
    // g_Paused = true;
    // for(int i=0; i<9; ++i)
    //     for(int j=0; j<9; ++j)
    //     {
    //         const int x = 2*i-8;
    //         const int y = 2*j-8;
    //         if(x == 0 && y == 0) continue;
    //         theWorld.AddObject(std::make_shared<Boid>(Vec3d{x,y,0}));
    //     }
    theWorld.AddObject(std::make_shared<Void>(grav_center));
    const double random_noise = std::sqrt(N)*10.0;
    std::uniform_real_distribution<> dis(-random_noise, random_noise);
    for(int i=0; i<N; i++)
    {
        theWorld.AddObject(std::make_shared<Boid>(Vec3d{dis(gen),dis(gen),dis(gen)/*0*/}));
    }
    theSurface=CreateSurface("SDL2");
    }
    {
    ZoneScopedN("run");
    for(int i=0; i<10000;)
    // while(!theWorld.mObjects.empty())
    {
        theWorld.Activate();
        if(!g_Paused) ++i;
		if(theSurface)
		{
			ZoneScopedN("render");
			IRenderer* Renderer=theSurface->GetRenderer();
			if(Renderer)
			{
                RenderData render_data;
                theWorld.FillRenderData(render_data);
                DebugDrawer::Get().FillRenderData(render_data);
				Renderer->UpdateData(render_data);
			}
			theSurface->Present();
        }
        FrameMark;
    }
    }
    return 0;
}