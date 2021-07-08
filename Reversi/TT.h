#pragma once
#include <cstdint>
struct TT_entry
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

struct Bucket
{
	static constexpr int max_elements = 1;
	int curr_elements = 0;
	TT_entry elements[max_elements];
};


class TT
{
public:
	constexpr static unsigned int size = 6000000;
	void clear();
	void store(const TT_entry& entry);
	const TT_entry& get(const TT_entry& entry, bool& found);
	const TT_entry& get(unsigned long long posKey, bool& found);
private:
	Bucket _TT[size];
};
