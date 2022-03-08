#pragma once
#include <functional>
#include <fstream>

#include "matrix.h"
namespace CNN
{


template <int input_rows, int input_cols, int input_channels,
	int filter_rows, int filter_cols, 
	int output_rows, int output_cols, int output_channels,
	int left_col_padding, int right_col_padding, int top_row_padding, int bottom_row_padding,
	bool is_flattened>
	class conv_layer
{
public:
	static constexpr int padded_output_cols = left_col_padding + right_col_padding + output_cols;
	static constexpr int padded_output_rows = top_row_padding + bottom_row_padding + output_rows;
	static constexpr int flattened_output_size = padded_output_cols * padded_output_rows * output_channels;
	using output_type = matrix<padded_output_rows, padded_output_cols>;
	using input_type = matrix<input_rows, input_cols>;
	using output_type_flattened = float[flattened_output_size];
	using weights_matrix = matrix<filter_rows, filter_cols>;

private:
	alignas(128) output_type m_output[output_channels];
	alignas(128) weights_matrix m_filters[input_channels][output_channels];
	alignas(128) float m_biases[output_channels];
	alignas(128) output_type_flattened m_flattened_output;

	//bias templates are used because of the padding
	//instead of calculating which index doesn't belong to the zero padding every time forward pass is run (and so bias isn't added there)
	//we can just calculate that once, and then just memcpy the result at the begining of each forward pass
	alignas(128) output_type m_output_bias_template[output_channels];
	alignas(128) output_type_flattened m_flattened_output_bias_template;

public:
	const output_type* getOutput() const
	{
		return m_output;
	}

	const float* flatten() const
	{
		return m_flattened_output;
	}

	void print_output()
	{
		for (int output_channel = 0; output_channel < 6; output_channel++)
		{
			for (int row = 0; row < 8; row++)
			{
				for (int col = 0; col < 8; col++)
				{
					std::cout << "[" << m_output[output_channel](row, col) << "]";
				}
				std::cout << "\n";
			}
			std::cout << "\n\n";
		}
		std::cout << "\n\n\n";
	}

	void load_weights(float* weights, float* biases)
	{
		for (int filter_row = 0, index =0; filter_row < filter_rows; filter_row++)
		{
			for (int filter_col = 0; filter_col < filter_cols; filter_col++)
			{
				for (int input_channel = 0; input_channel < input_channels; input_channel++)
				{
					for (int output_channel = 0; output_channel < output_channels; output_channel++, index++)
					{
						m_filters[input_channel][output_channel](filter_row, filter_col) = weights[index];
					}
				}
			}
		}
		for (int output_channel = 0,index=0; output_channel < output_channels; output_channel++, index++)
		{
			m_biases[output_channel] = biases[index];
		}


		std::memset(m_flattened_output_bias_template, 0, sizeof(m_flattened_output_bias_template));
		std::memset(&m_output_bias_template, 0, sizeof(m_output_bias_template));
		//prepare the bias templates
		for (int input_channel = 0; input_channel < input_channels; input_channel++)
		{
			for (int output_row = 0; output_row < output_rows; output_row++)
			{
				for (int output_col = 0; output_col < output_cols; output_col++)
				{
					for (int output_channel = 0; output_channel < output_channels; output_channel++)
					{
						// include paddings in the row indices
						// don't forget the output is zero-padded to make sure input and output dimensions are identical
						const int output_row_index = output_row + top_row_padding;
						const int output_col_index = output_col + left_col_padding;

						
						m_output_bias_template[output_channel](output_row_index, output_col_index) = m_biases[output_channel];

						const int flattened_output_index = output_channel +
							(output_col + left_col_padding) * output_channels +
							(output_row + top_row_padding) * padded_output_cols * output_channels;

						m_flattened_output_bias_template[flattened_output_index] = m_biases[output_channel];
					}
				}
			}
		}
	}

	conv_layer()
	{
	    
		

	}
	conv_layer(std::ifstream& weightsFile)
	{
		
	}

