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
			__m128i zero_vector = _mm_setzero_si128();

			//128 bits, each packed value is 8 bits in size
			constexpr int int8_pack_size = 128 / 8;
			//16 bit values are twice as large, so we can fit only half of them 
			constexpr int int16_pack_size = int8_pack_size / 2;

			for (int pack = 0; pack < size / int8_pack_size; pack++)
			{
				//accumulator data has size "size", there are size/8 packs, we load two of them into registers
				//we multiply  pack by two, because 
				__m128i accumulator_data_first = _mm_loadu_si128(( __m128i*) (&acc.output[side_to_move][(pack*2) * (int16_pack_size)]));
				__m128i accumulator_data_second = _mm_loadu_si128(( __m128i*) (&acc.output[side_to_move][((pack * 2) + 1) * (int16_pack_size)]));
				//we convert and move the 16 bit values to 8 bit 
				//it does saturate the 8 bit integers, however we also want to apply ReLU on them later
				accumulator_data_first  = _mm_packs_epi16(accumulator_data_first, accumulator_data_second);

				//apply the ReLU function val = max(val,0) on each of the 8 bit integers
				accumulator_data_first = _mm_max_epi8(accumulator_data_first, zero_vector);

				//store the result in our ReLU layer
				_mm_store_si128((__m128i*) &output[pack * (int8_pack_size)], accumulator_data_first);
			}
		}
	};
}