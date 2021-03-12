#pragma once

#include "core.h"

#include <memory>


BEGIN_NAMESPACE(DynamicStore)


struct ArrayIndex {
	static constexpr uint64 invalid_array_index_value = (uint64)-1;
	uint64 value = invalid_array_index_value;
	constexpr explicit ArrayIndex(uint64 value) : value(value) {}
	bool IsInvalid() const { return value == invalid_array_index_value; }
};


struct ABSTRACT_BASE Engine {
public:
	DYNAMICSTORE_API static std::unique_ptr<Engine> Create(const wchar file[]);
	virtual ~Engine() pure;

	//// metadata ////
private:
	virtual void LoadUserMetadata(void* data, uint64 size) const pure;
	virtual void StoreUserMetadata(const void* data, uint64 size) pure;
public:
	static constexpr uint64 max_metadata_size = 144;
public:
	template<class Metadata>
	Metadata GetMetadata() { 
		static_assert(sizeof(Metadata) <= max_metadata_size);
		Metadata metadata; LoadUserMetadata(&metadata, sizeof(Metadata)); return metadata;
	}
	template<class Metadata>
	void SetMetadata(const Metadata& metadata) {
		static_assert(sizeof(Metadata) <= max_metadata_size);
		StoreUserMetadata(&metadata, sizeof(Metadata));
	}

	//// array operations ////
public:
	virtual ArrayIndex CreateArray() pure;
	virtual void DestroyArray(ArrayIndex index) pure;
	virtual uint64 GetArraySize(ArrayIndex index) const pure;
	virtual void SetArraySize(ArrayIndex index, uint64 size) pure;
	virtual void ReadArray(ArrayIndex index, uint64 offset, uint64 size, void* data) const pure;
	virtual void WriteArray(ArrayIndex index, const void* data, uint64 size, uint64 offset) pure;
};


END_NAMESPACE(DynamicStore)