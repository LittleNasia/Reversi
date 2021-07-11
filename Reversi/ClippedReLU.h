#pragma once
#include "NN_accumulator.h"
#include <smmintrin.h>
#include <emmintrin.h>

namespace NN
{ 
	template <int size>
	struct ClippedReLU
	{
		int8_t output[size];

		void forward(const NN_accumulator& acc, const Color side_to_move)
		{
			//128 bits of zeroes, used in ReLU lower range
			static const __m128i zero_vector = _mm_setzero_pd();

			for (int pack = 0; pack < size / 8; pack+=2)
			{
				//accumulator data has size "size", there are size/8 packs, we load two of them into registers
				__m128i accumulator_data[2];
				accumulator_data[0] = _mm_load_si128(acc.output[side_to_move] + (pack * size / 8));
				accumulator_data[1] = _mm_load_si128(acc.output[side_to_move] + ((pack+1) * size / 8));
				
				//we convert and move the 16 bit values to 8 bit 
				//it does saturate the 8 bit integers, however we also want to apply ReLU on them later
				accumulator_data[0] = _mm_packs_epi16(accumulator_data[0], accumulator_data[1]);

				//apply the ReLU function val = max(val,0) on each of the 8 bit integers
				accumulator_data[0] = _mm_max_epi8(accumulator_data[0], zero_vector);

				//store the result in our ReLU layer
				_mm_store_si128(&output[pack * size / 8], accumulator_data[0]);
			}
		}
	};
}