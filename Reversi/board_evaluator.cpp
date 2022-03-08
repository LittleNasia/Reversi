#include "board_evaluator.h"
#include <fstream>
#if use_nnue
NN::board_evaluator::board_evaluator()
{

	std::ifstream input("256x128_32_32_32_1.nnue", std::ios::binary);
	float accumulator_weights[layer_sizes[0]][layer_sizes[1]];
	float layer_1_weights[layer_sizes[1]][layer_sizes[2]];
	float layer_2_weights[layer_sizes[2]][layer_sizes[3]];
	float layer_3_weights[layer_sizes[3]][layer_sizes[4]];
	float accumulator_biases[layer_sizes[1]];
	float layer_1_biases[layer_sizes[2]];
	float layer_2_biases[layer_sizes[3]];
	float layer_3_biases[layer_sizes[4]];
	input.read((char*)accumulator_weights, sizeof(accumulator_weights));
	input.read((char*)accumulator_biases, sizeof(accumulator_biases));
	input.read((char*)layer_1_weights, sizeof(layer_1_weights));
	input.read((char*)layer_1_biases, sizeof(layer_1_biases));
	input.read((char*)layer_2_weights, sizeof(layer_2_weights));
	input.read((char*)layer_2_biases, sizeof(layer_2_biases));
	input.read((char*)layer_3_weights, sizeof(layer_3_weights));
	input.read((char*)layer_3_biases, sizeof(layer_3_biases));


	nnue_accumulator::read_weights((float*)accumulator_weights, (float*)accumulator_biases);
	layer_2.read_weights((float*)layer_1_weights, (float*)layer_1_biases);
	layer_3.read_weights((float*)layer_2_weights, (float*)layer_2_biases);
	layer_output.read_weights((float*)layer_3_weights, (float*)layer_3_biases);

}

void NN::board_evaluator::test()
{
	int16_t input[layer_sizes[0]];
	for (auto& val : input)
	{
		val = 0;
	}
	nnue_accumulator acc;
	acc.recompute_acc(input);
	//simple forward pass
	ReLU_layer_1.forward(acc.output[COLOR_WHITE]);
	std::cout << "\n\n\nfirst layer relu ";
	for (int i = 0; i < layer_sizes[1]; i++)
	{
		std::cout << (float)ReLU_layer_1.output[i] / 127 << " ";
	}
	
	layer_2.forward(ReLU_layer_1.output);
	std::cout << "\n\n\nlayer 2 ";
	for (int i = 0; i < layer_sizes[2]; i++)
	{
		std::cout << (float)layer_2.output[i] / 127 << " ";
	}

	ReLU_layer_2.forward(layer_2.output);
	std::cout << "\n\n\nlayer 2 relu ";
	for (int i = 0; i < layer_sizes[2]; i++)
	{
		std::cout << (float)ReLU_layer_2.output[i] / 127 << " ";
	}

	layer_3.forward(ReLU_layer_2.output);
	std::cout << "\n\n\nlayer 2 relu ";
	for (int i = 0; i < layer_sizes[3]; i++)
	{
		std::cout << (float)layer_3.output[i] / 127 << " ";
	}

	ReLU_layer_3.forward(layer_3.output);
	std::cout << "\n\n\nlayer 2 relu ";
	for (int i = 0; i < layer_sizes[3]; i++)
	{
		std::cout << (float)ReLU_layer_3.output[i] / 127 << " ";
	}

	layer_output.forward(ReLU_layer_3.output);
	std::cout<<"\n\n\nzeros " << (float)layer_output.output[0] / 127 << "\n\n\n\n\n\n";




	for (int i = 0; i < num_features/2; i++)
	{
		input[i]  = 1;
	}
	//simple forward pass
	acc.recompute_acc(input);
	ReLU_layer_1.forward(acc.output[COLOR_WHITE]);
	std::cout << "\n\n\nfirst layer relu ";
	for (int i = 0; i < layer_sizes[1]; i++)
	{
		std::cout << (float)ReLU_layer_1.output[i] / 127 << " ";
	}

	layer_2.forward(ReLU_layer_1.output);
	std::cout << "\n\n\nlayer 2 ";
	for (int i = 0; i < layer_sizes[2]; i++)
	{
		std::cout << (float)layer_2.output[i] / 127 << " ";
	}

	ReLU_layer_2.forward(layer_2.output);
	std::cout << "\n\n\nlayer 2 relu ";
	for (int i = 0; i < layer_sizes[2]; i++)
	{
		std::cout << (float)ReLU_layer_2.output[i] / 127 << " ";
	}

	layer_3.forward(ReLU_layer_2.output);
	std::cout << "\n\n\nlayer 2 relu ";
	for (int i = 0; i < layer_sizes[3]; i++)
	{
		std::cout << (float)layer_3.output[i] / 127 << " ";
	}

	ReLU_layer_3.forward(layer_3.output);
	std::cout << "\n\n\nlayer 2 relu ";
	for (int i = 0; i < layer_sizes[3]; i++)
	{
		std::cout << (float)ReLU_layer_3.output[i] / 127 << " ";
	}

	layer_output.forward(ReLU_layer_3.output);
	std::cout << "\n\nones " << (float)layer_output.output[0]/127 << "\n";

}

int NN::board_evaluator::evaluate(const board& b)
{
	//we get the input present in the current accumulator from the board 
	const int16_t* input = b.get_current_accumulator_output();
	//for (int i = 0; i < 32; i++)
	{
	//		std::cout << (float)input[i] / 127 << " ";
	}
	//std::cout << "\n";
	
	//simple forward pass
	ReLU_layer_1.forward(input);
	///for (int i = 0; i < 32; i++)
	{
	///	std::cout << (float)ReLU_layer_1.output[i] / 127 << " ";
	}
	layer_2.forward(ReLU_layer_1.output);
	ReLU_layer_2.forward(layer_2.output);
	///std::cout << "\n\n";
	///for (int i = 0; i < 32; i++)
	{
	///	std::cout << (float)ReLU_layer_2.output[i] / 127 << " ";
	}
	///std::cout << "\n\n\n\n";
	layer_3.forward(ReLU_layer_2.output);
	ReLU_layer_3.forward(layer_3.output);
	layer_output.forward(ReLU_layer_3.output);
	return std::tanh((float)layer_output.output[0]/input_scaling_factor) * 100;
	return 0;
}


#endif