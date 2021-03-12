#pragma once

#include "engine.h"

#include <array>


BEGIN_NAMESPACE(DynamicStore)

using std::array;


//// cluster format ////

constexpr uint64 cluster_size = 4096;  // 4kb
constexpr uint64 cluster_size_bits = 12;
constexpr uint64 cluster_offset_mask = 0xFFFFFFFFFFFFF000;

static_assert(cluster_size == 1 << cluster_size_bits);
static_assert(cluster_offset_mask + cluster_size - 1 == (uint64)-1);


constexpr uint64 cluster_index_size = sizeof(uint64);
constexpr uint64 cluster_index_number = cluster_size / cluster_index_size;
constexpr uint64 cluster_index_bits = 9;
constexpr uint64 cluster_index_mask = 0x1FF;
constexpr uint64 max_cluster_hierarchy_depth = 6;  // ceil((64 - 12) / 9)

static_assert(cluster_index_number == 1 << cluster_index_bits);
static_assert(cluster_index_mask == cluster_index_number - 1);

inline uint64 GetClusterNumber(uint64 size) { 
	return (size + ~cluster_offset_mask) >> cluster_size_bits; 
}

inline uint64 GetClusterLogicIndexOfLevel(uint64 offset, uint64 level) {
	assert(level < max_cluster_hierarchy_depth);
	return offset >> (cluster_size_bits + level * 9);
}


//// separate blocks format ////

enum class BlockType : uint64 { L8 = 0, L16, L32, L64, L128, L256, L512, L1024, L2048, L4096, L4096Plus, _Number = L4096Plus};
constexpr uint64 block_type_number = (uint64)BlockType::_Number;
static_assert(block_type_number == 10);  // L4096Plus is not included in seperate block types

constexpr array<uint64, block_type_number> block_size_table = { 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };

// Calculate the block type of an array, return value is in [L8, L4096Plus]. (or [0, 10])
inline BlockType GetBlockType(uint64 array_size) {
	auto it = std::lower_bound(block_size_table.begin(), block_size_table.end(), array_size);
	uint64 table_index = it - block_size_table.begin();
	return static_cast<BlockType>(table_index);
}


//// indexed arrays format ////

constexpr uint64 free_entry_array_size = (uint64)-1;

struct IndexEntry {
	uint64 array_size;
	union {
		// for free entry
		ArrayIndex next_free_index;

		// for L8 blocks (data is directly stored in the entry)
		uint64 data; // char data[8];

		// for L16 - L4096 blocks (data is stored in the block at offset)
		// for L4096Plus blocks (data is stored in hierarchical indexed clusters)
		uint64 offset;
	};
};
constexpr uint64 index_entry_size = sizeof(IndexEntry);
static_assert(index_entry_size == 16);


//// static metadata format ////

constexpr ArrayIndex free_index_tail = ArrayIndex((uint64)-1);
constexpr uint64 free_block_tail = (uint64)-1;
constexpr uint64 max_user_metadata_size = Engine::max_metadata_size;

struct StaticMetadata {
	uint64 file_size;
	IndexEntry index_table_entry;
	union {
		ArrayIndex free_index_head;
		uint64 free_block_head[block_type_number - 1];
	};
	uint64 free_cluster_head; // &free_block_head[L4096] == &free_cluster_head.
	uint64 user_metadata_size;
	char user_metadata[max_user_metadata_size];
};
static_assert(sizeof(StaticMetadata) == 256);


END_NAMESPACE(DynamicStore)