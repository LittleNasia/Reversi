#pragma once
#include "board.h"
#include "conv_layer.h"
#include "cnn_input_layer.h"
#include "linear_layer.h"

#include <fstream>
namespace CNN
{
	constexpr int num_layers = 5;
	constexpr int num_conv_layers = 3;
	constexpr int num_dense_layers = 2;

	constexpr int filter_rows = 4;
	constexpr int filter_cols = 4;

	constexpr int conv_layer_channels[num_conv_layers] = { 64,64,64 };

	enum layer_names
	{
		FIRST_CONV_LAYER,
		SECOND_CONV_LAYER,
		THIRD_CONV_LAYER,

		FIRST_DENSE_LAYER = 0,
		SECOND_DENSE_LAYER,
		OUTPUT_LAYER = SECOND_DENSE_LAYER
	};


	struct conv_layer_info
	{
		int input_rows;
		int input_cols;
		int input_channels;

		int filter_rows;
		int filter_cols;

		int output_rows;
		int output_cols;
		int output_channels;

		int left_col_padding;
		int right_col_padding;
		int top_row_padding;
		int bottom_row_padding;

		bool is_flattened;
	};

	constexpr conv_layer_info conv_layers_info[num_conv_layers] =
	{
		//first_layer
		{
			//input rows
			board::rows,
			//input cols
			board::cols,
			//input_channels
			input_channels,

			//filter rows
			filter_rows,
			//filter cols
			filter_cols,

			//output rows
			board::rows - (filter_rows - 1),
			//output cols
			board::cols - (filter_cols - 1),
			//output_channels
			conv_layer_channels[FIRST_CONV_LAYER],

			//left col padding
			1,
			//right col padding
			2,
			//top row padding
			1,
			//bottom row padding
			2,
			//flattened
			false
		},

		//second_layer
		{
			//input rows
			board::rows,
			//input cols
			board::cols,
			//input_channels
			conv_layer_channels[FIRST_CONV_LAYER],

			//filter rows
			filter_rows,
			//filter cols
			filter_cols,

			//output rows
			board::rows - (filter_rows - 1),
			//output cols
			board::cols - (filter_cols - 1),
			//output_channels
			conv_layer_channels[SECOND_CONV_LAYER],

			//left col padding
			1,
			//right col padding
			2,
			//top row padding
			1,
			//bottom row padding
			2,
			//flattened
			false
		},

		//third_layer
		{
			//input rows
			board::rows,
			//input cols
			board::cols,
			//input_channels
			conv_layer_channels[SECOND_CONV_LAYER],

			//filter rows
			filter_rows,
			//filter cols
			filter_cols,

			//output rows
			board::rows - (filter_rows - 1),
			//output cols
			board::cols - (filter_cols - 1),
			//output_channels
			conv_layer_channels[THIRD_CONV_LAYER],

			//left col padding
			1,
			//right col padding
			2,
			//top row padding
			1,
			//bottom row padding
			2,
			//flattened
			true
		}
	};

	constexpr int dense_layer_sizes[num_dense_layers] = { 128,1 };

	class cnn
	{
	public:
		cnn() :output_layer(false) 
		{
			std::ifstream weights_file("64c_64c_64c_128_1.cnn", std::ios::binary);

			float first_conv_layer_weights[conv_layers_info[FIRST_CONV_LAYER].filter_rows][conv_layers_info[FIRST_CONV_LAYER].filter_cols]
				[conv_layers_info[FIRST_CONV_LAYER].input_channels][conv_layers_info[FIRST_CONV_LAYER].output_channels];
			float first_conv_layer_biases[conv_layers_info[FIRST_CONV_LAYER].output_channels];

			float second_conv_layer_weights[conv_layers_info[SECOND_CONV_LAYER].filter_rows][conv_layers_info[SECOND_CONV_LAYER].filter_cols]
				[conv_layers_info[SECOND_CONV_LAYER].input_channels][conv_layers_info[SECOND_CONV_LAYER].output_channels];
			float second_conv_layer_biases[conv_layers_info[SECOND_CONV_LAYER].output_channels];

			float third_conv_layer_weights[conv_layers_info[THIRD_CONV_LAYER].filter_rows][conv_layers_info[THIRD_CONV_LAYER].filter_cols]
				[conv_layers_info[THIRD_CONV_LAYER].input_channels][conv_layers_info[THIRD_CONV_LAYER].output_channels];
			float third_conv_layer_biases[conv_layers_info[THIRD_CONV_LAYER].output_channels];


			weights_file.read((char*)first_conv_layer_weights, sizeof(first_conv_layer_weights));
			weights_file.read((char*)first_conv_layer_biases, sizeof(first_conv_layer_biases));
			first_conv_layer.load_weights((float*)first_conv_layer_weights, first_conv_layer_biases);


			weights_file.read((char*)second_conv_layer_weights, sizeof(second_conv_layer_weights));
			weights_file.read((char*)second_conv_layer_biases, sizeof(second_conv_layer_biases));
			second_conv_layer.load_weights((float*)second_conv_layer_weights, second_conv_layer_biases);


			weights_file.read((char*)third_conv_layer_weights, sizeof(third_conv_layer_weights));
			weights_file.read((char*)third_conv_layer_biases, sizeof(third_conv_layer_biases));
			third_conv_layer.load_weights((float*)third_conv_layer_weights, third_conv_layer_biases);


			float first_dense_layer_weights[third_conv_layer_type::flattened_output_size][dense_layer_sizes[FIRST_DENSE_LAYER]];
			float first_dense_layer_biases[dense_layer_sizes[FIRST_DENSE_LAYER]];

			float second_dense_layer_weights[dense_layer_sizes[FIRST_DENSE_LAYER]][dense_layer_sizes[SECOND_DENSE_LAYER]];
			float second_dense_layer_biases[dense_layer_sizes[SECOND_DENSE_LAYER]];

			weights_file.read((char*)first_dense_layer_weights, sizeof(first_dense_layer_weights));
			weights_file.read((char*)first_dense_layer_biases, sizeof(first_dense_layer_biases));
			fully_connected_layer.load_weights((float*)first_dense_layer_weights, first_dense_layer_biases);


			weights_file.read((char*)second_dense_layer_weights, sizeof(second_dense_layer_weights));
			weights_file.read((char*)second_dense_layer_biases, sizeof(second_dense_layer_biases));
			output_layer.load_weights((float*)second_dense_layer_weights, second_dense_layer_biases);


			weights_file.close();
		}
		using first_conv_layer_type = conv_layer<
			conv_layers_info[FIRST_CONV_LAYER].input_rows,
			conv_layers_info[FIRST_CONV_LAYER].input_cols,
			conv_layers_info[FIRST_CONV_LAYER].input_channels,
			conv_layers_info[FIRST_CONV_LAYER].filter_rows,
			conv_layers_info[FIRST_CONV_LAYER].filter_cols,
			conv_layers_info[FIRST_CONV_LAYER].output_rows,
			conv_layers_info[FIRST_CONV_LAYER].output_cols,
			conv_layers_info[FIRST_CONV_LAYER].output_channels,
			conv_layers_info[FIRST_CONV_LAYER].left_col_padding,
			conv_layers_info[FIRST_CONV_LAYER].right_col_padding,
			conv_layers_info[FIRST_CONV_LAYER].top_row_padding,
			conv_layers_info[FIRST_CONV_LAYER].bottom_row_padding,
			conv_layers_info[FIRST_CONV_LAYER].is_flattened
		>;

