#pragma once
#include "LinearLayer.h"
#include "ClippedReLU.h"
#include "NN_accumulator.h"
#include "Board.h"

#include <string>

namespace NN
{ 
	class BoardEvaluator
	{
	public:
		//reads weights from file 
		BoardEvaluator(std::string& weights_filename);
		//random weights initialization
		BoardEvaluator();

		int Evaluate(const Board& b);
	private:
		ClippedReLU<32,true> ReLU_layer_1;
		LinearLayer<32, 32> layer_2;
		ClippedReLU<32, true> ReLU_layer_2;
		LinearLayer<32, 1> layer_output;
	};

	inline thread_local BoardEvaluator be;
}
