#pragma once
#ifndef PHYSICS_OBJECT_H_INCLUDED
#define PHYSICS_OBJECT_H_INCLUDED

#include "Basics.h"
#include <utility>
#include <vector>
#include <cmath>
#include <memory>
#include <cassert>

struct PhysEffect
{
	Vec3d dP,dV,dA;
	double dM,dR;
	PhysEffect(Vec3d p=Vec3d(), Vec3d v=Vec3d(), Vec3d a=Vec3d(), double m=0.0, double r=0.0)
		: dP(p)
		, dV(v)
		, dA(a)
		, dM(m)
		, dR(r)
	{}
};

class Object : public std::enable_shared_from_this<Object>
{
public:
	typedef long ObjectID;
private:
	static ObjectID ID;
//protected:
public:
	const ObjectID mID;
public:
	Object()
		: mID(++ID)
	{}
	virtual ~Object() {}
	// Compute next physical state based on current state
	virtual void Physics(double dT) = 0;
	// Compute next logical state based on current state and other objects
	virtual void Logic(const std::vector<std::shared_ptr<Object>>& objs) = 0;
	// Compute collision with other object
	virtual bool Collide(const Object* other) const = 0;
	// Handles the collision with the other object
	virtual std::shared_ptr<PhysEffect> DoCollision(const Object* other) = 0;
	// Applies the effect to the object
	virtual void DoEffect(std::shared_ptr<PhysEffect> e) = 0;
	// Returns the topleft and bottomright corner of the bounding box
	virtual std::pair<Vec3d,Vec3d> GetBoundingBox() const = 0;
	// Returns the center position of the object
	virtual Vec3d GetPosition() const = 0;
};

class Sphere : public Object
{
public:
	double mMass;
	double mRadius;
	Vec3d mPosition;
	Vec3d mVelocity;
	Vec3d mAcceleration;
	std::vector<double> mFrictionCoeffs; // refactor!!!
	Clr4d mColor;
	bool mAlive;
public:
	Sphere(Vec3d p=Vec3d())
		: Object()
		, mMass(1.0)
		, mRadius(1.0)
		, mPosition(p)
		, mVelocity()
		, mAcceleration()
		, mFrictionCoeffs{ {0.0,0.0} } // first is linear second is quadratic (like in water)
		, mColor{ {1,1,1,1} }
		, mAlive(true)
	{
		//might be better to store prev pos, than acceleration
	}

	virtual void Physics(double dT) override
	{
		//Eulerish(dT);
		RK4(dT);
	}

	virtual void Logic(const std::vector<std::shared_ptr<Object>>& /*objs*/) override
	{
		// intentionally do nothing
	}

	void Eulerish(double dT)
	{
		Vec3d ad;
		/*for c in enumerate(self.Frics):
			ad+=-1*c[1]*(abs(self.Vel)**c[0])*self.Vel*/
		for(size_t i=0; i<mFrictionCoeffs.size(); ++i)
		{
			ad+=mVelocity*pow(mVelocity.Length(),i)*mFrictionCoeffs[i]*-1.0;
		}
		ad/=mMass;
		mAcceleration+=ad;
		mPosition+=mVelocity*dT+mAcceleration*((dT*dT)/2);
		mVelocity+=mAcceleration*dT;
		mAcceleration=Vec3d();
	}

	void RK4(double dT)
	{
		Vec3d ad;
		for(size_t i=0; i<mFrictionCoeffs.size(); ++i)
		{
			ad+=mVelocity*pow(mVelocity.Length(),i)*mFrictionCoeffs[i]*-1.0;
		}
		ad/=mMass;
		mAcceleration+=ad;
		Vec3d k1[2]={mVelocity*dT,mAcceleration*dT};
		Vec3d k2[2]={(mVelocity+k1[0]*0.5)*dT,(mAcceleration+k1[1]*0.5)*dT};
		Vec3d k3[2]={(mVelocity+k2[0]*0.5)*dT,(mAcceleration+k2[1]*0.5)*dT};
		Vec3d k4[2]={(mVelocity+k3[0])*dT,(mAcceleration+k3[1])*dT};
		mPosition+=(k1[0]+k2[0]*2.0+k3[0]*2.0+k4[0])*(1.0/6.0);
		mVelocity+=(k1[1]+k2[1]*2.0+k3[1]*2.0+k4[1])*(1.0/6.0);
		mAcceleration=Vec3d();
	}

	virtual bool Collide(const Object* other) const override
	{
		//Object.NumCollTests+=1
		const Sphere* other_sphere = dynamic_cast<const Sphere*>(other);
		assert(mID!=other_sphere->mID);
		// intentionally allow crashing
		double rad_sum=mRadius+other_sphere->mRadius;
		Vec3d pos_diff=mPosition-other_sphere->mPosition;
		if(std::abs(pos_diff[0]) > rad_sum ||
			std::abs(pos_diff[1]) > rad_sum ||
			std::abs(pos_diff[2]) > rad_sum)
		{
			return false;
		}
		// technically pos_diff.Length() < rad_sum, but sqrt is expensive
		bool collision= pos_diff.Dot(pos_diff) < rad_sum*rad_sum;
		//if(collision)
		//	fprintf(stderr,"collision: %ldx%ld %lf (%lf,%lf)\n",mID,other_sphere->mID,rad_sum,pos_diff[0],pos_diff[1]);
		return collision;
	}

	virtual std::shared_ptr<PhysEffect> DoCollision(const Object* other) override
	{
		//Object.NumColls+=1
		//TODO WTF
		const Sphere* other_sphere = dynamic_cast<const Sphere*>(other);
		// intentionally allow crashing
		Vec3d d=mPosition-other_sphere->mPosition;
		d.Normalize();
		Vec3d vtkp=(mVelocity*mMass+other_sphere->mVelocity*other_sphere->mMass)/(mMass+other_sphere->mMass);
		Vec3d v1=other_sphere->mVelocity-vtkp;
		Vec3d vm=d*(v1.Dot(d));
		Vec3d vp=v1-vm;
		Vec3d vcorr=vp-vm;
		vcorr+=vtkp;

		Vec3d pcorr=d*-1.0;
		double dd=mRadius+other_sphere->mRadius;
		dd-=(other_sphere->mPosition-mPosition).Length();
		dd*=mMass;
		dd/=(other_sphere->mMass+mMass);
		pcorr*=dd;
		vcorr-=other_sphere->mVelocity;
		return std::shared_ptr<PhysEffect>(new PhysEffect(pcorr,vcorr));
	}

	virtual void DoEffect(std::shared_ptr<PhysEffect> effect) override
	{
		// intentionally allow crashing
		PhysEffect& e=*effect;
		mPosition+=e.dP;
		mVelocity+=e.dV;
		mAcceleration+=e.dA;
		mMass+=e.dM;
		mRadius+=e.dR;
	}

	virtual std::pair<Vec3d,Vec3d> GetBoundingBox() const override
	{
		return std::make_pair(mPosition-Vec3d{ {mRadius,mRadius,mRadius} }, mPosition+Vec3d{ {mRadius,mRadius,mRadius} }); 
	}

	virtual Vec3d GetPosition() const override
	{
		return mPosition;
	}
};

#endif