	const output_type* forward(const input_type* input)
	{
		if (is_flattened)
		{
			std::memcpy(m_flattened_output, m_flattened_output_bias_template, sizeof(m_flattened_output));
		}
		for (int output_channel = 0; output_channel < output_channels; output_channel++)
		{
			auto& curr_output = m_output[output_channel];
			//we can just set the entire layer to be equal to the bias template
			std::memcpy(&m_output[output_channel], &m_output_bias_template[output_channel], sizeof(m_output[output_channel]));

			for (int input_channel = 0; input_channel < input_channels; input_channel++)
			{
				const auto& curr_input = input[input_channel];
				const auto& curr_filter = m_filters[input_channel][output_channel];
				
				for (int filter_row = 0; filter_row < filter_rows; filter_row++)
				{
					// iterate every single filter row separately (as opposed to computing entire filter at once)
					// each filter goes through the entire input, then the next row etc. 
					// however, different rows of different filters do not ever see some of the rows of the input matrix
					// for example, row 1 of the filter will never see the last row of the input

					const auto& curr_filter_row = curr_filter.get_row(filter_row);

					for (int output_row = 0; output_row < output_rows; output_row++)
					{
						for (int output_col = 0; output_col < output_cols; output_col++)
						{
							// include paddings in the row indices
							// don't forget the output is zero-padded to make sure input and output dimensions are identical
							const int output_row_index = output_row + top_row_padding;
							const int output_col_index = output_col + left_col_padding;
							//std::cout << output_row_index << " " << output_row + filter_row << "\n";
							// multiplies the row of the weight matrix with corresponding input slice and sums the result
							const float&& sum= dot_prod(curr_filter_row, curr_input, output_row + filter_row, output_col);
							//std::cout << sum << "\n";
							if(!is_flattened)
								curr_output(output_row_index, output_col_index) += sum;
							else
							{
								//the order of loops if we were to convert a 3D layer to 1D layer is as follows:
								// 1. row
								// 2. col
								// 3. output_channel
								//however, our order is different, and so we can't just do index++ in the innermost loop
								//this is due to how tensorflow works
 								//this index calculation is made to make sure it is compatible with it
								const int flattened_output_index = output_channel +
									(output_col + left_col_padding) * output_channels +
									(output_row + top_row_padding) * padded_output_cols * output_channels;
								m_flattened_output[flattened_output_index] += sum;
							}
							
						}
					}

				}
			}


			
			/*for (int input_channel = 0; input_channel < input_channels; input_channel++)
			{
				for (int output_row = 0; output_row < output_rows; output_row++)
				{
					for (int output_col = 0; output_col < output_cols; output_col++)
					{
						float sum = dot_prod_naive(m_filters[input_channel][output_channel], input[input_channel], output_row, output_col);
						m_output[output_channel](output_row + top_row_padding, output_col + left_col_padding)
							+= sum;
						if (is_flattened)
						{
							const int flattened_output_index = output_channel +
								(output_col + top_row_padding) * output_channels +
								(output_row + left_col_padding) * padded_output_cols * output_channels;
							m_flattened_output[flattened_output_index] += sum;
						}
						
					}
				}
			}*/

			//apply relu to the current output channel as its value is final
			for (int output_col = 0; output_col < padded_output_cols; output_col++)
			{
				for (int output_row= 0; output_row < padded_output_rows; output_row++)
				{
					if (curr_output(output_col, output_row) < 0)
					{
						curr_output(output_col, output_row) = 0;
					}
				}
			}
		}
		

		//apply relu to the flattened output
		if (is_flattened)
		{
			for (int neuron = 0; neuron < flattened_output_size; neuron++)
			{
				if (m_flattened_output[neuron] < 0)
				{
					m_flattened_output[neuron] = 0;
				}
			}
		}
		return m_output;
	}

	inline constexpr float dot_prod_naive(const matrix<4, 4>& filter, const input_type& input, int row, int col)
	{
		float sum = 0;
		for (int filter_row = 0; filter_row < filter_rows; filter_row++)
		{
			for (int filter_col = 0; filter_col < filter_cols; filter_col++)
			{
				sum += input(row + filter_row, col + filter_col) * filter(filter_row, filter_col);
			}
		}
		return sum;
	}

	inline constexpr float dot_prod(const float* filter, const input_type& input, int row, int col)
	{
		const __m128 filter_row = _mm_load_ps(filter);
		const __m128 input_part = _mm_loadu_ps(&input(row, col));

		__m128 elementwise_mul_result = _mm_mul_ps(filter_row, input_part);

		// sum all results using two horizontal sums
		__m128 first_sum_result = _mm_hadd_ps(elementwise_mul_result, elementwise_mul_result);
		__m128 summed_vector = _mm_hadd_ps(first_sum_result, first_sum_result);

		// returns the lowest byte in a vector
		return _mm_cvtss_f32(summed_vector);
	}


	inline constexpr output_type& forward_flattened(const input_type& input)
	{
		return m_output;
	}




};

}