#pragma once
#include "board.h"
#include "linear_layer.h"                                                                                                         
#include "mlp_input_layer.h"


namespace MLP
{
	// doesn't include the input, includes the output
	inline constexpr int num_layers = 3;
	inline constexpr int layer_sizes[num_layers] =
	{
		128, 128, 1
	};
	class mlp
	{
	public:
		mlp(const std::string& filename);
		float forward(const board& b);
	private:
		mlp_input_layer<layer_sizes[0]>					first_layer;
		linear_layer<layer_sizes[0], layer_sizes[1]>	second_layer;
		linear_layer<layer_sizes[1], layer_sizes[2]>	output_layer;
	};

	inline thread_local mlp mlp_evaluator("32_32_1.mlp");
}


