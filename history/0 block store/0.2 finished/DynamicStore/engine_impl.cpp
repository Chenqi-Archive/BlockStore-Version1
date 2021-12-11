#include "engine_impl.h"


#ifdef _DEBUG
#pragma comment(lib, "../build/x64/Debug/BlockStoreCore.lib")
#else
#pragma comment(lib, "../build/x64/Release/BlockStoreCore.lib")
#endif // _DEBUG


BEGIN_NAMESPACE(DynamicStore)


DYNAMICSTORE_API std::unique_ptr<Engine> Engine::Create(const wchar file[]) {
	return std::make_unique<EngineImpl>(file);
}


EngineImpl::EngineImpl(const wchar file[]) : _file(file) {
	if (_file.GetSize() == 0) {
		Format();
	} else {
		_file.DoMapping();
		UpdateStaticMetadataAddress();
		if (LoadAndCheck() == false) {
			Format();
		}
	}
}

EngineImpl::~EngineImpl() {
	// Trim the size reserved but not used.
	uint64 file_size = _file.GetSize();
	if (file_size > _static_metadata->used_size) {
		_file.SetSize(_static_metadata->used_size);
	}
}

bool EngineImpl::LoadAndCheck() {
	uint64 file_size = _file.GetSize();
	assert(file_size != 0);
	if (file_size % cluster_size != 0) { return false; }

	if (_static_metadata->used_size > file_size) { return false; }

	if (true) {
		uint64 index_table_size = _static_metadata->index_table_entry.array_size;
		if (index_table_size > cluster_size) {
			if (index_table_size % cluster_size != 0) { return false; }
		} else {
			if (block_size_table[(uint64)GetBlockType(index_table_size)] != index_table_size) { return false; }
		}
	}
	
	if (_static_metadata->user_metadata_size > max_user_metadata_size) { return false; }

	assert(CheckConsistency() == true);

	return true;
}

void EngineImpl::Format() {
	// Set file size to 4k.
	_file.SetSize(cluster_size);
	_file.DoMapping();
	UpdateStaticMetadataAddress();

	// Initialize metadata.
	_static_metadata->index_table_entry.array_size = 8;
	_static_metadata->index_table_entry.data = 0;
	_static_metadata->free_index_head = free_index_tail;
	for (uint64 block_type = 1; block_type < block_type_number - 1; ++block_type) {
		_static_metadata->free_block_head[block_type] = free_block_tail;
	}
	_static_metadata->free_cluster_head = free_cluster_tail;
	_static_metadata->user_metadata_size = 0;

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

	// Set used size to 4k.
	_static_metadata->used_size = cluster_size;
}

bool EngineImpl::CheckConsistency() {

	/* to implement */

	return true;
}

void EngineImpl::LoadUserMetadata(void* data, uint64 size) const {
	assert(size <= max_user_metadata_size);
	memcpy(data, _static_metadata->user_metadata, std::min(_static_metadata->user_metadata_size, size));
}

void EngineImpl::StoreUserMetadata(const void* data, uint64 size) {
	assert(size <= max_user_metadata_size);
	_static_metadata->user_metadata_size = size;
	memcpy(_static_metadata->user_metadata, data, size);
}

void EngineImpl::ReserveMoreCluster(uint64 cluster_number) {
	uint64 old_size = _file.GetSize();
	assert(old_size >= _static_metadata->used_size && old_size % cluster_size == 0);
	_file.SetSize(old_size + cluster_size * cluster_number); 
	_file.DoMapping();
	UpdateStaticMetadataAddress();
}

uint64 EngineImpl::AllocateCluster() {
	uint64 file_size = _file.GetSize();
	uint64 current_free_cluster = _static_metadata->free_cluster_head;
	if (current_free_cluster == free_cluster_tail) {
		assert(_static_metadata->used_size <= file_size);
		if (_static_metadata->used_size == file_size) {
			ReserveMoreCluster(1);
		}
		current_free_cluster = _static_metadata->used_size;
		_static_metadata->used_size += cluster_size;
	} else {
		uint64 next_free_cluster = Get<uint64>(current_free_cluster);
		_static_metadata->free_cluster_head = next_free_cluster;
	}
	return current_free_cluster;
}

void EngineImpl::DeallocateCluster(uint64 cluster_offset) {
	assert(cluster_offset % cluster_size == 0);
	uint64 next_free_cluster = _static_metadata->free_cluster_head;
	Set<uint64>(cluster_offset, next_free_cluster);
	_static_metadata->free_cluster_head = cluster_offset;
}

