#include "World.h"
#include "Debug.h"
#include <chrono>

bool g_Paused = false;

World::TileGeometry::Tile& World::TileGeometry::GetTile(double x, double y)
{
	std::pair<long long,long long> c((long long)(floor(x/double(mTileSize))),(long long)(floor(y/double(mTileSize))));
	if(mTiles.count(c)==0)
		mTiles.emplace(std::make_pair(c,Tile(c.first,c.second)));
	return mTiles.at(c);
}

void World::TileGeometry::BroadPhase(std::vector<std::shared_ptr<Object>> os)
{
	ZoneScopedN("TileGeometry::BroadPhase");
	mTiles.clear();
	double max_radius = 0.0;
	for( auto obj : os)
	{
		Sphere* o = dynamic_cast<Sphere*>(obj.get());
		max_radius = std::max(o->mRadius, max_radius);
	}
	mTileSize = (long long)(std::floor(max_radius * 2.0 + 0.5)) + 1ll;
	for( auto obj : os)
	{
		Sphere* o = dynamic_cast<Sphere*>(obj.get());
		Tile& t = GetTile(o->mPosition[0],o->mPosition[1]);
		t.AddObject(o);
		// here comes the fugly part
		std::map<char,int> additional;
		if (t.mCoord.first * mTileSize > o->mPosition[0] - o->mRadius)
		{
			GetTile(o->mPosition[0] - o->mRadius, o->mPosition[1]).AddObject(o);
			additional['x']=0;
		}
		if ((t.mCoord.first+1) * mTileSize < o->mPosition[0] + o->mRadius)
		{
			GetTile(o->mPosition[0] + o->mRadius, o->mPosition[1]).AddObject(o);
			additional['x']=1;
		}
		if (t.mCoord.second * mTileSize > o->mPosition[1] - o->mRadius)
		{
			GetTile(o->mPosition[0], o->mPosition[1] - o->mRadius).AddObject(o);
			additional['y']=0;
		}
		if ((t.mCoord.second+1) * mTileSize < o->mPosition[1] + o->mRadius)
		{
			GetTile(o->mPosition[0], o->mPosition[1] + o->mRadius).AddObject(o);
			additional['y']=1;
		}
		if(additional.size() == 2)
		{
			Vec<2,long long> corner{t.mCoord.first+additional['x'], t.mCoord.second+additional['y']};
			corner*=mTileSize;
			if((Vec3d{(double)corner[0],(double)corner[1],0}-o->mPosition).Length() < o->mRadius)
			{
				// ermagherd it's sooo bad!!!
				GetTile(corner[0]+additional['x']*2-1,corner[1]+additional['y']*2-1).AddObject(o);
			}
		}
	}
}

void World::TileGeometry::NarrowPhase()
{
	ZoneScopedN("TileGeometry::NarrowPhase");
	std::set<std::pair<Object*,Object*>> colls;
	auto& dd = DebugDrawer::Get();
	for( auto p : mTiles)
	{
		dd.DrawBox(
			Vec3d{double(p.first.first * mTileSize), double(p.first.second * mTileSize), 0.0},
			Vec3d{double((p.first.first+1ll) * mTileSize), double((p.first.second+1ll) * mTileSize), 0.0});
		auto c=p.second.Collide();
		for(auto& ci : c)
		{
			dd.DrawLine(ci.first->GetPosition(), ci.second->GetPosition());
		}
		colls.insert(c.begin(),c.end());
	}
	typedef std::pair<Object*,std::shared_ptr<PhysEffect>> objeff;
	typedef std::pair<objeff,objeff> interaction;
	std::vector<interaction> colleffs;
	for (auto c : colls)
	{
		objeff a(c.first,c.first->DoCollision(c.second));
		objeff b(c.second,c.second->DoCollision(c.first));
		colleffs.push_back(interaction(a,b));
	}
	for (auto i : colleffs)
	{
		i.first.first->DoEffect(i.second.second);
		i.second.first->DoEffect(i.first.second);
	}
}


