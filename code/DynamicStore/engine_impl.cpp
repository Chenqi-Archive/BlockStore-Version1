#include "engine_impl.h"


BEGIN_NAMESPACE(DynamicStore)


DYNAMICSTORE_API std::unique_ptr<Engine> Engine::Create(const wchar file[]) {
	return std::make_unique<EngineImpl>(file);
}


bool EngineImpl::LoadAndCheck() {
	if (_size == 0 || _size % cluster_size != 0) { return false; }

	_static_metadata = *(StaticMetadata*)GetClusterAddress(0);
	if (_static_metadata.file_size != _size) { return false; }
	if (uint64 size = _static_metadata.index_table_entry.array_size; size > cluster_size ? size % cluster_size != 0 : block_size_table[(uint64)GetBlockType(size)] != size) { return false; }
	if (_static_metadata.user_metadata_size > max_user_metadata_size) { return false; }

	return true;
}

void EngineImpl::Format() {
	// Set file size to 4k.
	SetSize(cluster_size);

	// Initialize metadata.
	_static_metadata.file_size = _size;
	_static_metadata.index_table_entry.array_size = 8;
	_static_metadata.index_table_entry.data = 0;
	_static_metadata.free_index_head = free_index_tail;
	for (uint64 block_type = 1; block_type < block_type_number - 1; ++block_type) {
		_static_metadata.free_block_head[block_type] = free_block_tail;
	}
	_static_metadata.free_cluster_head = free_block_tail;
	_static_metadata.user_metadata_size = 0;

	// Divide the remaining space of the first cluster to specified blocks below:
	// 		| 0               | 256             | 512             | 768             |
	//    0 | static metadata |    L16 * 16     |             L32 * 16				|
	// 1024 |                               L64 * 16								|
	// 2048 |                               L128 * 8								|
	// 3072 |                               L256 * 4								|
	static_assert(cluster_size == 4096);
	InitializeClusterSection(BlockType::L16, 0, 256, 512);
	InitializeClusterSection(BlockType::L32, 0, 512, 1024);
	InitializeClusterSection(BlockType::L64, 0, 1024, 2048);
	InitializeClusterSection(BlockType::L128, 0, 2048, 3072);
	InitializeClusterSection(BlockType::L256, 0, 3072, 4096);
}

void EngineImpl::LoadUserMetadata(void* data, uint64 size) const {
	assert(size <= max_user_metadata_size);
	memcpy(data, _static_metadata.user_metadata, size);
}

void EngineImpl::StoreUserMetadata(const void* data, uint64 size) {
	assert(size <= max_user_metadata_size);
	memcpy(_static_metadata.user_metadata, data, size);
}

uint64 EngineImpl::InitializeClusterSection(BlockType block_type, uint64 cluster_offset, uint64 begin_offset, uint64 end_offset) {
	assert(block_type > BlockType::L8 && block_type < BlockType::L4096Plus);
	uint64 block_size = block_size_table[(uint64)block_type];
	void* cluster_address = GetClusterAddress(cluster_offset);
	uint64 next_free_block = _static_metadata.free_block_head[(uint64)block_type];
	for (uint64 block_offset = end_offset - block_size;
		 block_offset >= begin_offset && block_offset < end_offset;
		 block_offset -= block_size) {
		Set<uint64>(cluster_address, block_offset, next_free_block);
		next_free_block = block_offset;
	}
	return _static_metadata.free_block_head[(uint64)block_type] = next_free_block;
}

uint64 EngineImpl::InitializeCluster(BlockType block_type, uint64 cluster_offset) {
	return InitializeClusterSection(block_type, cluster_offset, 0, cluster_size);
}

uint64 EngineImpl::ExtendFileByOneCluster() {
	uint64 old_size = _static_metadata.file_size;
	SetSize(old_size + cluster_size);
	return old_size;
}

uint64 EngineImpl::AllocateBlock(BlockType block_type) {
	assert(block_type > BlockType::L8 && block_type < BlockType::L4096Plus);
	uint64 current_free_block = _static_metadata.free_block_head[(uint64)block_type];
	if (current_free_block == free_block_tail) {
		uint64 cluster_offset = ExtendFileByOneCluster();
		current_free_block = InitializeCluster(block_type, cluster_offset);
	}
	assert(current_free_block % block_size_table[(uint64)block_type] == 0);
	uint64 next_free_block = Get<uint64>(current_free_block);
	_static_metadata.free_block_head[(uint64)block_type] = next_free_block;
	return current_free_block;
}