uint64 EngineImpl::InitializeClusterSection(BlockType block_type, uint64 cluster_offset, uint64 begin_offset, uint64 end_offset) {
	assert(block_type > BlockType::L8 && block_type < BlockType::L4096);
	uint64 block_size = block_size_table[(uint64)block_type];
	uint64 next_free_block = _static_metadata->free_block_head[(uint64)block_type];
	void* cluster_address = GetClusterAddress(cluster_offset);
	for (uint64 block_offset = end_offset - block_size; block_offset >= begin_offset && block_offset < end_offset; block_offset -= block_size) {
		Set<uint64>(cluster_address, block_offset, next_free_block);
		next_free_block = cluster_offset + block_offset;
	}
	return _static_metadata->free_block_head[(uint64)block_type] = next_free_block;
}

uint64 EngineImpl::InitializeCluster(BlockType block_type, uint64 cluster_offset) {
	return InitializeClusterSection(block_type, cluster_offset, 0, cluster_size);
}

uint64 EngineImpl::AllocateBlock(BlockType block_type) {
	assert(block_type > BlockType::L8 && block_type < BlockType::L4096Plus);
	if (block_type == BlockType::L4096) {
		return AllocateCluster();
	} else {
		uint64 current_free_block = _static_metadata->free_block_head[(uint64)block_type];
		if (current_free_block == free_block_tail) {
			uint64 cluster_offset = AllocateCluster();
			current_free_block = InitializeCluster(block_type, cluster_offset);
		}
		assert(current_free_block % block_size_table[(uint64)block_type] == 0);
		uint64 next_free_block = Get<uint64>(current_free_block);
		_static_metadata->free_block_head[(uint64)block_type] = next_free_block;
		return current_free_block;
	}
}

void EngineImpl::DeallocateBlock(BlockType block_type, uint64 block_offset) {
	assert(block_type > BlockType::L8 && block_type < BlockType::L4096Plus);
	assert(block_offset % block_size_table[(uint64)block_type] == 0);
	uint64 next_free_block = _static_metadata->free_block_head[(uint64)block_type];
	Set<uint64>(block_offset, next_free_block);
	_static_metadata->free_block_head[(uint64)block_type] = block_offset;
}


EngineImpl::L4096PlusClusterIterator::L4096PlusClusterIterator(EngineImpl& engine, IndexEntry entry) :
	engine(engine), entry(entry), stack_level_count(0) {
	assert(entry.array_size >= cluster_size);
	uint64 cluster_number = GetClusterNumber(entry.array_size);
	while (cluster_number > 1) {
		assert(stack_level_count < max_cluster_hierarchy_depth);
		cluster_level_stack[stack_level_count].cluster_number = cluster_number;
		stack_level_count = stack_level_count + 1;
		cluster_number = GetClusterNumber(cluster_number * cluster_index_size);
	}
}

void EngineImpl::L4096PlusClusterIterator::ExpandToSizeOfLevel(uint64 level, uint64 new_cluster_number) {
	assert(level < stack_level_count);
	assert(cluster_level_stack[level].cluster_number < new_cluster_number);
	engine.ReserveMoreCluster(new_cluster_number - cluster_level_stack[level].cluster_number);
	if (level == stack_level_count - 1) {
		assert(new_cluster_number <= cluster_index_number);
		void* cluster_address = engine.GetClusterAddress(entry.offset & cluster_offset_mask);
		uint64* root_cluster_index = (uint64*)((char*)cluster_address + (entry.offset & ~cluster_offset_mask));
		for (uint64 current_cluster = cluster_level_stack[level].cluster_number; current_cluster < new_cluster_number; ++current_cluster) {
			root_cluster_index[current_cluster] = engine.AllocateCluster();
		}
	} else {
		uint64 current_cluster_logic_index = cluster_level_stack[level].cluster_number;
		do {
			SeekToClusterOfLevel(level + 1, current_cluster_logic_index >> cluster_index_bits);
			uint64* cluster_address = (uint64*)engine.GetClusterAddress(cluster_level_stack[level + 1].current_cluster_offset);
			uint64 current_cluster_index = current_cluster_logic_index & cluster_index_mask;
			for (; current_cluster_index < cluster_index_number && current_cluster_logic_index < new_cluster_number;
				 ++current_cluster_index, ++current_cluster_logic_index) {
				cluster_address[current_cluster_index] = engine.AllocateCluster();
			}
		} while (current_cluster_logic_index < new_cluster_number);
	}
	cluster_level_stack[level].cluster_number = new_cluster_number;
}

