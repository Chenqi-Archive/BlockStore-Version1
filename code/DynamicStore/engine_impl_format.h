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


//// separate blocks format ////

enum class BlockType : uint64 { L8 = 0, L16, L32, L64, L128, L256, L512, L1024, L2048, L4096, _Number };
constexpr uint64 block_type_number = (uint64)BlockType::_Number;
constexpr array<uint64, block_type_number> block_size_table = { 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096 };

// Calculate the block type of an array, return value is in [0, 9].
inline BlockType GetBlockType(uint64 array_size) {
#pragma message(Remark"May simply use nested if-else.")
	auto it = std::lower_bound(block_size_table.begin(), block_size_table.end() - 1, array_size);
	uint64 table_index = it - block_size_table.begin();
	return static_cast<BlockType>(table_index);
}


//// indexed arrays format ////

static constexpr uint64 entry_block_offset_mask = 0xFFFFFFFFFFFFFFF0;

struct IndexEntry {
	uint64 array_size;
	union {
		// for free entry
		uint64 next_free_index;

		// for L8 blocks (data is directly stored in the entry)
		uint64 data; // char data[8];

		// for L16 - L2048 blocks (data is stored in the block at offset)
		union {
			BlockType block_type;  // bits 0-3
			uint64 offset;		   // bits 4-63
		};

		// for L4096 blocks
		//uint64 offset;
	};

	uint64 GetBlockOffset() const { return offset & entry_block_offset_mask; }
	BlockType GetBlockType() const { return (BlockType)((uint64)block_type & ~entry_block_offset_mask); }
};

constexpr uint64 index_entry_size = sizeof(IndexEntry);
static_assert(index_entry_size == 16);


//// static metadata format ////

constexpr ArrayIndex free_index_tail = { (uint64)-1 };
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