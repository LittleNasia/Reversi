#pragma once
#include <cstdint>
struct tt_entry
{
	static constexpr int flag_exact = 0;
	static constexpr int flag_alpha = 1;
	static constexpr int flag_beta = 2;
	unsigned long long posKey;
	int move;
	int score;
	int depth;
	int flag;	
};

struct bucket
{
	static constexpr int max_elements = 1;
	int curr_elements = 0;
	tt_entry elements[max_elements];
};


class tt
{
public:
	constexpr static unsigned int size = 2<<20;
	void clear();
	void store(const tt_entry& entry, bool always_replace = false);
	const tt_entry& get(const tt_entry& entry, bool& found);
	const tt_entry& get(unsigned long long posKey, bool& found);
private:
	bucket _tt[size];
};
