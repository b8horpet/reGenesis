#include <memory>
#include <cstdio>
#include <random>

#include "NeuralNet/Brain.h"

const int nx = 5;
const int ny = 5;
const int digits = 10;

char imgs [digits][ny][nx] = {
	{//0
		{ 0,1,1,1,0 },
		{ 1,0,0,0,1 },
		{ 1,0,0,0,1 },
		{ 1,0,0,0,1 },
		{ 0,1,1,1,0 }
	},//0
	{//1
		{ 0,0,1,1,0 },
		{ 0,1,0,1,0 },
		{ 0,0,0,1,0 },
		{ 0,0,0,1,0 },
		{ 0,0,0,1,0 }
	},//1
	{//2
		{ 0,1,1,1,0 },
		{ 1,0,0,0,1 },
		{ 0,0,1,1,0 },
		{ 0,1,0,0,0 },
		{ 1,1,1,1,1 }
	},//2
	{//3
		{ 0,1,1,1,0 },
		{ 1,0,0,0,1 },
		{ 0,0,0,1,0 },
		{ 1,0,0,0,1 },
		{ 0,1,1,1,0 }
	},//3
	{//4
		{ 0,0,0,1,0 },
		{ 0,0,1,1,0 },
		{ 0,1,0,1,0 },
		{ 1,1,1,1,1 },
		{ 0,0,0,1,0 }
	},//4
	{//5
		{ 1,1,1,1,1 },
		{ 1,0,0,0,0 },
		{ 1,1,1,1,0 },
		{ 0,0,0,0,1 },
		{ 0,1,1,1,0 }
	},//5
	{//6
		{ 0,1,1,1,0 },
		{ 1,0,0,0,0 },
		{ 1,1,1,1,0 },
		{ 1,0,0,0,1 },
		{ 0,1,1,1,0 }
	},//6
	{//7
		{ 1,1,1,1,1 },
		{ 0,0,0,0,1 },
		{ 0,0,0,1,0 },
		{ 0,0,1,0,0 },
		{ 0,1,0,0,0 }
	},//7
	{//8
		{ 0,1,1,1,0 },
		{ 1,0,0,0,1 },
		{ 0,1,1,1,0 },
		{ 1,0,0,0,1 },
		{ 0,1,1,1,0 }
	},//8
	{//9
		{ 0,1,1,1,0 },
		{ 1,0,0,0,1 },
		{ 0,1,1,1,1 },
		{ 0,0,0,0,1 },
		{ 0,1,1,1,0 }
	}//9
};

int current = 0;

std::function<double()> GetImgGetter(int x, int y)
{
	return [x, y]()
	{
		return double(imgs[current][y][x]);
	};
}


int main()
{
	auto ImageProcessor = std::make_unique<Brain>();
	std::mt19937 r;
	std::uniform_real_distribution<double> d(-0.1,0.1);

	for(int i=0; i<ny; ++i)
	{
		for(int j=0; j<nx; ++j)
		{
			ImageProcessor->mInputLayer.mNeurons.push_back(std::shared_ptr<NeuronBase>(new InputNeuron(GetImgGetter(j,i))));
		}
	}
	std::shared_ptr<IFunctionObject> tanh(new TangentHyperbolicFunc);
	for(int i=0; i<digits; ++i)
	{
		OutputNeuron* n = new OutputNeuron;
		n->mTransferFilter = tanh;
		ImageProcessor->mOutputLayer.mNeurons.push_back(std::shared_ptr<NeuronBase>(n));
	}

	ImageProcessor->mHiddenLayers.push_back(Brain::NeuronLayer());
	for (int i = 0; i < ny-1; ++i)
	{
		for (int j = 0; j < nx-1; ++j)
		{
			Neuron* n = new Neuron;
			n->mTransferFilter = tanh;
			ImageProcessor->mHiddenLayers[0].mNeurons.push_back(std::shared_ptr<NeuronBase>(n));
			for (int ii = 0; ii < 2; ++ii)
			{
				for (int jj = 0; jj < 2; ++jj)
				{
					new NeuronBase::Synapsis(ImageProcessor->mInputLayer.mNeurons[(i + ii)*nx + j + jj], ImageProcessor->mHiddenLayers[0].mNeurons[i*(nx - 1) + j],d(r));
				}
			}
			for (int ii = 0; ii < digits; ++ii)
			{
				new NeuronBase::Synapsis(ImageProcessor->mHiddenLayers[0].mNeurons[i*(nx - 1) + j], ImageProcessor->mOutputLayer.mNeurons[ii],d(r));
			}
		}
	}

	INeuralObject::Braveness = 0.2;
	for (int lol = 0; lol < 1000; ++lol)
	{
		for (int digit = 0; digit < digits; ++digit)
		{
			current = digit;
			ImageProcessor->Activate();
			std::vector<double> expected(digits);
			for (int d = 0; d < digits; ++d)
				expected[d] = (d == digit) ? 1.0 : -1.0;
			ImageProcessor->Propagate(expected);
		}
	}

	for (int digit = 0; digit < digits; ++digit)
	{
		current = digit;
		printf("#%d\n",digit);
		ImageProcessor->Activate();
		for (int i = 0; i < digits; ++i)
		{
			auto o = std::static_pointer_cast<OutputNeuron>(ImageProcessor->mOutputLayer.mNeurons[i]);
			printf("%1.3f%%\n",(o->mOutput+1.0)*50.0);
		}
	}

	return 0;
}
/*

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
*/
