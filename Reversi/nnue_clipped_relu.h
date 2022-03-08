#pragma once
#include "nnue_accumulator.h"
#include <smmintrin.h>
#include <emmintrin.h>


namespace NN
{ 
	template <int size, bool do_zero_max>
	struct clipped_relu
	{
		int8_t output[size];

		//a function that takes 16 bit input and sets the output to the clipped ReLU of that output (clamp(value,0,127)) 
		void forward(const int16_t* acc)
		{


			//128 bits of zeroes, used in ReLU lower range 
			const __m128i zero_vector = _mm_setzero_si128();

			//128 bits, each packed value is 8 bits in size 
			constexpr int int8_pack_size = 128 / 8;
			//16 bit values are twice as large, so we can fit only half of them  
			constexpr int int16_pack_size = int8_pack_size / 2;

			for (int pack = 0; pack < size / int8_pack_size; pack++)
			{
				//previous layer data has size "size", there are size/8 packs, we load two of them into registers 
				//we multiply pack variable by two, because the input is 16 bit, so we have to do twice as many loads of twice as small number of integers 
				__m128i accumulator_data_first = _mm_loadu_si128(( __m128i*) (&acc[(pack*2) * (int16_pack_size)]));
				const __m128i accumulator_data_second = _mm_loadu_si128(( __m128i*) (&acc[((pack * 2) + 1) * (int16_pack_size)]));
				//we convert and move the 16 bit values to 8 bit 
				//it does saturate the 8 bit integers (effectively clipping them to 127 -> we want that), however we also want to apply ReLU on them later 
				accumulator_data_first  = _mm_packs_epi16(accumulator_data_first, accumulator_data_second);

				if (do_zero_max)
				{
					//apply the ReLU function val = max(val,0) on each of the 8 bit integers 
					accumulator_data_first = _mm_max_epi8(accumulator_data_first, zero_vector);
				}

				//store the result in our ReLU layer 
				_mm_store_si128((__m128i*) &output[pack * (int8_pack_size)], accumulator_data_first);
			}
		}
		
		
	};
}