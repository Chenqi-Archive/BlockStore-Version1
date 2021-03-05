#pragma once

#include "engine.h"
#include "array.h"

#include <array>


BEGIN_NAMESPACE(DynamicStore)

using std::array;


constexpr uint64 cluster_size = 4096;  // 4kb
constexpr uint64 cluster_offset_mask = 0xFFFFFFFFFFFFF000;
static_assert(cluster_offset_mask + 0xFFF == (uint64)~0);


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
};
static_assert(sizeof(IndexEntry) == 32);


using BlockClusterArray = Array<BlockClusterEntry>;
using IndexArray = Array<IndexEntry>;
using ClusterOffsetArray = Array<uint64>;


class EngineImpl : public Engine {
public:
	EngineImpl(const wchar file[]);
	virtual ~EngineImpl() override;


	// system file api
private:
	using HANDLE = void*;
	HANDLE _file;
	uint64 _size;
private:
	void SetSize(uint64 size);


	//// format and metadata ////
private:
	bool IsFormatted() {}
public:
	virtual void Format() override {}
private:
	StaticMetadata _metadata;
private:
	virtual void LoadMetadata(void* data, uint64 size) const override {}
	virtual void StoreMetadata(void* data, uint64 size) override {}


	//// array ////	
public:
	virtual ArrayIndex CreateArray() override { return 0; }
	virtual void DestroyArray(ArrayIndex index) override {}
	virtual uint64 GetArraySize(ArrayIndex index) const override { return 0; }
	virtual void SetArraySize(ArrayIndex index, uint64 size) override {}
	virtual void ReadArray(ArrayIndex index, uint64 offset, uint64 size, void* data) const override {}
	virtual void WriteArray(ArrayIndex index, void* data, uint64 size, uint64 offset) override {}
};


END_NAMESPACE(DynamicStore)