#pragma once
#include "ClippedReLU.h"
#include <algorithm>

namespace NN
{ 
	template<int in_neurons,int out_neurons>
	struct LinearLayer
	{
		int16_t output[out_neurons];
		int8_t weights[in_neurons][out_neurons];
		int16_t biases[out_neurons];

		LinearLayer()
		{
			for (int i = 0; i < in_neurons; i++)
			{
				for (int j = 0; j < out_neurons; j++)
				{
					weights[i][j] = rng::rng();
				}
			}
		}

		void read_weights(float* weights_from_file, float* biases_from_file)
		{
			for (int col=0, index = 0 ; col < out_neurons; col++)
			{
				for (int row = 0; row < in_neurons; row++, index++)
				{
					//unlike in the accumulator, no funny rules apply here, the input incoming to the layer is scalled correctly
					//this means the calculation can be done perfectly fine using normal int8 weights and their scaling factors
					int weight_int = (int)(weights_from_file[index] * weight_scaling_factor);
					weight_int = std::clamp(weight_int, -128, 127);
					weights[row][col] = (int8_t)weight_int;
				}
			}
			for (int neuron = 0; neuron < out_neurons; neuron++)
			{
				//bias is as if you connected a node with value 1 float and used bias as a weight
				//1 float is equal to 1 * input_scaling_factor int value, meanwhile the bias value is scaled by the weight scaling factor
				//which means that effectively the calculation of bias node and its weight is as follows:
				//input_value(input (1) * input_scaling_factor) * weight_value(bias_float * weight_scaling_factor)
				//at the end, this simplifies to input_scaling_factor * bias_float * weight_scaling_factor
				//this funny business was not done in the accumulator, as the input to the acucmulator is not scaled by the input scaling factor
				int bias_int = (int)(biases_from_file[neuron] * weight_scaling_factor * input_scaling_factor);
				biases[neuron] = (int16_t)bias_int;
			}
		}

		void forward(const int8_t* input)
		{
			for (int output_neuron = 0; output_neuron < out_neurons; output_neuron++)
			{
				// clipped ReLU elements have size of 8 bits, we load pack_size of them at a time
				constexpr int int8_pack_size = 128 / 8;
				int sum=0;
				for (int input_pack = 0; input_pack < in_neurons / int8_pack_size; input_pack++)
				{
					//we first load the 8 bit integers into memory, it's the inputs
					__m128i ReLU_8_bit_integers = _mm_loadu_si128((__m128i*) (&input[(input_pack) * (int8_pack_size)]));

					//we convert them to 16 bit integers, so we can do elementwise multiplication
					//this instruction converts and moves only the lowest 8 bytes, so we will need to redo that again
					__m128i first_16bit_pack = _mm_cvtepi8_epi16(ReLU_8_bit_integers);
					//shift the register by 8 bytes to the right, just so that the highest 8 bytes will now be the lowest ones
					ReLU_8_bit_integers = _mm_bsrli_si128(ReLU_8_bit_integers, 8);
					//and do the conversion again
					__m128i second_16bit_pack = _mm_cvtepi8_epi16(ReLU_8_bit_integers);
				

					
					//load the weights for the packs
					__m128i weights_8_bit = _mm_loadu_si128((__m128i*) (&weights[output_neuron][(input_pack) * (int8_pack_size)]));
					//redo the operations from earlier
					__m128i first_16bit_weight_pack = _mm_cvtepi8_epi16(weights_8_bit);
					weights_8_bit = _mm_bsrli_si128(weights_8_bit, 8);
					__m128i second_16bit_weight_pack = _mm_cvtepi8_epi16(weights_8_bit);

					


					//elementwise multiplication of inputs and their respective weights
					__m128i result = _mm_mullo_epi16(first_16bit_pack, first_16bit_weight_pack);
					//store the result 
					int16_t temp[16];
					_mm_store_si128((__m128i*) temp, result);
					
					//redo the multiplication for second pack
					result = _mm_mullo_epi16(second_16bit_pack, second_16bit_weight_pack);
					//store the second part of the result
					_mm_store_si128((__m128i*) &temp[8], result);
					
					//sum all values, as the output of a node is sum(inputs * weights) 
					for (int i = 0; i < 16; i++)
					{
						sum += temp[i];
					}
				}
				//store the sum and divide it by the scaling factor 64
				output[output_neuron] = (sum + biases[output_neuron]) / weight_scaling_factor;
			}
		}
	};

}



