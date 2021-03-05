#include "engine.h"

#include <array>
#include <algorithm>


BEGIN_NAMESPACE(DynamicStore)

BEGIN_NAMESPACE(Anonymous)

using std::array;


enum LengthType {
	L8 = 0,		// stored in IndexEntry
	L16,		// } 
	L32,		// } 
	L64,		// } 
	L128,		// } stored in separate 
	L256,		// }  blocks
	L512,		// } 
	L1024,		// } 
	L2048,		// } 
	L4096,		// stored in 4k clusters
	_Number
};
static_assert(LengthType::_Number == 10);
constexpr array<uint64, LengthType::_Number> block_length_table = { 0, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };

LengthType GetArrayLengthType(uint64 array_length) {
	auto cmp = [](uint64 block_length, uint64 array_length) { return block_length <= array_length; };
#pragma message(Remark"Use nested if-else for optimization.")
	auto it = std::lower_bound(block_length_table.begin(), block_length_table.end(), array_length, cmp);
	assert(it > block_length_table.begin());
	uint64 table_index = it - block_length_table.begin() - 1;
	if (table_index != 0 && array_length <= block_length_table[table_index] * 3 / 2) { table_index--; }
	return static_cast<LengthType>(table_index);
}


struct BlockEntry {
	uint64 free_cluster_head;
};


struct IndexEntry {
	uint64 array_size;
	union {
		struct {
			char data[24];
		};
		struct {
			uint64 offset[3];
		};
		struct {
			uint64 offset[2];
			Index index;
		};
	};
};
static_assert(sizeof(IndexEntry) == 32);


END_NAMESPACE(Anonymous)





END_NAMESPACE(DynamicStore)