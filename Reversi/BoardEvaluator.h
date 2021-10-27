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
		void test();
	private:
		ClippedReLU<layer_sizes[1], true> ReLU_layer_1;
		LinearLayer<layer_sizes[1], layer_sizes[2]> layer_2;
		ClippedReLU<layer_sizes[2], true> ReLU_layer_2;
		LinearLayer<layer_sizes[2], layer_sizes[3]> layer_3;
		ClippedReLU<layer_sizes[3], true> ReLU_layer_3;
		LinearLayer<layer_sizes[3], layer_sizes[4]> layer_output;
	};

	inline thread_local BoardEvaluator be;
}
