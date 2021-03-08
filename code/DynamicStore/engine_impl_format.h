#pragma once

#include "engine.h"

#include <array>


BEGIN_NAMESPACE(DynamicStore)

using std::array;


//// cluster management ////

constexpr uint64 cluster_size = 4096;  // 4kb
constexpr uint64 cluster_size_bits = 12;
constexpr uint64 cluster_offset_mask = 0xFFFFFFFFFFFFF000;

static_assert(cluster_size == 4096);
static_assert(cluster_size == 1 << cluster_size_bits);
static_assert(cluster_offset_mask + cluster_size - 1 == (uint64)-1);


//// separate blocks management ////

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
constexpr uint64 block_type_number = (uint64)BlockSizeType::_Number;
static_assert(block_type_number == 10);

constexpr array<uint64, block_type_number> block_size_table         = { 0, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };
constexpr array<uint64, block_type_number - 1> array_size_map_table = { 24, 48, 96, 192, 384, 768, 1536, 3072, 6144 };

// Calculate the block size type of an array.
// Example: If array_size falls in (96, 192], then block_size_type = L64.
inline BlockSizeType GetBlockSizeType(uint64 array_size) {
#pragma message(Remark"May simply use nested if-else.")
	auto it = std::lower_bound(array_size_map_table.begin(), array_size_map_table.end(), array_size);
	uint64 table_index = it - array_size_map_table.begin();
	return static_cast<BlockSizeType>(table_index);
}


using BlockClusterIndex = ArrayIndex;

struct BlockClusterEntry {
	union {
		// for free entry
		BlockClusterIndex next_free_index;
		// for used entry
		union {
			uint free_block_head_offset : 12;
			uint64 cluster_offset;
		};
	};

	uint64 GetClusterOffset() const { return cluster_offset & cluster_offset_mask; }
	uint GetFreeBlockHeadOffset() const { return free_block_head_offset & ~cluster_offset_mask; }
};
static_assert(sizeof(BlockClusterEntry) == 8);


//// indexed arrays management ////

// for L16 - L2048 blocks whose data stored in separate blocks
struct BlockIndex {
	union {
		uint block_offset;  // bit[0-11]
		BlockClusterIndex cluster_index;  // bit[12-63]
	};
	uint GetBlockOffset() const { return block_offset & ~cluster_offset_mask; }
	BlockClusterIndex GetClusterIndex() const { return { cluster_index.value >> cluster_size_bits }; }
};
static_assert(sizeof(BlockIndex) == 8);

inline ArrayIndex GetBlockIndex(BlockSizeType block_size_type) {
	return { static_cast<uint64>(block_size_type) };
}

static constexpr uint64 initial_array_index_table_size = block_type_number - 1;


struct IndexEntry {
	static constexpr uint64 free_entry_array_size = (uint64)-1;

	uint64 array_size;
	union {
		// for free entry
		struct {
			ArrayIndex next_free_index;
			ArrayIndex prev_free_index;
			char _unused_data[8];
		};
		// for L8 blocks
		struct {
			char data[24];
		};
		// for L16 - L2048 blocks
		struct {
			BlockIndex block_index[3];
		};
		// for L4096 blocks
		struct {
			uint64 cluster_offset[2];
			ArrayIndex cluster_offset_table_index;
		};
	};

	void SetFree() { array_size = free_entry_array_size; }
	bool IsFree() const { return array_size == free_entry_array_size; }
};
static_assert(sizeof(IndexEntry) == 32);


//// static metadata management ////

constexpr uint64 max_user_metadata_size = Engine::max_metadata_size;

struct StaticMetadata {
	uint64 file_size;
	uint64 free_cluster_head_offset;
	union {
		uint64 free_index_head;
		uint64 free_block_cluster_id_head[block_type_number - 1];  // index 0 is not used, for alignment
	};
	union {
		uint64 array_index_table_offset[3];
		uint64 block_cluster_table_offset[block_type_number - 1][3];  // index 0 is not used, for alignment
	};
	uint64 user_metadata_size;
	char user_metadata[max_user_metadata_size];

	void Initialize(); // defined in engine_impl.cpp
};
static_assert(sizeof(StaticMetadata) == 512);


END_NAMESPACE(DynamicStore)