void EngineImpl::DeallocateBlock(BlockType block_type, uint64 block_offset) {
	assert(block_type > BlockType::L8 && block_type < BlockType::L4096Plus);
	assert(block_offset % block_size_table[(uint64)block_type] == 0);
	uint64 next_free_block = _static_metadata.free_block_head[(uint64)block_type];
	Set<uint64>(block_offset, next_free_block);
	_static_metadata.free_block_head[(uint64)block_type] = block_offset;
}

void EngineImpl::SortFreeBlockList() {
	// to implement (put the algorithm in a seperate header file)
}

void EngineImpl::RecycleFreeBlockList() {

}


EngineImpl::L4096PlusClusterIterator::L4096PlusClusterIterator(EngineImpl& engine, IndexEntry entry) :
	engine(engine), entry(entry), stack_top_level(-1), current_offset_in_array(-1) {
	assert(entry.array_size > cluster_size);
	uint64 cluster_number = GetClusterNumber(entry.array_size);
	while (cluster_number > 1) {
		stack_top_level = stack_top_level + 1; assert(stack_top_level < max_cluster_hierarchy_depth);
		cluster_level_stack[stack_top_level].cluster_number = cluster_number;
		cluster_level_stack[stack_top_level].current_cluster_logic_index = -1;
		cluster_number = GetClusterNumber(cluster_number * cluster_index_size);
	}
}

void EngineImpl::L4096PlusClusterIterator::SeekToCluster(uint64 offset_in_array) {
	assert(offset_in_array % cluster_size == 0 && offset_in_array < entry.array_size);
	// For the first time: bottom-up update logic index until remains unchanged in some level.
	uint64 level_to_update = 0;
	for (; level_to_update <= stack_top_level; ++level_to_update) {
		uint64 current_cluster_logic_index = GetClusterLogicIndexOfLevel(offset_in_array, level_to_update);
		assert(current_cluster_logic_index < cluster_level_stack[level_to_update].cluster_number);
		if (cluster_level_stack[level_to_update].current_cluster_logic_index == current_cluster_logic_index) {
			break;
		}
		cluster_level_stack[level_to_update].current_cluster_logic_index = current_cluster_logic_index;
	}
	// For the second time: top-down update offset from the unchanged level.
	uint64 parent_level_block_offset = level_to_update == stack_top_level ?
		entry.offset :
		cluster_level_stack[level_to_update + 1].current_cluster_offset;
	for (uint64 current_level = level_to_update; current_level <= level_to_update; --current_level) {
		uint64 current_cluster_logic_index = cluster_level_stack[current_level].current_cluster_logic_index;
		uint64 current_cluster_index_on_parent = current_cluster_logic_index & cluster_index_mask;
		uint64 current_cluster_index_offset = parent_level_block_offset + current_cluster_index_on_parent * cluster_index_size;
		uint64 current_cluster_offset = engine.Get<uint64>(current_cluster_index_offset);
		cluster_level_stack[current_level].current_cluster_offset = current_cluster_offset;
		parent_level_block_offset = current_cluster_offset;
	}
	current_offset_in_array = offset_in_array;
}


uint64 EngineImpl::AllocateClusterIndexBlock(uint64 first_cluster_offset, uint64 size) {

}

void EngineImpl::DeallocateClusterIndexBlock(uint64 cluster_index_block_offset, uint64 current_allocated_size) {

}

uint64 EngineImpl::GetIndexEntryOffset(ArrayIndex index) const {
	IndexEntry index_table_entry = _static_metadata.index_table_entry;
	uint64 entry_offset = index.value * index_entry_size;
	if (entry_offset >= index_table_entry.array_size) { throw std::invalid_argument("invalid array index"); }

	/* index entry table can be cached */

	BlockType type = GetBlockType(index_table_entry.array_size);
	assert(type > BlockType::L8);
	if (type < BlockType::L4096) {
		return index_table_entry.offset + entry_offset;
	} else {

	}
}

IndexEntry EngineImpl::GetIndexEntry(ArrayIndex index) const {
	assert(!index.IsInvalid());
	uint64 entry_offset = GetIndexEntryOffset(index);
	return Get<IndexEntry>(entry_offset);
}

void EngineImpl::SetIndexEntry(ArrayIndex index, IndexEntry entry) {
	assert(!index.IsInvalid());
	uint64 entry_offset = GetIndexEntryOffset(index);
	Set<IndexEntry>(entry_offset, entry);
}

