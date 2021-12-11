#include <array>

using std::array;

using uint64 = unsigned long long;


enum class BlockType : uint64 { L8 = 0, L16, L32, L64, L128, L256, L512, L1024, L2048, L4096, _Number };
constexpr uint64 block_type_number = (uint64)BlockType::_Number;
constexpr array<uint64, block_type_number> block_size_table = { 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };

// Calculate the block type of an array, return value is in [L8, L4096]. ([0,9])
inline BlockType GetBlockType(uint64 array_size) {
#pragma message(Remark"May simply use nested if-else.")
	auto it = std::lower_bound(block_size_table.begin(), block_size_table.end() - 1, array_size);
	uint64 table_index = it - block_size_table.begin();
	return static_cast<BlockType>(table_index);
}

int main() {
	BlockType type;
	type = GetBlockType(0);		// L8
	type = GetBlockType(1);		// L8
	type = GetBlockType(7);		// L8
	type = GetBlockType(8);		// L8
	type = GetBlockType(255);	// L256
	type = GetBlockType(512);	// L512
	type = GetBlockType(1536);	// L2048
	type = GetBlockType(4095);	// L4096
	type = GetBlockType(4096);	// L4096
	type = GetBlockType(4097);	// L4096
}
