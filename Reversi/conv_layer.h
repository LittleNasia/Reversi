#pragma once
#include <functional>
#include <fstream>

#include "matrix.h"

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
	using output_type = matrix<padded_output_rows, padded_output_cols>;
	using input_type = matrix<input_rows, input_cols>;
	using output_type_flattened = float[padded_output_cols * padded_output_rows * output_channels];
	using weights_matrix = matrix<filter_rows, filter_cols>;

private:
	output_type m_output[output_channels];
	weights_matrix m_filters[input_channels][output_channels];
	float m_biases[output_channels];
	std::function<void(output_type&)> m_activation_function;
	std::function<void(output_type_flattened&)> m_activation_function_flattened;
	output_type_flattened m_flattened_output;

public:
	const output_type* getOutput() const
	{
		return m_output;
	}

	const float* flatten() const
	{
		return m_flattened_output;
	}

	conv_layer()
	{
		for (int input_channel = 0; input_channel < input_channels; input_channel++)
		{
			for (int output_channel = 0; output_channel < output_channels; output_channel++)
			{
				for (int filter_row = 0; filter_row < filter_rows; filter_row++)
				{
					for (int filter_col = 0; filter_col < filter_cols; filter_col++)
					{
						m_filters[input_channel][output_channel](filter_row, filter_col) = 1.0f;
					}
				}
			}
		}
	}
	conv_layer(std::ifstream& weightsFile)
	{
		
		/*m_activation_function_flattened = [&](output_type_flattened& matrix) {for (auto& val : matrix.reshaped())
		{
			val = val > 0 ? val : 0;
		}
		};
		for (int filter_row = 0; filter_row < filter_rows; filter_row++)
		{
			for (int filter_col = 0; filter_col < filter_cols; filter_col++)
			{
				for (int input_channel = 0; input_channel < input_channels; input_channel++)
				{
					for (int output_channel = 0; output_channel < output_channels; output_channel++)
					{
						weightsFile.read((char*)&m_filters[input_channel][output_channel](filter_row, filter_col), sizeof(float));
					}
				}
			}
		}
		weightsFile.read((char*)m_biases, output_channels * sizeof(float));*/
	}

	const output_type* forward(const input_type* input)
	{
		for (int output_channel = 0; output_channel < output_channels; output_channel++)
		{
			auto& curr_output = m_output[output_channel];
			curr_output.set_zero();

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
							const float&& sum = dot_prod(curr_filter_row, curr_input, output_row, output_col);
							curr_output(output_row_index, output_col_index) += sum;
							const int flattened_output_index = 0;
						}
					}

				}
			}
		}
		return m_output;
	}

	inline constexpr float dot_prod(const float* filter, const input_type input, int row, int col)
	{
		const __m128 filter_row = _mm_load_ps(filter);
		const __m128 input_part = _mm_load_ps(&input(row, col));

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