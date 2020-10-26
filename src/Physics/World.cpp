#include "World.h"
#include <chrono>

World::TileGeometry::Tile& World::TileGeometry::GetTile(double x, double y)
{
	std::pair<int,int> c(int(floor(x/double(mTileSize))),int(floor(y/double(mTileSize))));
	if(mTiles.count(c)==0)
		mTiles.emplace(std::make_pair(c,Tile(c.first,c.second)));
	return mTiles.at(c);
}

void World::TileGeometry::BroadPhase(std::vector<std::shared_ptr<Object>> os)
{
	ZoneScopedN("TileGeometry::BroadPhase");
	mTiles.clear();
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
			Vec<2,int> corner{t.mCoord.first+additional['x'], t.mCoord.second+additional['y']};
			corner*=mTileSize;
			if((Vec3{(double)corner[0],(double)corner[1],0}-o->mPosition).Length() < o->mRadius)
			{
				// ermagherd it's sooo bad!!!
				GetTile(corner[0]+additional['x']*2-1,corner[1]+additional['y']*2-1).AddObject(o);
			}
		}
		/*
		   additional={}
		   if t.x*self.TileSize>o.Pos.x-o.Radius:
		   self.GetTile(o.Pos.x-o.Radius,o.Pos.y).AddObject(o)
		   additional['x']=0
		   if (t.x+1)*self.TileSize<o.Pos.x+o.Radius:
		   self.GetTile(o.Pos.x+o.Radius,o.Pos.y).AddObject(o)
		   additional['x']=1
		   if t.y*self.TileSize>o.Pos.y-o.Radius:
		   self.GetTile(o.Pos.x,o.Pos.y-o.Radius).AddObject(o)
		   additional['y']=0
		   if (t.y+1)*self.TileSize<o.Pos.y+o.Radius:
		   self.GetTile(o.Pos.x,o.Pos.y+o.Radius).AddObject(o)
		   additional['y']=1
		   if len(additional) == 2:
		   corner=Vector3D(t.x+additional['x'],t.y+additional['y'])
		   corner*=self.TileSize
		   if abs(corner-o.Pos)<o.Radius:
		   cx=corner.x+additional['x']*2-1
		   cy=corner.y+additional['y']*2-1
		   self.GetTile(cx,cy).AddObject(o)
		   */
	}
}

