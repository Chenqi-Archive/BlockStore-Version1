#pragma once

#include "core.h"

#include <memory>
#include <utility>


BEGIN_NAMESPACE(StaticStore)


struct ArrayIndex {
	static constexpr uint64 invalid_array_index_value = (uint64)-1;
	uint64 value;
	constexpr ArrayIndex(uint64 value = invalid_array_index_value) : value(value) {}
	bool IsInvalid() const { return value == invalid_array_index_value; }
};


struct ABSTRACT_BASE Engine {
public:
	STATICSTORE_API static std::unique_ptr<Engine> Create(const wchar file[]);
	STATICSTORE_API static std::unique_ptr<Engine> Create(const void* data = nullptr, uint64 size = 0);

	virtual ~Engine() pure {}


	//// load operations ////
public:
	virtual std::pair<const void*, uint64> GetRawData() const pure;
public:
	virtual std::pair<const void*, uint64> GetMetaData() const pure;
	virtual std::pair<const void*, uint64> GetArrayData(ArrayIndex index) const pure;
public:
	template<class Metadata> 
	Metadata GetMetadata() const { 
		Metadata metadata; 
		auto [data, size] = GetMetaData();
		memcpy(&metadata, data, std::min(size, sizeof(Metadata)));
		return metadata; 
	}


	//// save operations ////
public:
	virtual void Format() pure;
	virtual const ArrayIndex CreateArray(const void* data, uint64 size) pure;
	virtual void SetMetadata(const void* metadata, uint64 metadata_size) pure;
public:
	template<class Metadata> 
	void SetMetadata(const Metadata& metadata) { SetMetadata(&metadata, sizeof(Metadata)); }
};


END_NAMESPACE(StaticStore)