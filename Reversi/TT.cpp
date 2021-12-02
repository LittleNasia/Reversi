#include "tt.h"
#include <cstring>
#include <cstdint>


void tt::clear()
{
	std::memset(_tt, 0, sizeof(_tt));
}

void tt::store(const tt_entry& entry, bool always_replace)
{
	//https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
	const int index = ((uint32_t)entry.posKey * (uint64_t)size) >> 32;
	auto& bucket = _tt[index];
	//if we have a free space in a bucket, try to store it there
	if (bucket.curr_elements < bucket::max_elements)
	{
		bucket.elements[bucket.curr_elements++] = entry;
	}
	//look if we can replace any element in the current index
	else
	{
		for (int bucket_index = 0; bucket_index < bucket::max_elements; bucket_index++)
		{
			auto& bucket_elem = bucket.elements[bucket_index];
			//replace the same position, or replace based on depth
			//higher depth elements are more important, replace based on that 
			//the same position replacement is mostly to guarantee we have the rootPos stored at the end of the serach, no matter what
			//why is it the first condition when it's the least likely? don't ask
			if((bucket_elem.posKey == entry.posKey) || (bucket_elem.depth <= entry.depth) || always_replace)
			{
				bucket_elem = entry;
				return;
			}
		}
	}
}

const tt_entry& tt::get(const tt_entry& entry, bool& found)
{
	return this->get(entry.posKey, found);
} 

const tt_entry& tt::get(unsigned long long posKey, bool& found)
{
	//https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
	const int index = ((uint32_t)posKey * (uint64_t)size) >> 32;
	auto& bucket = _tt[index];
	for (int bucket_index = 0; bucket_index < bucket::max_elements; bucket_index++)
	{
		//if we found element with our posKey, return it
		if (bucket.elements[bucket_index].posKey == posKey)
		{
			found = true;
			return bucket.elements[bucket_index];
		}
	}
	//return some element, however mark that it wasn't what we were looking for
	found = false;
	return bucket.elements[0];
}