void World::TileGeometry::NarrowPhase()
{
	ZoneScopedN("TileGeometry::NarrowPhase");
	std::set<std::pair<Object*,Object*>> colls;
	for( auto p : mTiles)
	{
		auto c=p.second.Collide();
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
	std::vector<std::tuple<std::vector<std::shared_ptr<Object>>,std::array<bool,2>,std::array<std::pair<double,double>,2>>> dirtyClusters;
	typedef std::tuple<std::vector<std::shared_ptr<Object>>,std::array<bool,2>,std::array<std::pair<double,double>,2>> clusterfuck_t;
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
	for( auto& cl : mClusters)
	{
		for(size_t i=0; i<cl.first.size()-1; ++i)
			for(size_t j=i+1; j<cl.first.size(); ++j)
						if(cl.first[i]->Collide(cl.first[j].get()))
							colls.insert(std::make_pair(cl.first[i].get(),cl.first[j].get()));
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
	ZoneScoped;
	//MEASURE();
	Spawn();
	Logic();
	for(int i=0; i<5; ++i)
	{
		mTickCnt++;
		Physics(0.01);
	}
}


std::vector<std::shared_ptr<ObjectData>> World::GetRenderData()
{
	ZoneScoped;
	//MEASURE();
	std::vector<std::shared_ptr<ObjectData>> ret;
	for(auto o : mObjects)
	{
		std::shared_ptr<SphereData> s(new SphereData());

		s->Center=o->GetPosition();
		s->Radius=1.0;
		s->Meta.Color=Vec3{1.0,0.0,0.0};
		ret.push_back(s);
	}
	return ret;
}


/*
import cProfile, pstats
import pickle
from functools import partial


def ConstantFrics(p: Vector3D) -> float:
	return 0.05,0.3


class World:
	class IGeometry:
		def GetDebugData(self):
			pass

	class Tile:
		def __init__(self, x: int, y: int):
			self.x=x
			self.y=y
			self.count=0
			self.Objects=[]

		def AddObject(self, o):
			self.count+=1
			self.Objects.append(o)

		def Collide(self):
			if self.count == 1:
				return set()
			colls=set()
			for i in range(0,len(self.Objects)):
				for j in range(i+1,len(self.Objects)):
					if self.Objects[i].Collide(self.Objects[j]) == True:
						colls.add((self.Objects[i],self.Objects[j]))
			return colls

	class Geometry(IGeometry):
		def __init__(self):
			self.UpdateFrics=ConstantFrics
			self.TileSize=5
			self.Tiles={}

		def GetTile(self, x: float, y: float):
			tx=int(np.floor(x))//self.TileSize
			ty=int(np.floor(y))//self.TileSize
			if (tx,ty) not in self.Tiles:
				self.Tiles[(tx,ty)]=World.Tile(tx,ty)
			return self.Tiles[(tx,ty)]

		def GetDebugData(self):
			return [(self.Tiles[t].x*self.TileSize,self.Tiles[t].y*self.TileSize,(self.Tiles[t].x+1)*self.TileSize,(self.Tiles[t].y+1)*self.TileSize) for t in self.Tiles]

		def DoCollisions(self, os):
			self.BroadPhase(os)
			self.NarrowPhase()

		def BroadPhase(self, os):
			self.Tiles={} # sry gc
			for o in os:
				t=self.GetTile(o.Pos.x,o.Pos.y)
				t.AddObject(o)
				additional={}
				if t.x*self.TileSize>o.Pos.x-o.Radius:
					self.GetTile(o.Pos.x-o.Radius,o.Pos.y).AddObject(o)
					additional['x']=0
				if (t.x+1)*self.TileSize<o.Pos.x+o.Radius:
					self.GetTile(o.Pos.x+o.Radius,o.Pos.y).AddObject(o)
					additional['x']=1
				if t.y*self.TileSize>o.Pos.y-o.Radius:
					self.GetTile(o.Pos.x,o.Pos.y-o.Radius).AddObject(o)
					additional['y']=0
				if (t.y+1)*self.TileSize<o.Pos.y+o.Radius:
					self.GetTile(o.Pos.x,o.Pos.y+o.Radius).AddObject(o)
					additional['y']=1
				if len(additional) == 2:
					corner=Vector3D(t.x+additional['x'],t.y+additional['y'])
					corner*=self.TileSize
					if abs(corner-o.Pos)<o.Radius:
						cx=corner.x+additional['x']*2-1
						cy=corner.y+additional['y']*2-1
						self.GetTile(cx,cy).AddObject(o)

		def NarrowPhase(self):
			colls=set()
			for p in self.Tiles:
				colls.update(self.Tiles[p].Collide())
			colleffs=[]
			for o,p in colls:
				oc=o.DoCollision(p)
				pc=p.DoCollision(o)
				colleffs.append(((o,oc),(p,pc)))
			for (o,oc),(p,pc) in colleffs:
				o.DoEffect(pc)
				p.DoEffect(oc)

	class Geometry_RDC(IGeometry):
		def __init__(self):
			self.UpdateFrics=ConstantFrics
			self.TileSize=5
			self.Clusters=[]

		def GetDebugData(self):
			return [(c[1][0][0],c[1][1][0],c[1][0][1],c[1][1][1]) for c in self.Clusters]
			#return [(self.Tiles[t].x*self.TileSize,self.Tiles[t].y*self.TileSize,(self.Tiles[t].x+1)*self.TileSize,(self.Tiles[t].y+1)*self.TileSize) for t in self.Tiles]

		def DoCollisions(self, os):
			self.BroadPhase(os)
			self.NarrowPhase()

		def get_pos_by_dim(d, v):
			return getattr(v.GetBoundingBox()[0],d)

		def BroadPhase(self, os):
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

		def NarrowPhase(self):
			for cbb in self.Clusters:
				c=cbb[0]
				for i in range(len(c)-1):
					o1=c[i]
					for j in range(i+1,len(c)):
						o2=c[j]
						colls=[]
						if o1.Collide(o2) == True:
							colls.append((o1,o2))
						colleffs=[]
						for o,p in colls:
							oc=o.DoCollision(p)
							pc=p.DoCollision(o)
							colleffs.append(((o,oc),(p,pc)))
						for (o,oc),(p,pc) in colleffs:
							o.DoEffect(pc)
							p.DoEffect(oc)


	def __init__(self, random):
		"""
		... and the programmer called the constructor, and there was World
		"""

		self.Random = random
		self.Objects = []
		self.Creatures = []
		self.ObjLimit=1000
		self.Size=25.0
		self.TickCnt=0
		#self.Geometry = World.Geometry()
		self.Geometry = World.Geometry_RDC()
		if DEBUG:
			self.tkp=None
			self.dtkp=None

	def AddObject(self,o):
		self.Objects.append(o)
		if type(o) == Creature:
			self.Creatures.append(o)

	def RemoveObject(self,o):
		self.Objects.remove(o)
		if type(o) == Creature:
			self.Creatures.remove(o)

	def Physics(self, dT: float) -> None:
		dead=[]
		self.Geometry.TileSize=0.0
		for o in self.Objects:
			if o.Alive:
				o.Frics=self.Geometry.UpdateFrics(o.Pos)
				if self.Geometry.TileSize<o.Radius:
					self.Geometry.TileSize=o.Radius
				o.Physics(dT)
			else:
				dead.append(o)
		for d in dead:
			self.RemoveObject(d)
		self.Geometry.TileSize*=2.0
		self.Geometry.TileSize=int(np.round(self.Geometry.TileSize))+1
		self.Geometry.DoCollisions(self.Objects)

	def Logic(self):
		for c in self.Creatures:
			c.UpdateInputs(None)
			for oj in self.Objects:
				if c is oj:
					continue
				c.UpdateInputs(oj)
			c.Logic()

	def Spawn(self):
		if len(self.Objects)<self.ObjLimit:
			r=self.Random.uniform(0.0, 100.0)
			if r < 10:
				o=Food()
				o.Pos=Vector3D(self.Random.uniform(-self.Size, self.Size), self.Random.uniform(-self.Size, self.Size))
				self.AddObject(o)


	def Activate(self):
		if PROFILE:
			pr = cProfile.Profile()
			pr.enable()
		for i in range(0,5):
			self.TickCnt+=1
			self.TickCnt%=5
			if self.TickCnt == 1:
				self.Spawn()
			self.Physics(0.01)
			if self.TickCnt == 0:
				self.Logic()
		if PROFILE:
			pr.disable()
			sortby = 'cumulative'
			ps = pstats.Stats(pr).sort_stats(sortby)
			ps.print_stats()

	def Dump(self):
		l=[]
		for o in self.Objects:
			l.append((type(o),o.Pos.x,o.Pos.y))
		return pickle.dumps(l)

	def GetRenderData(self):
		self.Activate()
			#cProfile.run('theWorld.Activate()')

		pos = []
		siz = []
		col = []
		txt = []
		# Surface2D for now
		if DEBUG:
			tkp=Vector3D()
			mt=0.0
		for o in self.Objects:
			pos.append((o.Pos.x, o.Pos.y))
			if DEBUG:
				tkp+=o.Mass*o.Pos
				mt+=o.Mass
			siz.append(o.Radius)
			col.append(o.Color)
			if type(o) == Creature:
				txt.append("%3.3f %4.3f" % (o.Health, o.Energy))
			else:
				txt.append("")
		if DEBUG:
			if mt>0.0:
				tkp/=mt
				if self.tkp != None:
					if self.dtkp != None:
						if abs(abs(self.tkp-tkp)-self.dtkp)>0.001:
							#print("momentum fucked")
							self.dtkp=abs(self.tkp-tkp)
					else:
						self.dtkp=abs(self.tkp-tkp)
				self.tkp=tkp
				pos.append((tkp.x,tkp.y))
				siz.append(0.1)
				col.append((0,0,1,1))
		return pos,siz,col,txt


print("	World class imported")
*/

