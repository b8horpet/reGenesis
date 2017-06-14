#pragma once
#ifndef NEURALNET_BRAIN_H_INCLUDED
#define NEURALNET_BRAIN_H_INCLUDED

#include "Common.h"
#include "Neuron.h"
#include <vector>

class Brain : public INeuralObject
{
public:
	class NeuronLayer : public INeuralObject
	{
	public:
		std::vector<std::shared_ptr<NeuronBase>> mNeurons;
		virtual void Activate() override
		{
			for(auto& n : mNeurons)
			{
				n->Activate();
			}
		}
		virtual void Propagate() override
		{
			for(auto& n : mNeurons)
			{
				n->Propagate();
			}
		}
	};
	NeuronLayer mInputLayer;
	std::vector<NeuronLayer> mHiddenLayers;
	NeuronLayer mOutputLayer;
    void RegisterNeuron(std::shared_ptr<NeuronBase> n)
	{
		std::shared_ptr<InputNeuron> i=std::dynamic_pointer_cast<InputNeuron>(n);
		std::shared_ptr<OutputNeuron> o=std::dynamic_pointer_cast<OutputNeuron>(n);
        if(i)
		{
            mInputLayer.mNeurons.push_back(n);
		}
        else if(o)
		{
            mOutputLayer.mNeurons.push_back(n);
		}
        else
		{
			// should be handled differently
			// target layer should be passed as a parameter
			throw "wtf?";
		}
	}
	virtual void Activate() override
	{
		mInputLayer.Activate();
		for(auto& h : mHiddenLayers)
			h.Activate();
		mOutputLayer.Activate();
	}
	virtual void Propagate() override
	{
		mInputLayer.Propagate();
		for(auto& h : mHiddenLayers)
			h.Propagate();
		mOutputLayer.Propagate();
	}
	void Propagate(std::vector<double> expected)
	{
		if(expected.size() != mOutputLayer.mNeurons.size())
			throw "size mismatch!";
		// todo
		Propagate();
	}
};

//todo
/*
class Brain(NeuralObjectInterface):
    def __init__(self):
        self.InputLayer=NeuronLayer()
        self.OutputLayer=NeuronLayer()
        self.HiddenLayers=[]

    def RegisterNeuron(self, n):
        if type(n)==InputNeuron:
            self.InputLayer.Neurons.append(n)
        elif type(n)==OutputNeuron:
            self.OutputLayer.Neurons.append(n)
        else:
            raise Exception() #wtf?

    def FillSynapsisGraph(self, random):
        l=len(self.HiddenLayers)
        if l>0:
            for i in self.InputLayer.Neurons:
                for o in self.HiddenLayers[0].Neurons:
                    Synapsis(i,o,random.uniform(-1,1))
            for h in range(0,l-1):
                for i in self.HiddenLayers[h].Neurons:
                    for o in self.HiddenLayers[h+1].Neurons:
                        Synapsis(i,o,random.uniform(-1,1))
            for i in self.HiddenLayers[l-1].Neurons:
                for o in self.OutputLayer.Neurons:
                    Synapsis(i,o,random.uniform(-1,1))
        else:
            for i in self.InputLayer.Neurons:
                for o in self.OutputLayer.Neurons:
                    Synapsis(i,o,random.uniform(-1,1))

    def Activate(self):
        self.InputLayer.Activate()
        for i in self.HiddenLayers:
            i.Activate()
        self.OutputLayer.Activate()

    def Propagate(self, Expected):
        if len(Expected) != len(self.OutputLayer.Neurons):
            raise Exception()
        for i in range(0,len(Expected)):
            currN=self.OutputLayer.Neurons[i]
            #error=0.5*(Expected[i]-currN.Output)**2
            currN.eps=Expected[i]-currN.Output
        self.OutputLayer.Propagate()
        for i in reversed(self.HiddenLayers):
                i.Propagate()
        self.InputLayer.Propagate()


*/

#endif
