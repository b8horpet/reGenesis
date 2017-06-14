import sys,os,inspect
sys.path.append((os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe()))))+"/..")
import NeuralNet

imgs=[
[#0
'01110',
'10001',
'10001',
'10001',
'01110'
],#0
[#1
'00110',
'01010',
'00010',
'00010',
'00010'
],#1
[#2
'01110',
'10001',
'00110',
'01000',
'11111'
],#2
[#3
'01110',
'10001',
'00010',
'10001',
'01110'
],#3
[#4
'00010',
'00110',
'01010',
'11111',
'00010'
],#4
[#5
'11111',
'10000',
'11110',
'00001',
'01110'
],#5
[#6
'01110',
'10000',
'11110',
'10001',
'01110'
],#6
[#7
'11111',
'00001',
'00010',
'00100',
'01000'
],#7
[#8
'01110',
'10001',
'01110',
'10001',
'01110'
],#8
[#9
'01110',
'10001',
'01111',
'00001',
'01110'
]#9
]

import random
r=random.SystemRandom()
r.seed(0)


ny=len(imgs[0])
nx=len(imgs[0][0])
digs=len(imgs)

Currimg=[0]
def GetImgGetter(x:int,y:int):
    def GetInput():
        global Currimg
        v=NeuralNet.ListValueHolder(Currimg)
        return float(imgs[v()][y][x])
    return GetInput

ImageProcessor=NeuralNet.Brain()
for i in range(0,ny):
    for j in range(0,nx):
        ImageProcessor.InputLayer.Neurons.append(NeuralNet.InputNeuron())
        ImageProcessor.InputLayer.Neurons[i*nx+j].GetInput=GetImgGetter(j,i)
for i in range(0,digs):
    ImageProcessor.OutputLayer.Neurons.append(NeuralNet.OutputNeuron())
    ImageProcessor.OutputLayer.Neurons[i].TransferFilter=NeuralNet.TangentHyperbolic()

# legyen egy hidden layer

ImageProcessor.HiddenLayers.append(NeuralNet.NeuronLayer())
for i in range(0,ny-1):
    for j in range(0,nx-1):
        ImageProcessor.HiddenLayers[0].Neurons.append(NeuralNet.HiddenNeuron())
        ImageProcessor.HiddenLayers[0].Neurons[i*(nx-1)+j].TransferFilter=NeuralNet.TangentHyperbolic()
        for ii in range(0,2):
            for jj in range(0,2):
                NeuralNet.Synapsis(ImageProcessor.InputLayer.Neurons[(i+ii)*nx+j+jj],ImageProcessor.HiddenLayers[0].Neurons[i*(nx-1)+j],r.uniform(-0.1,0.1))
        for ii in range(0,digs):
            NeuralNet.Synapsis(ImageProcessor.HiddenLayers[0].Neurons[i*(nx-1)+j],ImageProcessor.OutputLayer.Neurons[ii],r.uniform(-0.1,0.1))

NeuralNet.NeuralObjectInterface.Braveness=0.2
for lol in range(0,50):
    for digit in range(0, digs):
        Currimg[0]=digit
        ImageProcessor.Activate()
        expected=[-1 for i in range(0,digs)]
        expected[digit]=1
        ImageProcessor.Propagate(expected)
for digit in range(0, digs):
    Currimg[0]=digit
    ImageProcessor.Activate()
    print("#%d:" % (digit))
    for i in range(0,digs):
        print("%1.3f%%" % ((ImageProcessor.OutputLayer.Neurons[i].Output+1.0)*50.0))
    print()


print("Sandbox program")
