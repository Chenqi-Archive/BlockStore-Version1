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
		*(T*)((char*)cluster_address + offset_in_cluster) = obj;
	}

	template<class T>
	T Get(uint64 offset_in_file) const {
		void* cluster_address = GetClusterAddress(offset_in_file & cluster_offset_mask);
		return Get<T>(cluster_address, offset_in_file & ~cluster_offset_mask);
	}
	template<class T>
	void Set(uint64 offset_in_file, const T& obj) {
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
	bool CheckConsistency();


	//// static metadata management ////
private:
	StaticMetadata& GetStaticMetadata() const { return *(StaticMetadata*)GetClusterAddress(0); }
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
	void SortFreeBlockList() {}		  /* to implement */
	void RecycleFreeBlockList() {}	  /* to implement */


	//// L4096Plus cluster hierarchy management ////
private:
	class L4096PlusClusterIterator {
	private:
		EngineImpl& engine;
		IndexEntry entry;
	private:
		struct ClusterIndexLevelStatus {
			uint64 cluster_number = 0;
			mutable uint64 current_cluster_logic_index = (uint64)-1;
			mutable uint64 current_cluster_offset = (uint64)-1;
		};
		ClusterIndexLevelStatus cluster_level_stack[max_cluster_hierarchy_depth];
		uint64 stack_level_count;
	public:
		L4096PlusClusterIterator(EngineImpl& engine, IndexEntry entry);
	private:
		void ExpandToSizeOfLevel(uint64 level, uint64 new_cluster_number);
		void ShrinkToSizeOfLevel(uint64 level, uint64 new_cluster_number);
	private:
		void ExpandToSize(uint64 new_size);
		void ShrinkToSize(uint64 new_size);
	public:
		void Resize(uint64 new_size);
		IndexEntry GetEntry() const { return entry; }
	private:
		void SeekToClusterOfLevel(uint64 level, uint64 cluster_logic_index) const;
	public:
		void SeekToCluster(uint64 cluster_logic_index) const {
			SeekToClusterOfLevel(0, cluster_logic_index);
		}
	public:
		uint64 GetCurrentClusterOffset() const {
			assert(cluster_level_stack[0].current_cluster_logic_index != -1);
			return cluster_level_stack[0].current_cluster_offset;
		}
		void* GetCurrentClusterAddress() const {
			return engine.GetClusterAddress(GetCurrentClusterOffset());
		}
	};


	//// indexed arrays management ////
private:
	uint64 GetIndexEntryOffset(ArrayIndex index) const;
	IndexEntry GetIndexEntry(ArrayIndex index) const;
	void SetIndexEntry(ArrayIndex index, IndexEntry entry);

	ArrayIndex InitializeIndexEntry(ArrayIndex index_begin, IndexEntry* index_entry_begin, uint64 index_entry_number);
	ArrayIndex InitializeIndexEntry(ArrayIndex index_begin, ArrayIndex index_end);
	ArrayIndex ExtendIndexTable();
	ArrayIndex AllocateIndex();
	void DeallocateIndex(ArrayIndex index);

	IndexEntry ResizeL4096PlusIndexEntry(IndexEntry entry, uint64 new_size);
	IndexEntry ResizeIndexEntry(IndexEntry entry, uint64 new_size);

private:
	bool IsIndexValid(ArrayIndex index) const {
		return index.value < GetStaticMetadata().index_table_entry.array_size / index_entry_size;
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