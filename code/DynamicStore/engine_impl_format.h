#pragma once

#include "engine.h"
#include "array.h"

#include <array>
#include <algorithm>


BEGIN_NAMESPACE(DynamicStore)

using std::array;


constexpr uint64 cluster_size = 4096;  // 4kb
constexpr uint64 cluster_offset_mask = 0xFFFFFFFFFFFFF000;
static_assert(cluster_offset_mask + cluster_size - 1 == (uint64)-1);

constexpr uint64 invalid_array_size = (uint64)-1;

enum class LengthType : uint64 {
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
static_assert((uint64)LengthType::_Number == 10);

constexpr array<uint64, (uint64)LengthType::_Number> block_length_table = { 0, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };
constexpr uint64 separate_block_number = 8;

inline LengthType GetArrayLengthType(uint64 array_length) {
	auto cmp = [](uint64 block_length, uint64 array_length) { return block_length <= array_length; };
#pragma message(Remark"Use nested if-else for optimization.")
	auto it = std::lower_bound(block_length_table.begin(), block_length_table.end(), array_length, cmp);
	assert(it > block_length_table.begin());
	uint64 table_index = it - block_length_table.begin() - 1;
	if (table_index != 0 && array_length <= block_length_table[table_index] * 3 / 2) { table_index--; }
	return static_cast<LengthType>(table_index);
}


struct BlockEntry {
	uint64 free_cluster_id_head;
};

struct BlockClusterEntry {
	union {
		uint free_block_head_offset;
		uint64 cluster_offset;
	};
	uint64 GetClusterOffset() const { return cluster_offset & cluster_offset_mask; }
	uint GetFreeBlockHeadOffset() const { return free_block_head_offset & ~cluster_offset_mask; }
};
static_assert(sizeof(BlockClusterEntry) == 8);


struct StaticMetadata {
	uint64 file_size;
	uint64 free_cluster_head_offset;
	uint64 free_index_head;
	BlockEntry seperate_block[separate_block_number];
	uint64 user_metadata_size;
	char user_metadata[Engine::max_metadata_size];
};


struct BlockClusterIndex {
	union {
		uint block_offset;
		uint64 cluster_index;
	};
	uint64 GetClusterIndex() const { return cluster_index & cluster_offset_mask; }
	uint GetBlockOffset() const { return block_offset & ~cluster_offset_mask; }
};
static_assert(sizeof(BlockClusterIndex) == 8);


struct IndexEntry {
	uint64 array_size;
	union {
		struct {
			char data[24];
		};
		struct {
			BlockClusterIndex block_index[3];
		};
		struct {
			uint64 cluster_offset[2];
			ArrayIndex cluster_offset_array_index;
		};
	};

	bool IsInvalid() const { return array_size == invalid_array_size; }
};
static_assert(sizeof(IndexEntry) == 32);


using BlockClusterArray = Array<BlockClusterEntry>;
using IndexArray = Array<IndexEntry>;
using ClusterOffsetArray = Array<uint64>;


END_NAMESPACE(DynamicStore)