inline void EngineImpl::InitializeIndexEntry(uint64 index_entry_offset_begin, uint64 index_entry_offset_end) {
	assert(GetClusterNumber(index_entry_offset_begin) == GetClusterNumber(index_entry_offset_end - 1));

	// convert to offset in file

	void* cluster_address = GetClusterAddress(index_entry_offset_begin);
	ArrayIndex next_free_index = _static_metadata.free_index_head;
	for (uint64 entry_offset = index_entry_offset_end - index_entry_size;
		 entry_offset >= index_entry_offset_begin && entry_offset < index_entry_offset_end;
		 entry_offset -= index_entry_size) {
		IndexEntry entry = {};



		Set<IndexEntry>(cluster_address, entry_offset, entry);
		next_free_index = block_offset;
	}
}

inline void EngineImpl::ExtendIndexTable() {
	IndexEntry index_table_entry = _static_metadata.index_table_entry;
	uint64 old_size = index_table_entry.array_size;
	// Old size must be a multiple of cluster_size, or a power of 2.
	assert(old_size > cluster_size ? old_size % cluster_size == 0 : block_size_table[(uint64)GetBlockType(old_size)] == old_size);
	uint64 size_to_extend = std::min(old_size, cluster_size);
	uint64 new_size = old_size + size_to_extend;
	index_table_entry = ResizeIndexEntry(index_table_entry, new_size);
	_static_metadata.index_table_entry = index_table_entry;
	InitializeIndexEntry(old_size, new_size);
}

inline ArrayIndex EngineImpl::AllocateIndex() {
	ArrayIndex current_free_index = _static_metadata.free_index_head;
	if (current_free_index.value == free_index_tail.value) {
		ExtendIndexTable();
	}
	ArrayIndex next_free_index;
}

inline void EngineImpl::DeallocateIndex(ArrayIndex index) {

}

IndexEntry EngineImpl::UpgradeL4096PlusIndexEntry(IndexEntry entry_with_new_size) {
	assert(entry_with_new_size.offset != 0 && entry_with_new_size.offset % cluster_size == 0);

}

IndexEntry EngineImpl::DowngradeL4096PlusIndexEntry(uint64 old_offset, uint64 old_size) {

}

IndexEntry EngineImpl::ResizeL4096PlusIndexEntry(uint64 old_offset, uint64 old_size, uint64 new_size) {
	if (GetClusterNumber(old_size) == GetClusterNumber(new_size)) {
		return { new_size, old_offset };
	}



}

IndexEntry EngineImpl::ResizeIndexEntry(IndexEntry entry, uint64 new_size) {
	uint64 old_size = entry.array_size;
	assert(old_size != free_entry_array_size);
	assert(new_size != free_entry_array_size);

	BlockType old_type = GetBlockType(old_size);
	BlockType new_type = GetBlockType(new_size);

	entry.array_size = new_size;

	if (old_type == new_type) {
		if (old_type != BlockType::L4096Plus) {
			return entry;
		} else {
			return ResizeL4096PlusIndexEntry(entry.offset, old_size, new_size);
		}
	}

	BlockType from_type = old_type;
	BlockType to_type = new_type;

	if (old_type == BlockType::L4096Plus) {
		entry = DowngradeL4096PlusIndexEntry(entry);
		from_type = BlockType::L4096;
	}

	if (new_type == BlockType::L4096Plus) {
		to_type = BlockType::L4096;
	}

	if (from_type != to_type) {
		if (to_type == BlockType::L8) {
			entry.data = Get<uint64>(entry.offset);
		} else {
			uint64 destination_data_offset = AllocateBlock(to_type);
			if (from_type == BlockType::L8) {
				Set<uint64>(destination_data_offset, entry.data);
			} else {
				MoveData(entry.offset, destination_data_offset, std::min(old_size, new_size));
			}
			entry.offset = destination_data_offset;
		}
	}

	if (new_type == BlockType::L4096Plus) {
		entry = UpgradeL4096PlusIndexEntry(entry);
	}

	return entry;
}

ArrayIndex EngineImpl::CreateArray() {
	return AllocateIndex();
}

void EngineImpl::DestroyArray(ArrayIndex index) {
	DeallocateIndex(index);
}

uint64 EngineImpl::GetArraySize(ArrayIndex index) const { 
	return GetIndexEntry(index).array_size;
}

void EngineImpl::SetArraySize(ArrayIndex index, uint64 size) {
	ResizeIndexEntry(GetIndexEntry(index), size);
}

void EngineImpl::ReadArray(ArrayIndex index, uint64 offset, uint64 size, void* data) const {

}

void EngineImpl::WriteArray(ArrayIndex index, const void* data, uint64 size, uint64 offset) {

}


END_NAMESPACE(DynamicStore)