#include "TT.h"
#include <cstring>
#include <cstdint>


void TT::clear()
{
	std::memset(_TT, 0, sizeof(_TT));
}

void TT::store(const TT_entry& entry)
{
	//https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
	const int index = ((uint32_t)entry.posKey * (uint64_t)size) >> 32;
	auto& bucket = _TT[index];
	//if we have a free space in a bucket, try to store it there
	if (bucket.curr_elements < Bucket::max_elements)
	{
		bucket.elements[bucket.curr_elements++] = entry;
	}
	//look if we can replace any element in the current index
	else
	{
		for (int bucket_index = 0; bucket_index < Bucket::max_elements; bucket_index++)
		{
			auto& bucket_elem = bucket.elements[bucket_index];
			//replace the same position, or replace based on depth
			//higher depth elements are more important, replace based on that 
			if((bucket_elem.posKey == entry.posKey) || (bucket_elem.depth <= entry.depth))
			{
				bucket_elem = entry;
				return;
			}
		}
	}
}

const TT_entry& TT::get(const TT_entry& entry, bool& found)
{
	return this->get(entry.posKey, found);
} 

const TT_entry& TT::get(unsigned long long posKey, bool& found)
{
	//https://lemire.me/blog/2016/06/27/a-fast-alternative-to-the-modulo-reduction/
	const int index = ((uint32_t)posKey * (uint64_t)size) >> 32;
	auto& bucket = _TT[index];
	for (int bucket_index = 0; bucket_index < Bucket::max_elements; bucket_index++)
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