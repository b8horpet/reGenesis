#pragma once
#ifndef PHYSICS_CREATURE_H_INCLUDED
#define PHYSICS_CREATURE_H_INCLUDED

#include "Physics/Basics.h"
#include "Physics/Object.h"
#include "NeuralNet/Brain.h"
#include <random>
#include <vector>
#include <memory>

class Creature : public Sphere
{
protected:
	std::mt19937 mRandom;
	double mEnergy;
	double mHealth;
public:
	class Organ
	{
	protected:
		Creature* mParent;
	public:
		Organ(Creature* p) : mParent(p) {}
		virtual ~Organ() {}
		virtual void Activate() = 0;
		virtual bool IsInput() const = 0;
		virtual bool IsOutput() const = 0;
	};
	std::vector<std::shared_ptr<Organ>> mOrgans;
	std::shared_ptr<Brain> mBrain;

	Creature(Vec3 p=Vec3())
		: Sphere(p)
		, mRandom(mID)
		, mEnergy(300.0)
		, mHealth(100.0)
	{
		mColor[0]=std::uniform_real_distribution<double>(0.0,0.3)(mRandom);
		mColor[1]=std::uniform_real_distribution<double>(0.2,0.8)(mRandom);
		mColor[2]=std::uniform_real_distribution<double>(0.0,0.3)(mRandom);
		//mColor[3]=1.0;
		SetupBrain();
	}

	void Logic(const std::vector<std::shared_ptr<Object>>& objs) override
	{
		for(const auto& o : objs)
		{
			if(o == shared_from_this())
				continue;
			// bla bla
		}
	}

	void SetupBrain()
	{
		mBrain.reset(new Brain);
		// todo
	}
	/*
	def SetupBrain(self):
		self.Brain=Brain()
		s=Creature.Sensor(self)
		m=Creature.Motor(self)
		self.Organs.append(s)
		self.Organs.append(m)
		#ix=self.Brain.InputLayer.Neurons[0]
		#iy=self.Brain.InputLayer.Neurons[1]
		#ox=self.Brain.OutputLayer.Neurons[0]
		#oy=self.Brain.OutputLayer.Neurons[1]
		#Synapsis(ix,ox,1.0)
		#Synapsis(ix,oy,0.0)
		#Synapsis(iy,ox,0.0)
		#Synapsis(iy,oy,1.0)

		nhl=2
		for i in range(0,nhl):
			self.Brain.HiddenLayers.append(NeuronLayer())
			nhn=10
			for j in range(0,nhn):
				self.Brain.HiddenLayers[i].Neurons.append(HiddenNeuron())
		self.Brain.FillSynapsisGraph(self.Random)
	*/

	/*
	def UpdateInputs(self,o):
		for i in self.Organs:
			if i.IsInput():
				i.UpdateTarget(o)
	*/

	/*
	def Physics(self, dT: float):
		for o in self.Organs:
			o.Activate()
		if self.Energy<100.0 and self.Health > 0.0:
			self.Health-=0.5
			self.Energy+=5.0
		elif self.Energy>1000.0 and self.Health < 95.0:
			self.Energy-=20.0
			self.Health+=1.0
		if self.Health<=0.0:
			self.Mass=0.1
			if self.Energy > 0:
				self.Color=(1,1,0,1)
			else:
				self.Color=(1,0,0,1)
			self.Energy-=0.5
			if self.Energy < -1000.0:
				self.Alive=False
		super(Creature,self).Physics(dT)
	*/

	//virtual std::shared_ptr<PhysEffect> DoCollision(const Object* other) override

	/*
	def DoCollision(self, other):
		e = Creature.Interact(super(Creature,self).DoCollision(other))
		if type(other) == Food:
			pass # food does the job
		elif type(other) == Creature:
			if other.Health <= 0.0:
				if self.IsAlive():
					e.dE-=other.Energy
					e.Kill=True
			elif other.Energy > 0.0:
				e.dE-=5.0
			if self.Health <= 0.0:
				if other.IsAlive():
					e.dE+=self.Energy/10
			elif self.Energy > 0.0:
				e.dH-=self.Energy/100 # and they bite
			else:
				pass # they are soft, do not deal damage from collision
		else:
			pass
			#self.Health-=1.0 # hm, haven't thought of this
		return e
	*/

	/*
	def Logic(self):
		if self.IsAlive():
			self.Brain.Activate()
			self.Energy-=0.05
	*/

	/*
	def DoEffect(self, e):
		self.Energy+=e.dE
		self.Health+=e.dH
		if e.Kill:
			self.Alive=False
		super(Creature,self).DoEffect(e)
	*/

	/*
	def IsAlive(self):
		return self.Health>0.0 and self.Energy>0.0
	*/
};

class Food : public Sphere
{
protected:
	double mNutrient;
public:
	Food(Vec3 p=Vec3())
		: Sphere(p)
		, mNutrient(200.0)
	{
		mRadius=0.1;
		mMass=0.1;
		mColor={1.0,1.0,0.0,1.0};
	}
	virtual void Physics(double dT) override
	{
		mNutrient-=dT*10.0;
		if(mNutrient<=0.0)
			mAlive=false;
		Sphere::Physics(dT);
	}

	/*
	def DoCollision(self, other):
		e=Creature.Interact(super(Food,self).DoCollision(other))
		if type(other) == Creature:
			if other.IsAlive():
				e.dE=self.Nutrient
				self.Nutrient=0.0
				self.Alive=False
		return e
	*/
};

class Obstacle : public Sphere
{
protected:
	double mDamage;
public:
	Obstacle(Vec3 p=Vec3())
		: Sphere(p)
		, mDamage(10.0)
	{
		mRadius=3.0;
		mMass=10.0;
		mColor={0.2,0.2,0.2,1.0};
	}
	/*
	def DoCollision(self, other):
		e=Creature.Interact(super(Obstacle,self).DoCollision(other))
		if type(other) == Creature:
			if other.Health > 0.0:
				e.dH=-self.Damage
		return e
	*/
};

#endif
