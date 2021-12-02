#pragma once
#include "linear_layer.h"
#include "clipped_relu.h"
#include "nnue_accumulator.h"
#include "board.h"

#include <string>

namespace NN
{ 
	class board_evaluator
	{
	public:
		//reads weights from file 
		board_evaluator(std::string& weights_filename);
		//random weights initialization
		board_evaluator();

		int evaluate(const board& b);
		void test();
	private:
		clipped_relu<layer_sizes[1], true> ReLU_layer_1;
		linear_layer<layer_sizes[1], layer_sizes[2]> layer_2;
		clipped_relu<layer_sizes[2], true> ReLU_layer_2;
		linear_layer<layer_sizes[2], layer_sizes[3]> layer_3;
		clipped_relu<layer_sizes[3], true> ReLU_layer_3;
		linear_layer<layer_sizes[3], layer_sizes[4]> layer_output;
	};

	inline thread_local board_evaluator be;
}
