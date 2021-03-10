#include "engine_impl.h"


BEGIN_NAMESPACE(DynamicStore)


DYNAMICSTORE_API std::unique_ptr<Engine> Engine::Create(const wchar file[]) {
	return std::make_unique<EngineImpl>(file);
}


bool EngineImpl::LoadAndCheck() {
	if (_size == 0 || _size % cluster_size != 0) { return false; }

	_static_metadata = *(StaticMetadata*)GetClusterAddress(0);
	if (_static_metadata.file_size != _size) { return false; }
	if (_static_metadata.user_metadata_size > max_user_metadata_size) { return false; }

	return true;
}

void EngineImpl::Format() {
	// Set file size to 4k.
	SetSize(cluster_size);

	// Initialize metadata.
	_static_metadata.file_size = _size;
	_static_metadata.index_table_entry.array_size = 0;
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

ArrayIndex EngineImpl::CreateArray() { 

}

void EngineImpl::DestroyArray(ArrayIndex index) {}

uint64 EngineImpl::GetArraySize(ArrayIndex index) const { 

}

void EngineImpl::SetArraySize(ArrayIndex index, uint64 size) {}

void EngineImpl::ReadArray(ArrayIndex index, uint64 offset, uint64 size, void* data) const {}

void EngineImpl::WriteArray(ArrayIndex index, const void* data, uint64 size, uint64 offset) {}



END_NAMESPACE(DynamicStore)