void World::Geometry_RDC::BroadPhase(std::vector<std::shared_ptr<Object>> os)
{
	ZoneScopedN("Geometry_RDC::BroadPhase");
	mClusters.clear();
	decltype(mClusters) dirtyClusters(1);
	auto& t = dirtyClusters.back();
	t.objects=os;
	for(int i=0; i<2; i++)
	{
		t.dirty[i]=true;
		auto& l = t.limits[i];
		l.Min = l.Max = 0.0;
	}
	int dim=0;
	while(!dirtyClusters.empty())
	{
		for(int i=dirtyClusters.size(); i-->0;)
		{
			auto c=dirtyClusters.back();
			dirtyClusters.pop_back();
			if(std::find(c.dirty.begin(),c.dirty.end(),true)==c.dirty.end())
			{
				mClusters.push_back(c);
				continue;
			}
			std::sort(c.objects.begin(),c.objects.end(),[dim](const auto& a, const auto& b){return a->GetBoundingBox().first[dim] < b->GetBoundingBox().first[dim];});
			const auto& bb0 = c.objects[0]->GetBoundingBox();
			DimDesc limits{bb0.first[dim], bb0.second[dim]};
			std::vector<std::pair<int,DimDesc>> ClusterBoundaries;
			for(long j=1; j<(long)c.objects.size(); ++j)
			{
				const auto& o=c.objects[j];
				const auto& bb = o->GetBoundingBox();
				const double od_r=bb.second[dim];
				const double od_l=bb.first[dim];
				if(od_l > limits.Max)
				{
					ClusterBoundaries.push_back(std::make_pair(j,limits));
					limits.Max=od_r;
					limits.Min=od_l;
				}
				else if(od_r > limits.Max)
				{
					limits.Max=od_r;
				}
			}
			if(ClusterBoundaries.empty())
			{
				c.dirty[dim]=false;
				c.limits[dim]=limits;
				dirtyClusters.push_back(c);
			}
			else
			{
				ClusterBoundaries.push_back(std::make_pair(c.objects.size(),limits));
				int LastBound=0;
				for(auto& j : ClusterBoundaries)
				{
					decltype(c) tmp;
					tmp.objects.insert(tmp.objects.begin(),c.objects.begin()+LastBound,c.objects.begin()+j.first);
					for(int x=0; x<2; ++x)
						tmp.dirty[x] = (x!=dim);
					tmp.limits = c.limits;
					tmp.limits[dim]=j.second;
					dirtyClusters.push_back(tmp);
					LastBound=j.first;
				}
			}
		}
		dim+=1;
		dim=dim%2;
	}
}

void World::Geometry_RDC::NarrowPhase()
{
	ZoneScopedN("Geometry_RDC::NarrowPhase");
	std::set<std::pair<Object*,Object*>> colls;
	auto& dd = DebugDrawer::Get();
	for( auto& cl : mClusters)
	{
		dd.DrawBox(Vec3d{cl.limits[0].Min, cl.limits[1].Min, 0.0}, Vec3d{cl.limits[0].Max, cl.limits[1].Max, 0.0});
		for(size_t i=0; i<cl.objects.size()-1; ++i)
			for(size_t j=i+1; j<cl.objects.size(); ++j)
						if(cl.objects[i]->Collide(cl.objects[j].get()))
						{
							dd.DrawLine(cl.objects[i]->GetPosition(), cl.objects[j]->GetPosition());
							colls.insert(std::make_pair(cl.objects[i].get(),cl.objects[j].get()));
						}
	}
	typedef std::pair<Object*,std::shared_ptr<PhysEffect>> objeff;
	typedef std::pair<objeff,objeff> interaction;
	std::vector<interaction> colleffs;
	for (auto c : colls)
	{
		objeff a(c.first,c.first->DoCollision(c.second));
		objeff b(c.second,c.second->DoCollision(c.first));
		colleffs.push_back(interaction(a,b));
	}
	for (auto i : colleffs)
	{
		i.first.first->DoEffect(i.second.second);
		i.second.first->DoEffect(i.first.second);
	}
}

void World::Physics(double dT)
{
	ZoneScoped;
	std::vector<std::shared_ptr<Object>> dead;
	//double TileSize=static_cast<TileGeometry*>(mGeometry.get())->mTileSize;
	//TileSize=0.0;
	{
	ZoneScopedN("Movement");
	for(auto o : mObjects)
	{
		std::shared_ptr<Sphere> s=std::dynamic_pointer_cast<Sphere>(o);
		if(s->mAlive)
		{
			//s->mFrictionCoeffs=static_cast<TileGeometry*>(mGeometry.get())->UpdateFrics(s->Pos);
			//if(TileSize<s->mRadius)
			//	TileSize=s->mRadius;
			o->Physics(dT);
			if(!(o->GetPosition().Length() < 1e10)) // for NaNs
			{
				dead.push_back(o);
			}
		}
		else
		{
			dead.push_back(o);
		}
	}
	}
	{
	ZoneScopedN("Remove dead");
	for(auto d : dead)
		RemoveObject(d);
	}
	//TileSize*=2.0;
	//static_cast<TileGeometry*>(mGeometry.get())->mTileSize=unsigned(TileSize+0.5)+1;
	mGeometry->DoCollisions(mObjects);
}

void World::Logic()
{
	ZoneScoped;
	for(auto& o : mObjects)
	{
		// todo
		o->Logic(mObjects);
	}
}

void World::Spawn()
{
	ZoneScoped;
	// todo
}


void World::Activate()
{
	if(g_Paused) return;
	ZoneScoped;
	//MEASURE();
	DebugDrawer::Get().Clear();
	Spawn();
	Logic();
	for(int i=0; i<5; ++i)
	{
		mTickCnt++;
		Physics(0.01);
	}
}


void World::FillRenderData(RenderData& renderData)
{
	ZoneScoped;
	//MEASURE();
	for(auto o : mObjects)
	{
		std::shared_ptr<Sphere> s=std::dynamic_pointer_cast<Sphere>(o);
		std::shared_ptr<SphereData> sd(new SphereData(o->GetPosition(), s->mRadius));
		sd->Meta.Color=s->mColor;
		renderData.push_back(sd);
	}
}
