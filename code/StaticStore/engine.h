#pragma once

#include "core.h"

#include <memory>
#include <utility>


BEGIN_NAMESPACE(StaticStore)


struct ABSTRACT_BASE Engine {
public:
	STATICSTORE_API static std::unique_ptr<Engine> Create(const wchar file[]);
	STATICSTORE_API static std::unique_ptr<Engine> Create(const void* data, uint64 size);
	static std::unique_ptr<Engine> Create() { return Create(nullptr, 0); }

	virtual ~Engine() pure {}


private:
	template<class ElementType> friend class Array;


	//// load ////
public:
	virtual std::pair<const void*, uint64> GetRawData() const pure;
private:
	virtual std::pair<const void*, uint64> GetMetadata() const pure;
public:
	template<class Metadata> Metadata GetMetadata() const { 
		Metadata metadata; auto [data, size] = GetMetadata();
		if (size == sizeof(Metadata)) { memcpy(&metadata, data, size); }
		return metadata; 
	}
private:
	virtual std::pair<const void*, uint64> GetArrayData(uint64 array_index) const pure;


	//// save ////
public:
	virtual void Format() pure;
private:
	virtual void SetMetadata(const void* metadata, uint64 metadata_size) pure;
public:
	template<class Metadata> void SetMetadata(const Metadata& metadata) { 
		SetMetadata(&metadata, sizeof(Metadata)); 
	}
private:
	virtual uint64 CreateArray(const void* data, uint64 size) pure;
};


END_NAMESPACE(StaticStore)