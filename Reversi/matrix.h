#pragma once
#include <cstring>



template <int rows, int cols>
class matrix
{
public:
	matrix()
	{
		set_zero();
	}
	matrix(float* vals)
	{
		std::memcpy(_data, vals, sizeof(_data));
	}
	inline float& operator()(const int row, const int col)
	{
		return _data[row][col];
	}
	inline const float& operator()(const int row, const int col) const
	{
		return _data[row][col];
	}
	inline const float* get_row(const int row) const
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
	void set_zero()
	{
		std::memset(_data, 0, sizeof(_data));
	}
private:
	float _data[rows][cols];
};

