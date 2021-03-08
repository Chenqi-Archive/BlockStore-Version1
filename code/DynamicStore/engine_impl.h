#pragma once

#include "engine_impl_format.h"
#include "array.h"


BEGIN_NAMESPACE(DynamicStore)


class EngineImpl : public Engine {
public:
	EngineImpl(const wchar file[]);
	virtual ~EngineImpl() override;


	//// system file api ////
private:
	using HANDLE = void*;
	HANDLE _file;
	uint64 _size;
private:
	void SetSize(uint64 size);


	//// file map view cache management ////

	/* not implemented yet, just map the entire file. */
	/* put the file map view cache in a seperate class. (Win32File) */
private:
	HANDLE _file_map;
	void* _map_view_address;
private:
	void DoMapping();
	void UndoMapping();
private:
	void* GetClusterAddress(uint64 cluster_offset) {
		assert((cluster_offset & ~cluster_offset_mask) == 0);
		return (char*)_map_view_address + cluster_offset;
	}

	template<class T> 
	static T Get(void* cluster_address, uint64 offset_in_cluster) {
		assert(offset_in_cluster < cluster_size);
		return *(T*)((char*)cluster_address + offset_in_cluster);
	}
	template<class T> 
	static void Set(void* cluster_address, uint64 offset_in_cluster, const T& obj) {
		assert(offset_in_cluster < cluster_size);
		*(T*)((char*)cluster_address + offset_in_cluster) = pbj;
	}


	//// format ////
private:
	bool LoadAndCheck();
	void Format();


	//// static metadata management ////
private:
	StaticMetadata _static_metadata;
private:
	virtual void LoadUserMetadata(void* data, uint64 size) const override;
	virtual void StoreUserMetadata(const void* data, uint64 size) override;


	//// separate blocks management ////


	IndexEntry GetIndexEntry(ArrayIndex index) {
		_static_metadata.array_index_table_offset
	}


	BlockClusterEntry GetBlockClusterEntry(BlockSizeType block_size_type, BlockClusterIndex cluster_index) {
		assert(BlockSizeType::L8 < block_size_type && block_size_type < BlockSizeType::L4096);
		IndexEntry block_index_entry = GetIndexEntry(GetBlockIndex(block_size_type));
		
	}


	void InitializeBlockCluster(BlockSizeType block_size_type, uint64 cluster_offset, uint64 next_free_block) {
		assert(BlockSizeType::L8 < block_size_type && block_size_type <= BlockSizeType::L4096);
		uint64 block_size = block_size_table[(uint64)block_size_type];
		void* cluster_address = GetClusterAddress(cluster_offset);
		for (uint64 block_offset = cluster_size - block_size; block_offset < cluster_size; block_offset -= block_size) {
			Set(cluster_address, block_offset, next_free_block);
			next_free_block = block_offset;
		}
	}


	//// indexed arrays management ////





public:
	virtual ArrayIndex CreateArray() override;
	virtual void DestroyArray(ArrayIndex index) override;
	virtual uint64 GetArraySize(ArrayIndex index) const override;
	virtual void SetArraySize(ArrayIndex index, uint64 size) override;
	virtual void ReadArray(ArrayIndex index, uint64 offset, uint64 size, void* data) const override;
	virtual void WriteArray(ArrayIndex index, const void* data, uint64 size, uint64 offset) override;
};


END_NAMESPACE(DynamicStore)