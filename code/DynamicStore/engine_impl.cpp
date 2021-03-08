#include "engine_impl.h"


BEGIN_NAMESPACE(DynamicStore)


DYNAMICSTORE_API std::unique_ptr<Engine> Engine::Create(const wchar file[]) {
	return std::make_unique<EngineImpl>(file);
}


bool EngineImpl::LoadAndCheck() {
	if (_size == 0 || _size % cluster_size != 0) { return false; }

	_static_metadata = *(StaticMetadata*)GetClusterAddress(0);
	if (_static_metadata.file_size != _size) { return false; }
	if (_static_metadata.free_cluster_head_offset > _size) { return false; }
	
	// check the consistency of ArrayIndexTable and BlockClusterTable

	if (_static_metadata.user_metadata_size > max_user_metadata_size) { return false; }


}

void StaticMetadata::Initialize() {
	file_size = cluster_size;
	free_cluster_head_offset = cluster_size;  // point to the end of file
	free_index_head = initial_array_index_table_size; // point to the end of array index table


	user_metadata_size = 0;
	memset(user_metadata, 0, max_user_metadata_size);
}

void EngineImpl::Format() {
	// Set file size to 4k.
	SetSize(cluster_size);

	// Initialize metadata.
	_static_metadata.Initialize();
	
	
	
// Divide the remaining space to  
}

void EngineImpl::LoadUserMetadata(void* data, uint64 size) const {
	assert(size <= max_user_metadata_size);
	memcpy(data, _static_metadata.user_metadata, size);
}

void EngineImpl::StoreUserMetadata(const void* data, uint64 size) {
	assert(size <= max_user_metadata_size);
	memcpy(_static_metadata.user_metadata, data, size);
}

ArrayIndex EngineImpl::CreateArray() { return 0; }

void EngineImpl::DestroyArray(ArrayIndex index) {}

uint64 EngineImpl::GetArraySize(ArrayIndex index) const { return 0; }

void EngineImpl::SetArraySize(ArrayIndex index, uint64 size) {}

void EngineImpl::ReadArray(ArrayIndex index, uint64 offset, uint64 size, void* data) const {}

void EngineImpl::WriteArray(ArrayIndex index, const void* data, uint64 size, uint64 offset) {}



END_NAMESPACE(DynamicStore)