#pragma once
#include "ClippedReLU.h"

namespace NN
{ 
	template<int in_neurons,int out_neurons>
	struct LinearLayer
	{
		int16_t output[out_neurons];
		int8_t weights[in_neurons][out_neurons];

		LinearLayer()
		{
			std::memset(weights, 4, sizeof(weights));
		}

		template<bool do_zero>
		void forward(const ClippedReLU<in_neurons, do_zero>& input)
		{
			for (int output_neuron = 0; output_neuron < out_neurons; output_neuron++)
			{
				// clipped ReLU elements have size of 8 bits, we load pack_size of them at a time
				constexpr int int8_pack_size = 128 / 8;
				int sum=0;
				for (int input_pack = 0; input_pack < in_neurons / int8_pack_size; input_pack++)
				{
					//we first load the 8 bit integers into memory
					__m128i ReLU_8_bit_integers = _mm_loadu_si128((__m128i*) (&input.output[(input_pack) * (int8_pack_size)]));

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

					


					//elementwise multiplication 
					__m128i result = _mm_mullo_epi16(first_16bit_pack, first_16bit_weight_pack);
					//store the result 
					int16_t temp[16];
					_mm_store_si128((__m128i*) temp, result);
					
					//redo the multiplication for second pack
					result = _mm_mullo_epi16(second_16bit_pack, second_16bit_weight_pack);
					//store the second part of the result
					_mm_store_si128((__m128i*) &temp[8], result);
					
					for (int i = 0; i < 16; i++)
					{
						sum += temp[i];
					}
				}
				//store the sum, ReLU it (so the result isn't negative) and divide it by the scaling factor 64
				output[output_neuron] = ((sum>0)?sum:0)>>6;
			}
		}
	};

}



