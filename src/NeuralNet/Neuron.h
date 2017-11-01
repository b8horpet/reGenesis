#pragma once
#ifndef NEURALNET_NEURON_H_INCLUDED
#define NEURALNET_NEURON_H_INCLUDED

#include "Common.h"
#include <vector>
#include <functional>

class NeuronBase : public INeuralObject
{
public:
	class Synapsis : public INeuralObject
	{
	private:
		Synapsis(
			std::shared_ptr<NeuronBase> from,
			std::shared_ptr<NeuronBase> to,
			double weight = 1.0)
			: From(from)
			, To(to)
			, mWeight(weight)
			, mCurrent(0.0)
			, mEPS(0.0)
		{
		}
	public:
		std::shared_ptr<NeuronBase> From;
		std::shared_ptr<NeuronBase> To;
		double mWeight;
		double mCurrent;
		double mEPS;
		virtual void Activate() override
		{
			mCurrent=From->mOutput;
		}
		virtual void Propagate() override
		{
			From->mEPS += mEPS * mWeight;
			mWeight += INeuralObject::Braveness * mEPS * mCurrent;
			mEPS = 0.0;
		}
		static void CreateSynapsis(
			std::shared_ptr<NeuronBase> from,
			std::shared_ptr<NeuronBase> to,
			double weight = 1.0)
		{
			std::shared_ptr<Synapsis> s(new Synapsis(from,to,weight));
			to->AddInput(s);
		}
	};

	double mOutput;
	double mEPS;

	NeuronBase()
		: mOutput(0.0)
		, mEPS(0.0)
	{}

	virtual void AddInput(std::shared_ptr<NeuronBase::Synapsis> s) = 0;
};

class ConstantNeuron : public NeuronBase
{
public:
	static std::shared_ptr<NeuronBase> gConstantNeuron;
	ConstantNeuron() : NeuronBase() { mOutput=1.0; }
	virtual void Activate() override {}
	virtual void Propagate() override {}
	virtual void AddInput(std::shared_ptr<NeuronBase::Synapsis>) override {}
};

class InputNeuron : public NeuronBase
{
	std::function<double()> mInput;
public:
	InputNeuron(std::function<double()> f)
		: NeuronBase()
		, mInput(f)
	{}
	virtual void Activate() override
	{
		mOutput=mInput();
	}
	virtual void Propagate() override {}
	virtual void AddInput(std::shared_ptr<NeuronBase::Synapsis>) override {}
};

class Neuron : public NeuronBase
{
public:
	std::vector<std::shared_ptr<NeuronBase::Synapsis>> mInputs;
	double mSum;
	std::shared_ptr<IFunctionObject> mTransferFilter;
	Neuron()
		: NeuronBase()
		, mSum(0.0)
		, mTransferFilter(std::make_shared<TangentHyperbolicFunc>())
	{
	}

	virtual void Activate() override
	{
		double s=0.0;
		for(auto i : mInputs)
		{
			i->Activate();
			s += i->mWeight * i->mCurrent;
		}
		mSum=s;
		mOutput=(*mTransferFilter)(mSum);
	}

	virtual void Propagate() override
	{
		auto dtf = mTransferFilter->Differentiate();
		// if dtf is null intentionally crash
		for (auto i : mInputs)
		{
			i->mEPS = mEPS * (*dtf)(mSum);
			i->Propagate();
		}
		mEPS = 0.0;
	}

	virtual void AddInput(std::shared_ptr<NeuronBase::Synapsis> s) override
	{
		mInputs.push_back(s);
	}
};

class OutputNeuron : public Neuron
{
	//todo
};

//todo
/*
class Neuron(NeuralObjectInterface):
    def __init__(self):
        global ConstantOne
        self.Inputs = []
        self.Output = 0.0
        self.Sum = 0.0
        self.eps=0.0
        self.TransferFilter = TangentHyperbolic()
#        self.TransferFilter = LinearFilter()
#        self.TransferFilter = LinearSlabFilter
        Synapsis(ConstantOne, self, 0.0)

    def Activate(self):
        _sum = 0.0
        for i in self.Inputs:
            i.Activate()
            _sum += i.Weight*i.Current
        self.Sum=_sum
        self.Output=self.TransferFilter(self.Sum)

    def Propagate(self):
        dtf=self.TransferFilter.Differentiate()
        for i in self.Inputs:
            i.eps=self.eps*dtf(self.Sum)
            i.Propagate()
        self.eps=0.0


class InputNeuron(Neuron):
    def __init__(self):
        self.Output=0.0
        self.Inputs=[0.0]
        self.eps=0.0
        self.GetInput=ListValueHolder(self.Inputs)

    def Activate(self):
        self.Inputs[0]=self.GetInput()
        self.Output=self.Inputs[0]

    def Propagate(self):
        self.eps=0.0


class HiddenNeuron(Neuron):
    pass


class OutputNeuron(Neuron):
    pass


class ConstantNeuron(NeuralObjectInterface):
    def __init__(self):
        self.Output=1.0
        self.eps=0.0

    def Activate(self):
        pass

    def Propagate(self):
        self.eps=0.0


class Synapsis(NeuralObjectInterface):
    def __init__(self, begin: Neuron = None, end: Neuron = None, weight: float = 1.0):
        self.From = begin
        self.To = end
        self.Weight = weight
        self.eps=0.0
        self.Current = self.From.Output
        self.To.Inputs.append(self)

    def Activate(self):
        self.Current = self.From.Output

    def Propagate(self):
        self.From.eps+=self.eps*self.Weight
        self.Weight+=self.Braveness*self.eps*self.Current
        self.eps=0.0


class FixedSynapsis(Synapsis):
    def Propagate(self):
        # don't have to know, what propagate function exactly does
        W=self.Weight
        Synapsis.Propagate(self)
        self.Weight=W


ConstantOne = ConstantNeuron()

print("    Neuron class imported")
*/
#endif
