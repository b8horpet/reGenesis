#author: b8horpet


import unittest

import sys,os,inspect
sys.path.append((os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe()))))+"/..")
from NeuralNet import *


class TestNeuralNet(unittest.TestCase):
    def test_PureVirtual(self):
        """
        The base interface of all neural object should lack implementatiom.
        It is an abstract class. Cannot be initiated.
        """
        with self.assertRaises(NotImplementedError):
            n=NeuralObjectInterface()

    def test_SignalTransferSpeed(self):
        """
        The synapses should not delay the signal transfer.
        The input should go through the whole network in one tick.
        """
        b=Brain()
        i=InputNeuron()
        i.TransferFilter=LinearFilter()
        b.InputLayer.Neurons.append(i)
        b.HiddenLayers.append(NeuronLayer())
        h=HiddenNeuron()
        h.TransferFilter=LinearFilter()
        b.HiddenLayers[0].Neurons.append(h)
        b.HiddenLayers.append(NeuronLayer())
        h2=HiddenNeuron()
        h2.TransferFilter=LinearFilter()
        b.HiddenLayers[1].Neurons.append(h2)
        o=OutputNeuron()
        o.TransferFilter=LinearFilter()
        b.OutputLayer.Neurons.append(o)
        Synapsis(b.InputLayer.Neurons[0],b.HiddenLayers[0].Neurons[0])
        Synapsis(b.HiddenLayers[0].Neurons[0],b.HiddenLayers[1].Neurons[0])
        Synapsis(b.HiddenLayers[1].Neurons[0],b.OutputLayer.Neurons[0])
        x=3.14159265358979
        b.InputLayer.Neurons[0].GetInput=ConstValueHolder(x)
        b.Activate()
        self.assertEqual(b.OutputLayer.Neurons[0].Output,x)

    def test_Functions(self):
        fxsqrd=Polinomial(2)
        self.assertTrue(fxsqrd.Differentiate() != None)
        fxmult2=Polinomial(1,2)
        self.assertTrue(fxsqrd.Differentiate() == fxmult2)
        class duckpoly:
            def __init__(self):
                self.n=1
                self.a=2
        duck=duckpoly()
        self.assertFalse(fxmult2 == duck)
        linear=LinearFilter()
        self.assertFalse(fxmult2 == linear)

    def test_Propagation(self):
        """
        Propagation should alter the weights of synapses in a way
        that modifies the output of the system towards the desired value
        """
        b=Brain()
        i=InputNeuron()
        i.TransferFilter=LinearFilter()
        b.InputLayer.Neurons.append(i)
        b.HiddenLayers.append(NeuronLayer())
        h=HiddenNeuron()
        h.TransferFilter=LinearFilter()
        b.HiddenLayers[0].Neurons.append(h)
        b.HiddenLayers.append(NeuronLayer())
        h2=HiddenNeuron()
        h2.TransferFilter=LinearFilter()
        b.HiddenLayers[1].Neurons.append(h2)
        o=OutputNeuron()
        o.TransferFilter=LinearFilter()
        b.OutputLayer.Neurons.append(o)
        Synapsis(b.InputLayer.Neurons[0],b.HiddenLayers[0].Neurons[0])
        Synapsis(b.HiddenLayers[0].Neurons[0],b.HiddenLayers[1].Neurons[0])
        Synapsis(b.HiddenLayers[1].Neurons[0],b.OutputLayer.Neurons[0])
        x=5.0
        NeuralObjectInterface.Braveness=0.02
        b.InputLayer.Neurons[0].GetInput=ConstValueHolder(x)
        prewWeight=[
            b.HiddenLayers[0].Neurons[0].Inputs[1].Weight,
            b.HiddenLayers[1].Neurons[0].Inputs[1].Weight,
            b.OutputLayer.Neurons[0].Inputs[1].Weight
        ]
        b.Activate()
        self.assertEqual(b.OutputLayer.Neurons[0].Output,x)
        y=[1.0]
        b.Propagate(y)
        curWeight=[
            b.HiddenLayers[0].Neurons[0].Inputs[1].Weight,
            b.HiddenLayers[1].Neurons[0].Inputs[1].Weight,
            b.OutputLayer.Neurons[0].Inputs[1].Weight
        ]
        for i in range(0,len(prewWeight)):
            self.assertLess(curWeight[i],prewWeight[i])
        b.Activate()
        self.assertLess(b.OutputLayer.Neurons[0].Output,x)




if __name__ == '__main__':
    unittest.main()

