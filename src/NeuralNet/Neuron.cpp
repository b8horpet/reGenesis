#include "Neuron.h"

std::shared_ptr<NeuronBase> ConstantNeuron::gConstantNeuron
	= std::make_shared<ConstantNeuron>();
