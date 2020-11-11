#include "World.h"
#include "Debug.h"
#include <chrono>

bool g_Paused = false;

World::TileGeometry::Tile& World::TileGeometry::GetTile(std::array<double, World::TileGeometry::dimensions> coord)
{
	std::array<long long, World::TileGeometry::dimensions> c;
	for(int i=0; i<World::TileGeometry::dimensions; ++i)
	{
		c[i] = (long long)floor(coord[i]/double(mTileSize));
	}
	if(mTiles.count(c)==0)
		mTiles.emplace(std::make_pair(c,Tile(c)));
	return mTiles.at(c);
}

template<typename T, int D>
struct possibilities
{
private:
	std::array<T, D> const *first, *second;
	std::array<int, D+1> limit, counter;
	possibilities(std::array<T, D> const *f, std::array<T, D> const *s, const std::array<int, D+1>& l)
		: first{f}, second{s}
		, limit{l}, counter{}
	{}
public:
	possibilities(std::array<T, D> const *f, std::array<T, D> const *s, const std::array<int, D>& l)
		: first{f}, second{s}
		, limit{}, counter{}
	{
		for(int i=0; i<D; ++i)
			limit[i] = l[i];
		limit[D] = 1;
	}
	possibilities& operator++()
	{
		for(int i=0; i<D+1; ++i)
		{
			if(counter[i] < limit[i])
			{
				counter[i]++;
				return *this;
			}
			counter[i]=0;
		}
		return *this;
	}
	bool operator==(const possibilities& rhs) const
	{
		return limit == rhs.limit && counter == rhs.counter;
	}
	bool operator!=(const possibilities& rhs) const
	{
		return !(*this == rhs);
	}
	std::array<T, D> operator*() const
	{
		std::array<T, D> ret = *first;
		for(int i=0; i<D; ++i)
		{
			if(counter[i] == 1)
			{
				ret[i] = (*second)[i];
			}
		}
		return ret;
	}
	possibilities<T,D> begin() const
	{
		possibilities<T,D> ret{first, second, limit};
		return ret;
	}
	possibilities<T,D> end() const
	{
		possibilities<T,D> ret{first, second, limit};
		ret.counter[D] = 1;
		return ret;
	}
};

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
	mTileSize = (long long)(std::floor(max_radius * 2.0)) + 1ll;
	for( auto& o : os)
	{
		auto* obj = o.get();
		const auto& bb = o->GetBoundingBox();
		Tile& t = GetTile(bb.first.Data());
		std::array<int, World::TileGeometry::dimensions> next;
		for(int i=0; i<World::TileGeometry::dimensions; ++i)
		{
			next[i] = (t.mCoord[i] + 1) * mTileSize < bb.second[i] ? 1 : 0;
		}
		possibilities<double, World::TileGeometry::dimensions> poss{&bb.first.Data(), &bb.second.Data(), next};
		for(const auto& p : poss)
		{
			GetTile(p).AddObject(obj);
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
			Vec3d{double(p.first[0] * mTileSize), double(p.first[1] * mTileSize), 0.0},
			Vec3d{double((p.first[0]+1ll) * mTileSize), double((p.first[1]+1ll) * mTileSize), 0.0});
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
	decltype(mClusters) dirtyClusters(1), dirtyswap;
	auto& t = dirtyClusters.back();
	t.objects=os;
	for(int i=0; i<dimensions; i++)
	{
		t.dirty[i]=true;
		auto& l = t.limits[i];
		l.Min = l.Max = 0.0;
	}
	int dim=0;
	while(!dirtyClusters.empty())
	{
		while(!dirtyClusters.empty())
		{
			auto c=dirtyClusters.back();
			dirtyClusters.pop_back();
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
				if(std::find(c.dirty.begin(),c.dirty.end(),true)==c.dirty.end())
				{
					mClusters.push_back(c);
					continue;
				}
				dirtyswap.push_back(c);
			}
			else
			{
				ClusterBoundaries.push_back(std::make_pair(c.objects.size(),limits));
				int LastBound=0;
				for(auto& j : ClusterBoundaries)
				{
					decltype(c) tmp;
					tmp.objects.insert(tmp.objects.begin(),c.objects.begin()+LastBound,c.objects.begin()+j.first);
					for(int x=0; x<dimensions; ++x)
						tmp.dirty[x] = (x!=dim);
					tmp.limits = c.limits;
					tmp.limits[dim]=j.second;
					dirtyswap.push_back(tmp);
					LastBound=j.first;
				}
			}
		}
		std::swap(dirtyClusters, dirtyswap);
		dim+=1;
		dim=dim%dimensions;
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
