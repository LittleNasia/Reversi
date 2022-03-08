#include "mlp.h"

#include <fstream>
											//do not apply relu to linear layers
											//this is because we're using tanh layers instead
MLP::mlp::mlp(const std::string& filename) :second_layer(false), output_layer(false)
{
	std::ifstream weights_file(filename, std::ios::binary);
	if (!weights_file.good())
	{
		std::cout << "loasdd" << "\n";
	}

	float first_layer_weights[layer_sizes[0]][input_features];
	float first_layer_biases[layer_sizes[0]];

	float second_layer_weights[layer_sizes[1]][layer_sizes[0]];
	float second_layer_biases[layer_sizes[1]];

	float output_layer_weights[layer_sizes[2]][layer_sizes[1]];
	float output_layer_biases[layer_sizes[2]];


	weights_file.read((char*)first_layer_weights, sizeof(first_layer_weights));
	weights_file.read((char*)first_layer_biases, sizeof(first_layer_biases));
	first_layer.load_weights((float*)first_layer_weights, first_layer_biases);

	weights_file.read((char*)second_layer_weights, sizeof(second_layer_weights));
	weights_file.read((char*)second_layer_biases, sizeof(second_layer_biases));
	second_layer.load_weights((float*)second_layer_weights, second_layer_biases);

	weights_file.read((char*)output_layer_weights, sizeof(output_layer_weights));
	weights_file.read((char*)output_layer_biases, sizeof(output_layer_biases));
	output_layer.load_weights((float*)output_layer_weights, output_layer_biases);

	weights_file.close();
}

template<int layer_size>
void apply_activation_function(float* output)
{
	static constexpr int floats_per_pack = sizeof(__m128) / sizeof(float);
	static constexpr int num_packs = layer_size / floats_per_pack;
	for (int curr_pack_index = 0, curr_index = 0; curr_pack_index < num_packs; curr_pack_index++, curr_index += floats_per_pack)
	{
		// load the values from the output of the layer
		__m128 curr_pack = _mm_load_ps(&output[curr_index]);
		// apply tanh to the values
		curr_pack = _mm_tanh_ps(curr_pack);
		// store the values back to input after tanh operation
		_mm_store_ps(&output[curr_index], curr_pack);
	}
}

float MLP::mlp::forward(const board& b)
{
	auto first_layer_output = first_layer.forward(b);
	//for (int i = 0; i < 32; i++)
	//{
	//	std::cout << std::tanh(first_layer_output[i]) << "   ";
	//}
	//std::cout << "\n";
	apply_activation_function<layer_sizes[0]>(first_layer_output);
	//for (int i = 0; i < 32; i++)
	//{
	//	std::cout << first_layer_output[i] << "    ";
	//}
	//std::cout << "\n\n\n";
	auto second_layer_output = second_layer.forward(first_layer_output);
	apply_activation_function<layer_sizes[1]>(second_layer_output);

	auto output = output_layer.forward(second_layer_output);
	// output layer has size of 1, and so the pointer to the output "array" 
	// is just a pointer to the only output node, and so we can just dereference it
	float value = std::tanh(*output) * 1000;
	//std::cout << value << '\n';

	 
	return value;
}