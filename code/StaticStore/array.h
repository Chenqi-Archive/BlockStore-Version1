#pragma once

#include "engine.h"


BEGIN_NAMESPACE(StaticStore)


template<class ElementType>
class ArrayIndex {
	static constexpr uint64 invalid_array_index_value = (uint64)-1;
	friend class Array<ElementType>;
	uint64 value = invalid_array_index_value;
	bool IsInvalid() const { return value == invalid_array_index_value; }
};


template<class ElementType>
class Array {
private:
	static constexpr uint64 element_size = sizeof(ElementType);

private:
	Engine& _engine;
	ArrayIndex<ElementType>& _index;
	static_assert(sizeof(ArrayIndex<ElementType>) == 8);

public:
	Array(Engine& engine, ArrayIndex<ElementType>& index) : _engine(engine), _index(index) {}

	std::pair<const ElementType*, uint64> Load() const {
		if (_index.IsInvalid()) { return { nullptr, 0 }; }
		auto [data, size] = _engine.GetArrayData(_index.value);
		if (size % element_size != 0) { throw std::invalid_argument("bad array length"); }
		return { static_cast<const ElementType*>(data), size / element_size };
	}

	void Store(const ElementType* buffer, uint64 length) {
		_index.value = _engine.CreateArray(buffer, length * element_size);
	}

	std::pair<ElementType*, uint64> Store(uint64 length) {
		_index.value = _engine.CreateArray(nullptr, length * element_size); assert(!_index.IsInvalid());
		auto [data, size] = _engine.GetArrayData(_index.value); assert(size == length * element_size);
		return { const_cast<ElementType*>(static_cast<const ElementType*>(data)), length };
	}
};


END_NAMESPACE(StaticStore)