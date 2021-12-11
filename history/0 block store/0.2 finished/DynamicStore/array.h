#pragma once

#include "engine.h"


BEGIN_NAMESPACE(DynamicStore)


template<class ElementType>
class Array {
private:
	static constexpr uint64 element_size = sizeof(ElementType);

private:
	Engine& _engine;
	ArrayIndex& _index;

public:
	Array(Engine& engine, ArrayIndex& index) : _engine(engine), _index(index) {
		if (_index.IsInvalid()) { _index = _engine.CreateArray(); return; }
		if (_engine.GetArraySize(_index) % element_size != 0) { throw std::invalid_argument("bad array length"); }
	}

	void Destroy() {
		_engine.DestroyArray(_index);
		_index = {};
	}

	uint64 GetLength() const { 
		uint64 size = _engine.GetArraySize(_index);
		if (size % element_size != 0) { throw std::invalid_argument("bad array length"); }
		return size / element_size;
	}

	void SetLength(uint64 length) {
		_engine.SetArraySize(_index, length * element_size);
	}

	void Load(uint64 begin, uint64 length, ElementType* buffer) const {
		_engine.ReadArray(_index, begin * element_size, length * element_size, buffer);
	}

	void Store(const ElementType* buffer, uint64 length, uint64 begin) {
		_engine.WriteArray(_index, buffer, length * element_size, begin * element_size);
	}
};


END_NAMESPACE(DynamicStore)