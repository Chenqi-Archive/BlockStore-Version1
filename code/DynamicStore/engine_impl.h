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
	void* GetClusterAddress(uint64 cluster_offset) const {
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

	template<class T>
	static T Get(uint64 offset_in_file) {
		void* cluster_address = GetClusterAddress(offset_in_file & cluster_offset_mask);
		return Get<T>(cluster_address, offset_in_file & ~cluster_offset_mask);
	}
	template<class T>
	static void Set(uint64 offset_in_file, const T& obj) {
		void* cluster_address = GetClusterAddress(offset_in_file & cluster_offset_mask);
		Set<T>(cluster_address, offset_in_file & ~cluster_offset_mask, obj);
	}

	void MoveData(uint64 source_offset, uint64 destination_offset, uint64 data_size) {
		assert(source_offset + data_size <= destination_offset || destination_offset + data_size <= source_offset);
		uint64 source_cluster_offset = source_offset & cluster_offset_mask;
		uint64 destination_cluster_offset = destination_offset & cluster_offset_mask;
		uint64 source_offset_in_cluster = source_offset & ~cluster_offset_mask;
		uint64 destination_offset_in_cluster = destination_offset & ~cluster_offset_mask;
		assert(source_offset_in_cluster + data_size <= cluster_size);
		assert(destination_offset_in_cluster + data_size <= cluster_size);
		char* source_address = (char*)GetClusterAddress(source_cluster_offset) + source_offset_in_cluster;
		char* destination_address = (char*)GetClusterAddress(destination_cluster_offset) + destination_offset_in_cluster;
		memcpy(destination_address, source_address, data_size);
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
	uint64 InitializeClusterSection(BlockType block_type, uint64 cluster_offset, uint64 begin_offset, uint64 end_offset);
	uint64 InitializeCluster(BlockType block_type, uint64 cluster_offset);
	uint64 ExtendFileByOneCluster();

	uint64 AllocateBlock(BlockType block_type);
	void DeallocateBlock(BlockType block_type, uint64 block_offset);


	// space recycling and defragmentation
private:
	void SortFreeBlockList();
	void RecycleFreeBlockList();


	//// L4096Plus cluster hierarchy management ////
private:
	class L4096PlusClusterIterator {
	private:
		EngineImpl& engine;
		IndexEntry entry;
	private:
		struct ClusterIndexLevelStatus {
			uint64 cluster_number;
			uint64 current_cluster_logic_index;
			uint64 current_cluster_offset;
		};
		ClusterIndexLevelStatus cluster_level_stack[max_cluster_hierarchy_depth];
		uint64 stack_top_level;
		uint64 current_offset_in_array;
	public:
		L4096PlusClusterIterator(EngineImpl& engine, IndexEntry entry);
		void SeekToCluster(uint64 offset_in_array);
	public:
		void GotoPrevCluster() {
			assert(current_offset_in_array != -1);
			SeekToCluster(current_offset_in_array - cluster_size);
		}
		void GotoNextCluster() {
			assert(current_offset_in_array != -1);
			SeekToCluster(current_offset_in_array + cluster_size);
		}
	public:
		uint64 GetCurrentClusterOffset() const {
			assert(current_offset_in_array != -1);
			return cluster_level_stack[0].current_cluster_offset;
		}
		void* GetCurrentClusterAddress() const {
			return engine.GetClusterAddress(GetCurrentClusterOffset());
		}
	};

	uint64 AllocateClusterIndexBlock(uint64 first_cluster_offset, uint64 size);
	void DeallocateClusterIndexBlock(uint64 cluster_index_block_offset, uint64 current_allocated_size);


	//// indexed arrays management ////
private:
	uint64 GetIndexEntryOffset(ArrayIndex index) const;
	IndexEntry GetIndexEntry(ArrayIndex index) const;
	void SetIndexEntry(ArrayIndex index, IndexEntry entry);
	
	void InitializeIndexEntry(uint64 index_entry_offset_begin, uint64 index_entry_offset_end);
	void ExtendIndexTable();
	ArrayIndex AllocateIndex();
	void DeallocateIndex(ArrayIndex index);

	IndexEntry UpgradeL4096PlusIndexEntry(IndexEntry entry_with_new_size);
	IndexEntry DowngradeL4096PlusIndexEntry(uint64 old_offset, uint64 old_size);
	IndexEntry ResizeL4096PlusIndexEntry(uint64 old_offset, uint64 old_size, uint64 new_size);
	IndexEntry ResizeIndexEntry(IndexEntry entry, uint64 new_size);

public:
	virtual ArrayIndex CreateArray() override;
	virtual void DestroyArray(ArrayIndex index) override;
	virtual uint64 GetArraySize(ArrayIndex index) const override;
	virtual void SetArraySize(ArrayIndex index, uint64 size) override;
	virtual void ReadArray(ArrayIndex index, uint64 offset, uint64 size, void* data) const override;
	virtual void WriteArray(ArrayIndex index, const void* data, uint64 size, uint64 offset) override;
};


END_NAMESPACE(DynamicStore)