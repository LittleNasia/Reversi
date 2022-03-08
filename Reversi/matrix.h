#pragma once
#include <cstring>



template <int rows, int cols>
class matrix
{
public:
	matrix()
	{
		//zero
		set_val(0.0);
	}
	matrix(float* vals)
	{
		std::memcpy(_data, vals, sizeof(_data));
	}
	constexpr float* begin() const { return (float*)(_data); }
	constexpr float* end() const { return (float*)(_data[rows]); }
	inline float& operator()(const int row, const int col)
	{
		return _data[row][col];
	}
	inline const float& operator()(const int row, const int col) const
	{
		return _data[row][col];
	}
	inline constexpr const float* get_row(const int row) const
	{
		return _data[row];
	}
	inline constexpr const int get_rows() const
	{
		return rows;
	}
	inline constexpr const int get_cols() const
	{
		return cols;
	}
	void set_val(const float val)
	{
		for (int row = 0; row < rows; row++)
		{
			for (int col = 0; col < cols; col++)
			{
				_data[row][col] = val;
			}
		}
	}

private:
	float _data[rows][cols];
};

