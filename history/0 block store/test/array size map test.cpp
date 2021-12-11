#include <array>

using std::array;
using uint64 = unsigned long long;

enum class BlockSizeType : uint64 {
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
static_assert((uint64)BlockSizeType::_Number == 10);

constexpr uint64 separate_block_number = 8;

constexpr array<uint64, (uint64)BlockSizeType::_Number> block_size_table = { 0, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };
constexpr array<uint64, (uint64)BlockSizeType::_Number - 1> array_size_map_table = { 24, 48, 96, 192, 384, 768, 1536, 3072, 6144 };

// Example: If array_size falls in (96, 192], then block_size_type = L64.
inline BlockSizeType GetBlockSizeType(uint64 array_size) {
#pragma message(Remark"May simply use nested if-else.")
	auto it = std::lower_bound(array_size_map_table.begin(), array_size_map_table.end(), array_size);
	uint64 table_index = it - array_size_map_table.begin();
	return static_cast<BlockSizeType>(table_index);
}

int main() {
	BlockSizeType type;
	type = GetBlockSizeType(0);		 // L8
	type = GetBlockSizeType(1);		 // L8
	type = GetBlockSizeType(24);	 // L8
	type = GetBlockSizeType(25);	 // L16
	type = GetBlockSizeType(245);	 // L128
	type = GetBlockSizeType(786);	 // L512
	type = GetBlockSizeType(1536);	 // L512
	type = GetBlockSizeType(6144);	 // L2048
	type = GetBlockSizeType(6145);	 // L4096
	type = GetBlockSizeType(48416);	 // L4096
}