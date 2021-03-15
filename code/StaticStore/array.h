#pragma once

#include "engine.h"


BEGIN_NAMESPACE(StaticStore)


template<class ElementType>
class Array {
private:
	static constexpr uint64 element_size = sizeof(ElementType);

private:
	Engine& _engine;
	ArrayIndex& _index;

public:
	Array(Engine& engine, ArrayIndex& index) : _engine(engine), _index(index) {
		if (_index.IsInvalid()) { throw std::invalid_argument("invalid array index"); }
		if (_engine.GetArraySize(_index) % element_size != 0) { throw std::invalid_argument("bad array length"); }
	}

	uint64 GetLength() const {
		uint64 size = _engine.GetArraySize(_index);
		if (size % element_size != 0) { throw std::invalid_argument("bad array length"); }
		size = size / element_size;
		return size;
	}

	void Load(uint64 begin, uint64 length, ElementType* buffer) const {
		_engine.ReadArray(_index, begin * element_size, length * element_size, buffer);
	}

	void Create(uint64 length) {
		_index = _engine.CreateArray(length * element_size);
	}

	void Store(const ElementType* buffer, uint64 length, uint64 begin) {
		_engine.WriteArray(_index, buffer, length * element_size, begin * element_size);
	}
};


END_NAMESPACE(StaticStore)