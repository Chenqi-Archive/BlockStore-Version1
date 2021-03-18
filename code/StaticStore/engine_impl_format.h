#pragma once

#include "engine.h"


BEGIN_NAMESPACE(StaticStore)


inline uint64 RoundFileSizeUp(uint64 file_size) {
	constexpr uint64 cluster_size = 512;
	constexpr uint64 cluster_size_mask = 0x1FF;
	static_assert(cluster_size == cluster_size_mask + 1);
	return (file_size + cluster_size_mask) & ~cluster_size_mask;
}

inline uint64 AlignOffset(uint64 offset) {
	constexpr uint64 offset_alignment = 8;
	constexpr uint64 offset_alignment_mask = 7;
	static_assert(offset_alignment == offset_alignment_mask + 1);
	return (offset + offset_alignment_mask) & ~offset_alignment_mask;
}


constexpr uint64 invalid_array_offset = (uint64)-1;


struct IndexEntry {
	uint64 array_offset;
	uint64 array_size;
};
constexpr uint64 index_entry_size = sizeof(IndexEntry);


struct StaticMetadata {
	uint64 used_size;
	IndexEntry user_metadata_entry;
	IndexEntry index_table_entry;
};
constexpr uint64 static_metadata_size = sizeof(StaticMetadata);


END_NAMESPACE(StaticStore)