void EngineImpl::L4096PlusClusterIterator::ShrinkToSizeOfLevel(uint64 level, uint64 new_cluster_number) {
	assert(level < stack_level_count);
	assert(cluster_level_stack[level].cluster_number > new_cluster_number);
	if (level == stack_level_count - 1) {
		assert(new_cluster_number <= cluster_index_number);
		void* cluster_address = engine.GetClusterAddress(entry.offset & cluster_offset_mask);
		uint64* root_cluster_index = (uint64*)((char*)cluster_address + (entry.offset & ~cluster_offset_mask));
		for (uint64 current_cluster = new_cluster_number; current_cluster < cluster_level_stack[level].cluster_number; ++current_cluster) {
			engine.DeallocateCluster(root_cluster_index[current_cluster]);
		}
	} else {
		uint64 current_cluster_logic_index = new_cluster_number;
		do {
			SeekToClusterOfLevel(level + 1, current_cluster_logic_index >> cluster_index_bits);
			uint64* cluster_address = (uint64*)engine.GetClusterAddress(cluster_level_stack[level + 1].current_cluster_offset);
			uint64 current_cluster_index = current_cluster_logic_index & cluster_index_mask;
			for (; current_cluster_index < cluster_index_number && current_cluster_logic_index < cluster_level_stack[level].cluster_number;
				 ++current_cluster_index, ++current_cluster_logic_index) {
				engine.DeallocateCluster(cluster_address[current_cluster_index]);
			}
		} while (current_cluster_logic_index < cluster_level_stack[level].cluster_number);
	}
	cluster_level_stack[level].cluster_number = new_cluster_number;
}

void EngineImpl::L4096PlusClusterIterator::ExpandToSize(uint64 new_size) {
	uint64 new_cluster_level_number[max_cluster_hierarchy_depth];
	uint64 new_level_count = 0;
	uint64 cluster_number = GetClusterNumber(new_size);
	while (cluster_number > 1) {
		assert(new_level_count < max_cluster_hierarchy_depth);
		assert(cluster_level_stack[new_level_count].cluster_number <= cluster_number);
		if (cluster_level_stack[new_level_count].cluster_number == cluster_number) {
			break;
		}
		new_cluster_level_number[new_level_count] = cluster_number;
		new_level_count = new_level_count + 1;
		cluster_number = GetClusterNumber(cluster_number * cluster_index_size);
	}
	assert(new_level_count > 0 && new_level_count <= max_cluster_hierarchy_depth);
	if (new_level_count < stack_level_count) {
	} else if (new_level_count == stack_level_count) {
		// Reallocate root index block.
		uint64 old_size = cluster_level_stack[stack_level_count - 1].cluster_number * cluster_index_size;
		uint64 new_size = new_cluster_level_number[new_level_count - 1] * cluster_index_size;
		BlockType old_type = GetBlockType(old_size);
		BlockType new_type = GetBlockType(new_size);
		assert(new_type >= old_type);
		assert(new_type > BlockType::L8 && new_type < BlockType::L4096Plus);
		if (old_type != new_type) {
			uint64 new_offset = engine.AllocateBlock(new_type);
			engine.MoveData(entry.offset, new_offset, std::min(old_size, new_size));
			engine.DeallocateBlock(old_type, entry.offset);
			entry.offset = new_offset;
		}
	} else { // new_level_count > stack_level_count
		// Upgrade old root index block to 4k.
		uint64 old_size;
		if (stack_level_count == 0) {
			assert(entry.array_size == cluster_size);
			old_size = entry.array_size;
		} else {
			old_size = cluster_level_stack[stack_level_count - 1].cluster_number * cluster_index_size;
		}
		BlockType old_type = GetBlockType(old_size);
		uint64 prev_root_offset = entry.offset;
		if (old_type != BlockType::L4096) {
			uint64 new_offset = engine.AllocateCluster();
			engine.MoveData(prev_root_offset, new_offset, std::min(old_size, new_size));
			engine.DeallocateBlock(old_type, prev_root_offset);
			prev_root_offset = new_offset;
		}
		// Allocate 4k clusters from stack_level_count to new_level_count - 2.
		uint64 current_level = stack_level_count;
		for (; current_level < new_level_count - 1; current_level++) {
			uint64 current_root_offset = engine.AllocateCluster();
			engine.Set<uint64>(current_root_offset, prev_root_offset);
			prev_root_offset = current_root_offset;
			cluster_level_stack[current_level].cluster_number = 1;
		}
		// Allocate size dependent block for new_level_count - 1.
		assert(current_level == new_level_count - 1);
		uint64 new_size = new_cluster_level_number[current_level] * cluster_index_size;
		BlockType new_type = GetBlockType(new_size);
		assert(new_type > BlockType::L8 && new_type < BlockType::L4096Plus);
		uint64 current_root_offset = engine.AllocateBlock(new_type);
		engine.Set<uint64>(current_root_offset, prev_root_offset);
		entry.offset = current_root_offset;
		cluster_level_stack[current_level].cluster_number = 1;
		stack_level_count = new_level_count;
	}
	// Allocate clusters for all levels in reverse order.
	for (uint64 current_level = new_level_count - 1; current_level < new_level_count; --current_level) {
		ExpandToSizeOfLevel(current_level, new_cluster_level_number[current_level]);
	}
	entry.array_size = new_size;
}

