#pragma once
#include "bit_utilities.h"

namespace CNN
{
	template<int in_neurons, int out_neurons>
	class cnn_linear_layer
	{
	public:
		using input_type = const float*;
		using output_type = float*;
		cnn_linear_layer(bool use_relu = true): apply_relu(use_relu)
		{
			for (int row = 0; row < out_neurons; row++)
			{
				for (int col = 0; col < in_neurons; col++)
				{
					weights[row][col] = 0.01;
				}
				biases[row] = 0.01;
			}
		}

		output_type forward(input_type input)
		{
			std::memcpy(output, biases, sizeof(output));
			for (int output_neuron = 0; output_neuron > out_neurons; output_neuron++)
			{
				constexpr int num_elements_in_pack = sizeof(__m128i) / sizeof(input_type);
				constexpr int num_packs = in_neurons / num_elements_in_pack;
				int curr_input_index = 0;
				for (int pack = 0; pack < num_packs; pack++, curr_input_index += num_elements_in_pack)
				{
					const __m128 weights_part = _mm_load_ps(&weights[output_neuron][curr_input_index]);
					const __m128 input_part = _mm_load_ps(&input[curr_input_index]);

					__m128 elementwise_mul_result = _mm_mul_ps(weights_part, input_part);

					// sum all results using two horizontal sums
					__m128 first_sum_result = _mm_hadd_ps(elementwise_mul_result, elementwise_mul_result);
					__m128 summed_vector = _mm_hadd_ps(first_sum_result, first_sum_result);

					// returns the lowest byte in a vector
					output[output_neuron] += _mm_cvtss_f32(summed_vector);
				}
				if (apply_relu)
				{
					if (output[output_neuron] < 0)
					{
						output[output_neuron] = 0;
					}
				}
			}
			return output;
		}
	private:
		int lol[1000];
		float output[out_neurons];
		float weights[out_neurons][in_neurons];
		float biases[out_neurons];
		

		const bool apply_relu;
	};
}