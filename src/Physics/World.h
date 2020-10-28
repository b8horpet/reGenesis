#pragma once
#ifndef PHYSICS_WORLD_H_INCLUDED
#define PHYSICS_WORLD_H_INCLUDED

#include "Basics.h"
#include "SharedData.h"
#include "Object.h"
#include "IGeometry.h"
#include "KDTree.h"
#include "Creature/Creature.h"
#include <utility>
#include <vector>
#include <set>
#include <map>
#include <memory>
#include <algorithm>
#include <cassert>

class World
{
public:
	// this ain't a good idea
	std::vector<std::shared_ptr<Object>> mObjects;
	std::weak_ptr<Object> LookUp(Object::ObjectID id) const
	{
		if(mObject_lookup.count(id) == 1)
		{
			return mObject_lookup.at(id);
		}
		return {};
	}
private:
	std::map<Object::ObjectID, std::weak_ptr<Object>> mObject_lookup;
	size_t mObjLimit;
	double mSize;
	unsigned long mTickCnt;
#ifdef DEBUG
	Vec3 tkp;
	Vec3 dtkp;
#endif

public:
	std::unique_ptr<IGeometry> mGeometry;

	class TileGeometry : public IGeometry
	{
	public:
		struct Tile
		{
		private:
			Tile();
		public:
			std::pair<long long,long long> mCoord;
			//std::vector<std::weak_ptr<Object>> mObjects;
			std::vector<Object*> mObjects;
			Tile(long long x, long long y) : mCoord(x,y) {}
			void AddObject(Object* o)
			{
				for(auto x : mObjects)
					assert(x->mID!=o->mID);
				mObjects.push_back(o);
			}
			std::set<std::pair<Object*,Object*>> Collide() const
			{
				std::set<std::pair<Object*,Object*>> colls;
				for(size_t i=0; i<mObjects.size()-1; ++i)
					for(size_t j=i+1; j<mObjects.size(); ++j)
						if(mObjects[i]->Collide(mObjects[j]))
							colls.insert(std::make_pair(mObjects[i],mObjects[j]));
				return colls;
			}
		};
		virtual ~TileGeometry(){}
		TileGeometry()
			: mTileSize(0)
		{}
		virtual void DoCollisions(std::vector<std::shared_ptr<Object>> os) override
		{
			BroadPhase(os);
			NarrowPhase();
		}
		long long mTileSize;
	private:
		std::map<std::pair<long long,long long>,Tile> mTiles;
		Tile& GetTile(double x, double y);
		void BroadPhase(std::vector<std::shared_ptr<Object>> os);
		void NarrowPhase();
	};
	class Geometry_RDC : public IGeometry
	{
	public:
		Geometry_RDC()
			: mClusters()
		{}
		virtual ~Geometry_RDC() override
		{}
		virtual void DoCollisions(std::vector<std::shared_ptr<Object>> os) override
		{
			BroadPhase(os);
			NarrowPhase();
		}
	private:
		std::vector<std::pair<std::vector<std::shared_ptr<Object>>,std::array<std::pair<double,double>,2>>> mClusters;
		void BroadPhase(std::vector<std::shared_ptr<Object>> os);
		void NarrowPhase();
	};
	/*
	 * ... and the programmer called the constructor, and there was World
	 */
	World(const std::string& geometry = "")
		: mObjects()
		, mObjLimit(1000)
		, mSize(25.0)
		, mTickCnt(0)
		, mGeometry()
	{
		if(geometry == "RDC" || geometry == "" /*default*/)
		{
			mGeometry.reset(new Geometry_RDC());
		}
		else if(geometry == "tile")
		{
			mGeometry.reset(new TileGeometry());
		}
		else
		{
			throw "not implemented";
		}
		//self.Random = random
	}

	void AddObject(std::shared_ptr<Object> o)
	{
		mObjects.push_back(o);
		assert(mObject_lookup.count(o->mID) == 0);
		mObject_lookup.emplace(o->mID, o);
	}

	void RemoveObject(std::shared_ptr<Object> o)
	{
		//intentionally don't want to check if it's still in there
		assert(mObject_lookup.count(o->mID) == 1);
		mObject_lookup.erase(o->mID);
		mObjects.erase(std::find(mObjects.begin(),mObjects.end(),o));
	}

	virtual void Physics(double dT);
	virtual void Logic();
	virtual void Spawn();
	virtual void Activate();
	virtual std::vector<std::shared_ptr<ObjectData>> GetRenderData();
};

#endif
