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
		assert(offset_in_cluster + sizeof(T) <= cluster_size);
		return *(T*)((char*)cluster_address + offset_in_cluster);
	}
	template<class T> 
	static void Set(void* cluster_address, uint64 offset_in_cluster, const T& obj) {
		assert(offset_in_cluster < cluster_size);
		assert(offset_in_cluster + sizeof(T) <= cluster_size);
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
private:

	uint64 InitializeClusterSection(BlockType block_type, uint64 cluster_offset, uint64 begin_offset_in_cluster, uint64 end_offset_in_cluster) {
		assert(block_type > BlockType::L8 && block_type <= BlockType::L4096);
		uint64 block_size = block_size_table[(uint64)block_type];
		void* cluster_address = GetClusterAddress(cluster_offset);
		uint64 next_free_block = _static_metadata.free_block_head[(uint64)block_type];
		for (uint64 block_offset = end_offset_in_cluster - block_size; 
			 block_offset >= begin_offset_in_cluster && block_offset < end_offset_in_cluster;
			 block_offset -= block_size) {
			Set<uint64>(cluster_address, block_offset, next_free_block);
			next_free_block = block_offset;
		}
		return _static_metadata.free_block_head[(uint64)block_type] = next_free_block;
	}

	uint64 InitializeCluster(BlockType block_type, uint64 cluster_offset) {
		return InitializeClusterSection(block_type, cluster_offset, 0, cluster_size);
	}

	// Returns the offset of the new cluster.
	uint64 ExtendFileByOneCluster() {
		uint64 old_size = _static_metadata.file_size;
		SetSize(old_size + cluster_size);
		return old_size;
	}

	uint64 AllocateBlock(BlockType block_type) {
		assert(block_type > BlockType::L8 && block_type <= BlockType::L4096);
		uint64 current_free_block = _static_metadata.free_block_head[(uint64)block_type];
		if (current_free_block == free_block_tail) {
			uint64 cluster_offset = ExtendFileByOneCluster();
			current_free_block = InitializeCluster(block_type, cluster_offset);
		}
		assert(current_free_block % block_size_table[(uint64)block_type] == 0);
		void* cluster_address = GetClusterAddress(current_free_block & cluster_offset_mask);
		uint64 next_free_block = Get<uint64>(cluster_address, current_free_block & ~cluster_offset_mask);
		_static_metadata.free_block_head[(uint64)block_type] = next_free_block;
		return current_free_block;
	}

	void DeallocateBlock(BlockType block_type, uint64 block_offset) {
		assert(block_type > BlockType::L8 && block_type <= BlockType::L4096);
		assert(block_offset % block_size_table[(uint64)block_type] == 0);
		void* cluster_address = GetClusterAddress(block_offset & cluster_offset_mask);
		uint64 next_free_block = _static_metadata.free_block_head[(uint64)block_type];
		Set<uint64>(cluster_address, block_offset & ~cluster_offset_mask, next_free_block);
		_static_metadata.free_block_head[(uint64)block_type] = block_offset;
	}

	void MoveDataBetween(uint64 source_block_offset, uint64 data_size, uint64 destination_block_offset) {
		void* source_cluster_address = GetClusterAddress(source_block_offset & cluster_offset_mask);
		void* destination_cluster_address = GetClusterAddress(destination_block_offset & cluster_offset_mask);

		///

	}


	struct LargeArrayClusterIterator {
		EngineImpl& engine;
		uint64 array_size;

		uint64 current_cluster_offset;

	public:
		LargeArrayClusterIterator() {}

		void SeekToCluster(uint64 offset_in_array) {
			assert(offset_in_array < array_size);

		}
		void GotoPrevCluster() {

		}
		void GotoNextCluster() {
			
		}
	};

	IndexEntry ResizeLargeArray(IndexEntry entry) {

	}

	// space recycling and defragmentation
private:
	void SortFreeBlockList() {

		// to implement (put the algorithm in a seperate header file)

	}

	void RecycleFreeBlockList() {

	}


	//// indexed arrays management ////
private:

	IndexEntry GetIndexEntry(ArrayIndex index) {
		IndexEntry index_table_entry = _static_metadata.index_table_entry;
		uint64 entry_offset = index.value * index_entry_size;
		if (entry_offset >= index_table_entry.array_size) { throw std::invalid_argument("invalid array index"); }

		/* index entry table can be cached */

		BlockType type = GetBlockType(index_table_entry.array_size);
		assert(type > BlockType::L8);
		if (type < BlockType::L4096) {
			void* cluster_address = GetClusterAddress(index_table_entry.offset & cluster_offset_mask);
			uint64 offset_in_cluster = (index_table_entry.offset & ~cluster_offset_mask) + entry_offset;
			return Get<IndexEntry>(cluster_address, offset_in_cluster);
		} else {

		}
	}

	IndexEntry ResizeIndexEntry(IndexEntry entry, uint64 size) {
		BlockType old_type = GetBlockType(entry.array_size);
		BlockType new_type = GetBlockType(size);
		entry.array_size = size;

		// If block type did not change, just return.
		if (old_type == new_type && old_type != BlockType::L4096) { return entry; }

		// If block type shrinks, allocate a new block and copy the data
	}

	ArrayIndex ExtendIndexTable() {
		IndexEntry index_table_entry = _static_metadata.index_table_entry;
		ArrayIndex 
		index_table_entry = ResizeIndexEntry(index_table_entry, index_table_entry.array_size + index_entry_size);


	}

	void InitializeIndexEntry() {

	}

	ArrayIndex AllocateIndex() {
		ArrayIndex current_free_index = _static_metadata.free_index_head;
		if (current_free_index.value == free_index_tail) {
			ResizeIndexEntry()
		}
		ArrayIndex next_free_index
	}

	void DeallocateIndex(ArrayIndex index) {

	}


public:
	virtual ArrayIndex CreateArray() override;
	virtual void DestroyArray(ArrayIndex index) override;
	virtual uint64 GetArraySize(ArrayIndex index) const override;
	virtual void SetArraySize(ArrayIndex index, uint64 size) override;
	virtual void ReadArray(ArrayIndex index, uint64 offset, uint64 size, void* data) const override;
	virtual void WriteArray(ArrayIndex index, const void* data, uint64 size, uint64 offset) override;
};


END_NAMESPACE(DynamicStore)