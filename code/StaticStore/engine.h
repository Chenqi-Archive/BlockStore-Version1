#pragma once

#include "core.h"

#include <memory>


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
	virtual ~Engine() pure {}


	//// load operations ////
private:
	virtual void GetMetadata(void* data, uint64 size) const pure;
public:
	template<class Metadata> 
	Metadata GetMetadata() { Metadata metadata; GetMetadata(&metadata, sizeof(Metadata)); return metadata; }
public:
	virtual uint64 GetArraySize(ArrayIndex index) const pure;
	virtual void ReadArray(ArrayIndex index, uint64 offset, uint64 size, void* data) const pure;


	//// save operations ////
private:
	virtual void Format(const void* metadata, uint64 size) pure;
public:
	template<class Metadata> 
	void Format(const Metadata& metadata) { Format(&metadata, sizeof(Metadata)); }
public:
	virtual ArrayIndex CreateArray(uint64 array_size) pure;
	virtual void WriteArray(ArrayIndex index, const void* data, uint64 size, uint64 offset) pure;
};


END_NAMESPACE(StaticStore)