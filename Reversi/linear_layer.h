#pragma once
#include "bit_utilities.h"

template<int in_neurons, int out_neurons>
class linear_layer
{
public:
	using input_type =  float;
	using output_type = float;
	linear_layer(bool use_relu = true): apply_relu(use_relu)
	{
		for (int row = 0; row < out_neurons; row++)
		{
			for (int col = 0; col < in_neurons; col++)
			{
				weights[row][col] = 0.0001;
			}
			biases[row] = 0.0001;
		}
	}
	void print()
	{
		for (int i = 0; i < out_neurons; i++)
		{
			std::cout << output[i] << "\n";
		}
	}
	void load_weights(float* loaded_weights, float* loaded_biases)
	{
		for (int col = 0, index = 0; col < in_neurons; col++)
		{
			for (int row = 0; row < out_neurons; row++, index++)
			{
				weights[row][col] = loaded_weights[index];
			}
		}
		std::memcpy(biases, loaded_biases, sizeof(biases));


		if (in_neurons == 128)
		for (int row = 0; row < 1; row++)
		{
			for (int col = 0; col < in_neurons; col++)
			{
				//std::cout << weights[row][col] << " ";
			}
		}
	}

	output_type* forward(const input_type* input)
	{
		std::memcpy(output, biases, sizeof(output));
		
		for (int output_neuron = 0; output_neuron < out_neurons; output_neuron++)
		{
			constexpr int num_elements_in_pack = sizeof(__m128i) / sizeof(input_type);
			constexpr int num_packs = in_neurons / num_elements_in_pack;
			int curr_input_index = 0;
			for (int pack = 0; pack < num_packs; pack++, curr_input_index += num_elements_in_pack)
			{
				const __m128 input_part = _mm_load_ps(&input[curr_input_index]);
			
				const __m128 weights_part = _mm_load_ps(&weights[output_neuron][curr_input_index]);

				__m128 elementwise_mul_result = _mm_mul_ps(input_part, weights_part);

				// sum all results using two horizontal sums
				__m128 first_sum_result = _mm_hadd_ps(elementwise_mul_result, elementwise_mul_result);
				__m128 summed_vector = _mm_hadd_ps(first_sum_result, first_sum_result);

				// returns the lowest byte in a vector
				float sum = _mm_cvtss_f32(summed_vector);
				output[output_neuron] += sum;
			}
		}

		return output;
	}
private:
	alignas(128) float output[out_neurons];
	alignas(128) float weights[out_neurons][in_neurons];
	alignas(128) float biases[out_neurons];
		

	const bool apply_relu;
};
