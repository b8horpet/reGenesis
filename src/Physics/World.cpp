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
	//std::array<char,2> dimensions={'x','y'}; // why???
	typedef std::tuple<
				std::vector<std::shared_ptr<Object>>,
				std::array<bool,2>,
				std::array<std::pair<double,double>,2>
			> clusterfuck_t;
	std::vector<clusterfuck_t> dirtyClusters;
	clusterfuck_t t;
	std::get<0>(t)=os;
	std::get<1>(t)[0]=true;
	std::get<1>(t)[1]=true;
	std::get<2>(t)[0]=std::make_pair(0.,0.);
	std::get<2>(t)[1]=std::make_pair(0.,0.);

	dirtyClusters.push_back(t);
	int dim=0;
	while(!dirtyClusters.empty())
	{
		//char dc=dimensions[dim];
		for(int i=dirtyClusters.size(); i-->0;)
		{
			auto c=dirtyClusters.back();
			dirtyClusters.pop_back();
			auto& c1=std::get<1>(c);
			if(std::find(c1.begin(),c1.end(),true)==c1.end())
			{
				mClusters.push_back(std::make_pair(std::get<0>(c),std::get<2>(c)));
				continue;
			}
			auto& c0=std::get<0>(c);
			std::sort(c0.begin(),c0.end(),[dim](const auto& a, const auto& b){return a->GetPosition()[dim] < b->GetPosition()[dim];});
			double maxD=c0[0]->GetBoundingBox().second[dim];
			double minD=c0[0]->GetBoundingBox().first[dim];
			std::vector<std::pair<int,std::pair<double,double>>> ClusterBoundaries;
			for(long j=0; j<(long)c0.size(); ++j)
			{
				auto o=c0[j];
				auto bb = o->GetBoundingBox();
				double od_r=bb.second[dim];
				double od_l=bb.first[dim];
				if(od_l > maxD)
				{
					ClusterBoundaries.push_back(std::make_pair(j,std::make_pair(minD,maxD)));
					maxD=od_r;
					minD=od_l;
				}
				else if(od_r > maxD)
				{
					maxD=od_r;
				}
			}
			auto& c2=std::get<2>(c);
			if(ClusterBoundaries.empty())
			{
				c1[dim]=false;
				c2[dim].first=minD;
				c2[dim].second=maxD;
				dirtyClusters.push_back(c);
			}
			else
			{
				ClusterBoundaries.push_back(std::make_pair(c0.size(),std::make_pair(minD,maxD)));
				int LastBound=0;
				for(auto& j : ClusterBoundaries)
				{
					clusterfuck_t tmp;
					auto& tmp0=std::get<0>(tmp);
					tmp0.insert(tmp0.begin(),c0.begin()+LastBound,c0.begin()+j.first);
					auto& tmp1=std::get<1>(tmp);
					for(int x=0; x<2; ++x)
						tmp1[x]=x!=dim;
					auto& tmp2=std::get<2>(tmp);
					tmp2=c2;
					tmp2[dim]=j.second;
					dirtyClusters.push_back(tmp);
					LastBound=j.first;
				}
			}
		}
		dim+=1;
		dim=dim%2;
	}
/*
	self.Clusters=[]
	#could be done on separate threads
	dimensions=['x','y'] # only 2 dimension
	#this is fragile, should be indexed with numbers
	dirtyClusters=[[os,[True for d in dimensions],[(0,0) for d in dimensions]]]
	dim=0
	while dirtyClusters:
		dc=dimensions[dim]
		#print("pass %s %d" % (dc,len(dirtyClusters)))
		for i in reversed(range(len(dirtyClusters))):
			c=dirtyClusters.pop(i)
			if True not in c[1]:
				self.Clusters.append((c[0],c[2]))
				continue
			c[0].sort(key=partial(World.Geometry_RDC.get_pos_by_dim,dc))
			clusterBoundaries=[]
			maxD=getattr(c[0][0].GetBoundingBox()[1],dc)
			minD=getattr(c[0][0].GetBoundingBox()[0],dc)
			for j,o in enumerate(c[0]):
				bb=o.GetBoundingBox()
				od_r=getattr(bb[1],dc)
				od_l=getattr(bb[0],dc)
				if od_l > maxD:
					clusterBoundaries.append((j,(minD,maxD)))
					maxD=od_r
					minD=od_l
				elif od_r > maxD:
					maxD=od_r
			if not clusterBoundaries:
				c[1][dim]=False
				c[2][dim]=(minD,maxD)
				dirtyClusters.append(c)
			else:
				clusterBoundaries.append((len(c[0]),(minD,maxD)))
				lastbound=0
				for j in clusterBoundaries:
					currCluster=[c[0][lastbound:j[0]],[x!=dim for x in range(len(dimensions))],[i for i in c[2]]]
					currCluster[2][dim]=j[1]
					dirtyClusters.append(currCluster)
					lastbound=j[0]
		dim+=1
		dim%=len(dimensions)
*/
}

void World::Geometry_RDC::NarrowPhase()
{
	ZoneScopedN("Geometry_RDC::NarrowPhase");
	std::set<std::pair<Object*,Object*>> colls;
	auto& dd = DebugDrawer::Get();
	for( auto& cl : mClusters)
	{
		dd.DrawBox(Vec3d{cl.second[0].first, cl.second[1].first, 0.0}, Vec3d{cl.second[0].second, cl.second[1].second, 0.0});
		for(size_t i=0; i<cl.first.size()-1; ++i)
			for(size_t j=i+1; j<cl.first.size(); ++j)
						if(cl.first[i]->Collide(cl.first[j].get()))
						{
							dd.DrawLine(cl.first[i]->GetPosition(), cl.first[j]->GetPosition());
							colls.insert(std::make_pair(cl.first[i].get(),cl.first[j].get()));
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