		using second_conv_layer_type = conv_layer<
			conv_layers_info[SECOND_CONV_LAYER].input_rows,
			conv_layers_info[SECOND_CONV_LAYER].input_cols,
			conv_layers_info[SECOND_CONV_LAYER].input_channels,
			conv_layers_info[SECOND_CONV_LAYER].filter_rows,
			conv_layers_info[SECOND_CONV_LAYER].filter_cols,
			conv_layers_info[SECOND_CONV_LAYER].output_rows,
			conv_layers_info[SECOND_CONV_LAYER].output_cols,
			conv_layers_info[SECOND_CONV_LAYER].output_channels,
			conv_layers_info[SECOND_CONV_LAYER].left_col_padding,
			conv_layers_info[SECOND_CONV_LAYER].right_col_padding,
			conv_layers_info[SECOND_CONV_LAYER].top_row_padding,
			conv_layers_info[SECOND_CONV_LAYER].bottom_row_padding,
			conv_layers_info[SECOND_CONV_LAYER].is_flattened
		>;

		using third_conv_layer_type = conv_layer<
			conv_layers_info[THIRD_CONV_LAYER].input_rows,
			conv_layers_info[THIRD_CONV_LAYER].input_cols,
			conv_layers_info[THIRD_CONV_LAYER].input_channels,
			conv_layers_info[THIRD_CONV_LAYER].filter_rows,
			conv_layers_info[THIRD_CONV_LAYER].filter_cols,
			conv_layers_info[THIRD_CONV_LAYER].output_rows,
			conv_layers_info[THIRD_CONV_LAYER].output_cols,
			conv_layers_info[THIRD_CONV_LAYER].output_channels,
			conv_layers_info[THIRD_CONV_LAYER].left_col_padding,
			conv_layers_info[THIRD_CONV_LAYER].right_col_padding,
			conv_layers_info[THIRD_CONV_LAYER].top_row_padding,
			conv_layers_info[THIRD_CONV_LAYER].bottom_row_padding,
			conv_layers_info[THIRD_CONV_LAYER].is_flattened
		>;

		using first_dense_layer_type = linear_layer<third_conv_layer_type::flattened_output_size, dense_layer_sizes[FIRST_DENSE_LAYER]>;
		using second_dense_layer_type = linear_layer<dense_layer_sizes[FIRST_DENSE_LAYER], dense_layer_sizes[SECOND_DENSE_LAYER]>;

		float evaluate(const board& b)
		{
			const auto& conv_input = input.prepare_output(b);
			//simple forward pass

			const auto& first_conv_layer_output = first_conv_layer.forward(conv_input);
			const auto& second_conv_layer_output = second_conv_layer.forward(first_conv_layer_output);
			const auto& third_conv_layer_output = third_conv_layer.forward(second_conv_layer_output);
			const auto& third_conv_layer_flattened_output = third_conv_layer.flatten();

			const auto& fully_connected_layer_output = fully_connected_layer.forward(third_conv_layer_flattened_output);
			const auto& final_layer_output = output_layer.forward(fully_connected_layer_output);
			return std::tanh(*final_layer_output);
		}

		

	private:
		cnn_input_layer input;

		first_conv_layer_type first_conv_layer;
		second_conv_layer_type second_conv_layer;
		third_conv_layer_type third_conv_layer;

		first_dense_layer_type fully_connected_layer;
		second_dense_layer_type output_layer;
	};
}