void EngineImpl::L4096PlusClusterIterator::ShrinkToSize(uint64 new_size) {
	assert(stack_level_count > 0 && stack_level_count <= max_cluster_hierarchy_depth);
	uint64 new_cluster_level_number[max_cluster_hierarchy_depth];
	uint64 new_level_count = 0;
	uint64 cluster_number = GetClusterNumber(new_size);
	while (cluster_number > 1) {
		assert(new_level_count < max_cluster_hierarchy_depth);
		assert(cluster_level_stack[new_level_count].cluster_number >= cluster_number);
		if (cluster_level_stack[new_level_count].cluster_number == cluster_number) {
			break;
		}
		new_cluster_level_number[new_level_count] = cluster_number;
		new_level_count = new_level_count + 1;
		cluster_number = GetClusterNumber(cluster_number * cluster_index_size);
	}
	assert(new_level_count <= stack_level_count);
	uint64 old_size = cluster_level_stack[stack_level_count - 1].cluster_number * cluster_index_size;  // old root block size
	entry.array_size = new_size;
	// Deallocate clusters from level 0 to new_level_count - 1 in normal order.
	for (uint64 current_level = 0; current_level < new_level_count; ++current_level) {
		ShrinkToSizeOfLevel(current_level, new_cluster_level_number[current_level]);
	}
	if (new_level_count < stack_level_count && cluster_number > 1) {
	} else if(new_level_count == stack_level_count) {
		// Reallocate root index block.
		uint64 new_size = new_cluster_level_number[new_level_count - 1] * cluster_index_size;
		BlockType old_type = GetBlockType(old_size);
		BlockType new_type = GetBlockType(new_size);
		assert(new_type <= old_type);
		assert(new_type > BlockType::L8 && new_type < BlockType::L4096Plus);
		if (old_type != new_type) {
			uint64 new_offset = engine.AllocateBlock(new_type);
			engine.MoveData(entry.offset, new_offset, std::min(old_size, new_size));
			engine.DeallocateBlock(old_type, entry.offset);
			entry.offset = new_offset;
		}
	} else { // new_level_count < stack_level_count && cluster_number == 1
		// Deallocate clusters from new_level_count to stack_level_count - 1.
		for (uint64 current_level = new_level_count; current_level < stack_level_count; ++current_level) {
			ShrinkToSizeOfLevel(current_level, 1);
		}
		// Deallocate size dependent block for stack_level_count.
		uint64 current_root_offset = engine.Get<uint64>(entry.offset);
		BlockType old_type = GetBlockType(old_size);
		assert(old_type > BlockType::L8 && old_type < BlockType::L4096Plus);
		engine.DeallocateBlock(old_type, entry.offset);
		// Deallocate 4k clusters from stack_level_count - 1 to new_level_count + 1.
		for (uint64 current_level = stack_level_count - 1; current_level > new_level_count; --current_level) {
			uint64 next_root_offset = engine.Get<uint64>(current_root_offset);
			engine.DeallocateCluster(current_root_offset);
			current_root_offset = next_root_offset;
			cluster_level_stack[current_level].cluster_number = 0;
		}
		// Downgrade new root index block depending on its size.
		uint64 new_size;
		if (new_level_count == 0) {
			assert(entry.array_size == cluster_size);
			new_size = entry.array_size;
		} else {
			new_size = cluster_level_stack[new_level_count - 1].cluster_number * cluster_index_size;
		}
		BlockType new_type = GetBlockType(new_size);
		if (new_type != BlockType::L4096) {
			uint64 new_offset = engine.AllocateBlock(new_type);
			engine.MoveData(new_offset, current_root_offset, new_size);
			engine.DeallocateCluster(current_root_offset);
			entry.offset = new_offset;
		} else {
			entry.offset = current_root_offset;
		}
		stack_level_count = new_level_count;
	}
}

