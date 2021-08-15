#pragma once

#include "core.h"

#include <memory>
#include <utility>


BEGIN_NAMESPACE(StaticStore)


template<class ElementType>
class ArrayIndex {
	friend struct Engine;
	static constexpr uint64 invalid_array_index_value = (uint64)-1;
	uint64 value = invalid_array_index_value;
	bool IsInvalid() const { return value == invalid_array_index_value; }
};


struct ABSTRACT_BASE Engine {
public:
	STATICSTORE_API static std::unique_ptr<Engine> Create(const wchar file[]);
	STATICSTORE_API static std::unique_ptr<Engine> Create(const void* data, uint64 size);
	static std::unique_ptr<Engine> Create() { return Create(nullptr, 0); }

	virtual ~Engine() pure {}


public:
	virtual std::pair<const void*, uint64> GetRawData() const pure;
private:
	virtual std::pair<const void*, uint64> GetMetadata() const pure;
public:
	template<class Metadata> 
	Metadata GetMetadata() const { 
		Metadata metadata; auto [data, size] = GetMetadata();
		if (size == sizeof(Metadata)) { memcpy(&metadata, data, size); }
		return metadata; 
	}
private:
	virtual std::pair<const void*, uint64> GetArrayData(uint64 array_index) const pure;
public:
	template<class ElementType>
	std::pair<const ElementType*, uint64> LoadArray(ArrayIndex<ElementType> array_index) const {
		static_assert(sizeof(ArrayIndex<ElementType>) == 8);
		constexpr uint64 element_size = sizeof(ElementType);
		if (array_index.IsInvalid()) { return { nullptr, 0 }; }
		auto [data, size] = GetArrayData(array_index.value);
		if (size % element_size != 0) { throw std::invalid_argument("bad array length"); }
		return { static_cast<const ElementType*>(data), size / element_size };
	}


public:
	virtual void Format() pure;
private:
	virtual void SetMetadata(const void* metadata, uint64 metadata_size) pure;
public:
	template<class Metadata> 
	void SetMetadata(const Metadata& metadata) { 
		SetMetadata(&metadata, sizeof(Metadata)); 
	}
private:
	virtual uint64 CreateArray(const void* data, uint64 size) pure;
public:
	template<class ElementType>
	void SaveArray(ArrayIndex<ElementType>& array_index, const ElementType* buffer, uint64 length) {
		static_assert(sizeof(ArrayIndex<ElementType>) == 8);
		constexpr uint64 element_size = sizeof(ElementType);
		array_index.value = CreateArray(buffer, length * element_size);
	}
};


END_NAMESPACE(StaticStore)