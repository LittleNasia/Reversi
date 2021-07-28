#include "BoardEvaluator.h"
#include <fstream>

NN::BoardEvaluator::BoardEvaluator()
{
	std::ifstream input("C:/Users/Anastazja/reversi.nnue", std::ios::binary);
	float accumulator_weights[256][32];
	float layer_1_weights[32][32];
	float layer_2_weights[32][1];
	float accumulator_biases[32];
	float layer_1_biases[32];
	float later_2_biases[1];
	input.read((char*)accumulator_weights, sizeof(accumulator_weights));
	input.read((char*)accumulator_biases, sizeof(accumulator_biases));
	input.read((char*)layer_1_weights, sizeof(layer_1_weights));
	input.read((char*)layer_1_biases, sizeof(layer_1_biases));
	input.read((char*)layer_2_weights, sizeof(layer_2_weights));
	input.read((char*)later_2_biases, sizeof(later_2_biases));
	NN_accumulator::read_weights((float*)accumulator_weights, (float*)accumulator_biases);
	layer_2.read_weights((float*)layer_1_weights, (float*)layer_1_biases);
	layer_output.read_weights((float*)layer_2_weights, (float*)later_2_biases);
}

void NN::BoardEvaluator::test()
{
	int16_t input[layer_sizes[0]];
	for (auto& val : input)
	{
		val = 0;
	}
	NN_accumulator acc;
	acc.recompute_acc(input);
	//simple forward pass
	ReLU_layer_1.forward(acc.output[COLOR_WHITE]);
	std::cout << "\n\n\nfirst layer relu ";
	for (int i = 0; i < 32; i++)
	{
		std::cout << (float)ReLU_layer_1.output[i] / 127 << " ";
	}
	
	layer_2.forward(ReLU_layer_1.output);
	std::cout << "\n\n\nlayer 2 ";
	for (int i = 0; i < 32; i++)
	{
		std::cout << (float)layer_2.output[i] / 127 << " ";
	}

	ReLU_layer_2.forward(layer_2.output);
	std::cout << "\n\n\nlayer 2 relu ";
	for (int i = 0; i < 32; i++)
	{
		std::cout << (float)ReLU_layer_2.output[i] / 127 << " ";
	}

	layer_output.forward(ReLU_layer_2.output);
	std::cout<<"\n\n\nzeros " << (float)layer_output.output[0] / 127 << "\n\n\n\n\n\n";

	for (auto& val : input)
	{
		val = 1;
	}
	//simple forward pass
	acc.recompute_acc(input);
	ReLU_layer_1.forward(acc.output[COLOR_WHITE]);
	std::cout << "\n\n\nfirst layer relu ";
	for (int i = 0; i < 32; i++)
	{
		std::cout << (float)ReLU_layer_1.output[i]/127 << " ";
	}

	layer_2.forward(ReLU_layer_1.output);
	std::cout << "\n\n\nlayer 2 ";
	for (int i = 0; i < 32; i++)
	{
		std::cout << (float)layer_2.output[i] / 127 << " ";
	}

	ReLU_layer_2.forward(layer_2.output);
	std::cout << "\n\n\nlayer 2 relu ";
	for (int i = 0; i < 32; i++)
	{
		std::cout << (float)ReLU_layer_2.output[i] / 127 << " ";
	}

	layer_output.forward(ReLU_layer_2.output);
	std::cout << "\n\nones " << (float)layer_output.output[0]/127 << "\n";
}

int NN::BoardEvaluator::Evaluate(const Board& b)
{
	//we get the input present in the current accumulator from the board 
	const int16_t* input = b.get_current_accumulator_output();
	
	//simple forward pass
	ReLU_layer_1.forward(input);
	layer_2.forward(ReLU_layer_1.output);
	ReLU_layer_2.forward(layer_2.output);
	layer_output.forward(ReLU_layer_2.output);
	return layer_output.output[0];
}