void EngineImpl::L4096PlusClusterIterator::Resize(uint64 new_size) {
	assert(new_size >= cluster_size);
	// Invalidate current cluster logic indexes.
	for (uint64 current_level = 0; current_level < max_cluster_hierarchy_depth; ++current_level) {
		cluster_level_stack[current_level].current_cluster_logic_index = (uint64)-1;
	}
	if (new_size > entry.array_size) { ExpandToSize(new_size); return; }
	if (new_size < entry.array_size) { ShrinkToSize(new_size); return; }
}

void EngineImpl::L4096PlusClusterIterator::SeekToClusterOfLevel(uint64 level, uint64 cluster_logic_index) const {
	assert(level < stack_level_count);
	// For the first time: bottom-up update logic index until remains unchanged in some level.
	uint64 level_to_update = level;
	for (; level_to_update < stack_level_count; ++level_to_update) {
		assert(cluster_logic_index < cluster_level_stack[level_to_update].cluster_number);
		if (cluster_level_stack[level_to_update].current_cluster_logic_index == cluster_logic_index) {
			break;
		}
		cluster_level_stack[level_to_update].current_cluster_logic_index = cluster_logic_index;
		cluster_logic_index >>= cluster_index_bits;
	}
	// For the second time: top-down update offset from the unchanged level.
	uint64 parent_level_block_offset = level_to_update == stack_level_count ?
		entry.offset :
		cluster_level_stack[level_to_update].current_cluster_offset;
	for (uint64 current_level = level_to_update - 1; current_level >= level && current_level < level_to_update; --current_level) {
		uint64 current_cluster_logic_index = cluster_level_stack[current_level].current_cluster_logic_index;
		uint64 current_cluster_index_on_parent = current_cluster_logic_index & cluster_index_mask;
		uint64 current_cluster_index_offset = parent_level_block_offset + current_cluster_index_on_parent * cluster_index_size;
		uint64 current_cluster_offset = engine.Get<uint64>(current_cluster_index_offset);
		cluster_level_stack[current_level].current_cluster_offset = current_cluster_offset;
		parent_level_block_offset = current_cluster_offset;
	}
}

uint64 EngineImpl::GetIndexEntryOffset(ArrayIndex index) const {
	IndexEntry index_table_entry = _static_metadata->index_table_entry;
	uint64 index_entry_offset_in_table = index.value * index_entry_size;
	assert(index_entry_offset_in_table < index_table_entry.array_size);
	BlockType type = GetBlockType(index_table_entry.array_size);
	assert(type > BlockType::L8);
	if (type < BlockType::L4096Plus) {
		return index_table_entry.offset + index_entry_offset_in_table;
	} else {
		const L4096PlusClusterIterator iterator(const_cast<EngineImpl&>(*this), index_table_entry);
		iterator.SeekToCluster(index_entry_offset_in_table >> cluster_size_bits);
		return iterator.GetCurrentClusterOffset() + (index_entry_offset_in_table & ~cluster_offset_mask);
	}
}

IndexEntry EngineImpl::GetIndexEntry(ArrayIndex index) const {
	uint64 entry_offset = GetIndexEntryOffset(index);
	IndexEntry entry = Get<IndexEntry>(entry_offset);
	return entry;
}

void EngineImpl::SetIndexEntry(ArrayIndex index, IndexEntry entry) {
	uint64 entry_offset = GetIndexEntryOffset(index);
	Set<IndexEntry>(entry_offset, entry);
}

