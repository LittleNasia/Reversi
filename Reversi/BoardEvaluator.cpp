#include "BoardEvaluator.h"


NN::BoardEvaluator::BoardEvaluator()
{

}

int NN::BoardEvaluator::Evaluate(const Board& b)
{
	//we get the input present in the current accumulator from the board 
	const int16_t* layer_1 = b.get_current_accumulator();

	//simple forward pass
	ReLU_layer_1.forward(layer_1);
	layer_2.forward(ReLU_layer_1);
	ReLU_layer_2.forward(layer_2.output);
	layer_output.forward(ReLU_layer_2);
	return layer_output.output[0];
}