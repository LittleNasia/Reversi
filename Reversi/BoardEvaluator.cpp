#include "BoardEvaluator.h"


NN::BoardEvaluator::BoardEvaluator()
{

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