ArrayIndex EngineImpl::InitializeIndexEntry(ArrayIndex index_begin, IndexEntry* index_entry_begin, uint64 index_entry_number) {
	IndexEntry* index_entry_end = index_entry_begin + index_entry_number;
	ArrayIndex next_free_index = _static_metadata->free_index_head;
	for (IndexEntry* current_index_entry = index_entry_end - 1; current_index_entry >= index_entry_begin; --current_index_entry) {
		current_index_entry->array_size = free_entry_array_size;
		current_index_entry->next_free_index = next_free_index;
		next_free_index = ArrayIndex(index_begin.value + current_index_entry - index_entry_begin);
	}
	return _static_metadata->free_index_head = next_free_index;
}

ArrayIndex EngineImpl::InitializeIndexEntry(ArrayIndex index_begin, ArrayIndex index_end){
	uint64 index_entry_begin_offset = GetIndexEntryOffset(index_begin);
	void* cluster_address = GetClusterAddress(index_entry_begin_offset & cluster_offset_mask);
	IndexEntry* index_entry_begin = (IndexEntry*)((char*)cluster_address + (index_entry_begin_offset & ~cluster_offset_mask));
	return InitializeIndexEntry(index_begin, index_entry_begin, index_end.value - index_begin.value);
}

ArrayIndex EngineImpl::ExtendIndexTable() {
	IndexEntry index_table_entry = _static_metadata->index_table_entry;
	uint64 old_size = index_table_entry.array_size;
	assert(old_size > cluster_size ? old_size % cluster_size == 0 : block_size_table[(uint64)GetBlockType(old_size)] == old_size);
	uint64 size_to_extend = std::min(old_size, cluster_size);
	uint64 new_size = old_size + size_to_extend;
	index_table_entry = ResizeIndexEntry(index_table_entry, new_size);
	_static_metadata->index_table_entry = index_table_entry;
	return InitializeIndexEntry(ArrayIndex(old_size / index_entry_size), ArrayIndex(new_size / index_entry_size));
}

ArrayIndex EngineImpl::AllocateIndex() {
	ArrayIndex current_free_index = _static_metadata->free_index_head;
	if (current_free_index.value == free_index_tail.value) {
		current_free_index = ExtendIndexTable();
	}
	IndexEntry entry = GetIndexEntry(current_free_index);
	_static_metadata->free_index_head = entry.next_free_index;
	entry.array_size = 0; entry.data = 0;
	SetIndexEntry(current_free_index, entry);
	return current_free_index;
}

void EngineImpl::DeallocateIndex(ArrayIndex index) {
	IndexEntry entry = GetIndexEntry(index);
	entry = ResizeIndexEntry(entry, 0);
	InitializeIndexEntry(index, &entry, 1);
	SetIndexEntry(index, entry);
}

IndexEntry EngineImpl::ResizeL4096PlusIndexEntry(IndexEntry entry, uint64 new_size) {
	if (GetClusterNumber(entry.array_size) == GetClusterNumber(new_size)) {
		entry.array_size = new_size;
		return entry;
	} else {
		L4096PlusClusterIterator iterator(*this, entry);
		iterator.Resize(new_size);
		return iterator.GetEntry();
	}
}

IndexEntry EngineImpl::ResizeIndexEntry(IndexEntry entry, uint64 new_size) {
	uint64 old_size = entry.array_size;
	assert(old_size != free_entry_array_size);
	assert(new_size != free_entry_array_size);

	BlockType old_type = GetBlockType(old_size);
	BlockType new_type = GetBlockType(new_size);

	if (old_type == new_type) {
		if (old_type != BlockType::L4096Plus) {
			entry.array_size = new_size;
			return entry;
		} else {
			return ResizeL4096PlusIndexEntry(entry, new_size);
		}
	}

	BlockType from_type = old_type;
	BlockType to_type = new_type;

	if (old_type == BlockType::L4096Plus) {
		entry = ResizeL4096PlusIndexEntry(entry, cluster_size);
		from_type = BlockType::L4096;
	}

	if (new_type == BlockType::L4096Plus) {
		to_type = BlockType::L4096;
	}

	if (from_type != to_type) {
		if (to_type == BlockType::L8) {
			uint64 old_block_offset = entry.offset;
			entry.data = Get<uint64>(old_block_offset);
			DeallocateBlock(from_type, old_block_offset);
		} else {
			uint64 destination_data_offset = AllocateBlock(to_type);
			if (from_type == BlockType::L8) {
				Set<uint64>(destination_data_offset, entry.data);
			} else {
				MoveData(entry.offset, destination_data_offset, std::min(old_size, new_size));
				DeallocateBlock(from_type, entry.offset);
			}
			entry.offset = destination_data_offset;
		}
	}

	if (new_type == BlockType::L4096Plus) {
		entry.array_size = cluster_size;
		entry = ResizeL4096PlusIndexEntry(entry, new_size);
	} else {
		entry.array_size = new_size;
	}

	return entry;
}

