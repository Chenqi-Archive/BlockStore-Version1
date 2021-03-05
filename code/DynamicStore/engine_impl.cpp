#include "engine_impl.h"

#include <algorithm>


BEGIN_NAMESPACE(DynamicStore)


DYNAMICSTORE_API std::unique_ptr<Engine> Engine::Create(const wchar file[]) {
	return std::make_unique<EngineImpl>(file);
}


inline LengthType GetArrayLengthType(uint64 array_length) {
	auto cmp = [](uint64 block_length, uint64 array_length) { return block_length <= array_length; };
#pragma message(Remark"Use nested if-else for optimization.")
	auto it = std::lower_bound(block_length_table.begin(), block_length_table.end(), array_length, cmp);
	assert(it > block_length_table.begin());
	uint64 table_index = it - block_length_table.begin() - 1;
	if (table_index != 0 && array_length <= block_length_table[table_index] * 3 / 2) { table_index--; }
	return static_cast<LengthType>(table_index);
}


uint64 EngineImpl::GetMetadataSize() const {

}

void EngineImpl::LoadMetadata(void* data) {

}

void EngineImpl::StoreMetadata(void* data, uint64 size) {

	// format 
}

END_NAMESPACE(DynamicStore)