ArrayIndex EngineImpl::CreateArray() {
	return AllocateIndex();
}

void EngineImpl::DestroyArray(ArrayIndex index) {
	if (!IsIndexValid(index)) { throw std::invalid_argument("invalid array index"); }
	DeallocateIndex(index);
}

uint64 EngineImpl::GetArraySize(ArrayIndex index) const {
	if (!IsIndexValid(index)) { throw std::invalid_argument("invalid array index"); }
	return GetIndexEntry(index).array_size;
}

void EngineImpl::SetArraySize(ArrayIndex index, uint64 size) {
	if (!IsIndexValid(index)) { throw std::invalid_argument("invalid array index"); }
	IndexEntry entry = GetIndexEntry(index);
	entry = ResizeIndexEntry(entry, size);
	SetIndexEntry(index, entry);
}

void EngineImpl::ReadArray(ArrayIndex index, uint64 offset, uint64 size, void* data) const {
	if (!IsIndexValid(index)) { throw std::invalid_argument("invalid array index"); }
	if (size == 0) { return; }
	IndexEntry entry = GetIndexEntry(index);
	if (offset >= entry.array_size || offset + size > entry.array_size) { throw std::invalid_argument("invalid offset or size"); }
	BlockType type = GetBlockType(entry.array_size);
	if (type == BlockType::L8) {
		memcpy(data, (char*)&entry.data + offset, size); 
		return;
	}
	if (type < BlockType::L4096Plus) {
		void* cluster_address = GetClusterAddress(entry.offset & cluster_offset_mask);
		memcpy(data, (char*)cluster_address + (entry.offset & ~cluster_offset_mask) + offset, size);
		return;
	}
	assert(type == BlockType::L4096Plus);
	const L4096PlusClusterIterator iterator(const_cast<EngineImpl&>(*this), entry); 
	do{
		iterator.SeekToCluster(offset >> cluster_size_bits);
		void* cluster_address = iterator.GetCurrentClusterAddress();
		uint64 offset_in_cluster = offset & ~cluster_offset_mask;
		uint64 size_to_copy = std::min(size, cluster_size - offset_in_cluster);
		memcpy(data, (char*)cluster_address + offset_in_cluster, size_to_copy);
		size = size - size_to_copy;
		data = (char*)data + size_to_copy;
		offset = offset + size_to_copy;
	} while (size > 0);
}

void EngineImpl::WriteArray(ArrayIndex index, const void* data, uint64 size, uint64 offset) {
	if (!IsIndexValid(index)) { throw std::invalid_argument("invalid array index"); }
	if (size == 0) { return; }
	IndexEntry entry = GetIndexEntry(index);
	if (offset >= entry.array_size || offset + size > entry.array_size) { throw std::invalid_argument("invalid offset or size"); }
	BlockType type = GetBlockType(entry.array_size);
	if (type == BlockType::L8) {
		memcpy((char*)&entry.data + offset, data, size);
		SetIndexEntry(index, entry);
		return;
	}
	if (type < BlockType::L4096Plus) {
		void* cluster_address = GetClusterAddress(entry.offset & cluster_offset_mask);
		memcpy((char*)cluster_address + (entry.offset & ~cluster_offset_mask) + offset, data, size);
		return;
	}
	assert(type == BlockType::L4096Plus);
	const L4096PlusClusterIterator iterator(*this, entry);
	do {
		iterator.SeekToCluster(offset >> cluster_size_bits);
		void* cluster_address = iterator.GetCurrentClusterAddress();
		uint64 offset_in_cluster = offset & ~cluster_offset_mask;
		uint64 size_to_copy = std::min(size, cluster_size - offset_in_cluster);
		memcpy((char*)cluster_address + offset_in_cluster, data, size_to_copy);
		size = size - size_to_copy;
		data = (char*)data + size_to_copy;
		offset = offset + size_to_copy;
	} while (size > 0);
}


END_NAMESPACE(DynamicStore)