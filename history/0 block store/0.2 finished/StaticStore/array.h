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
	uint64 _length;

public:
	Array(Engine& engine, ArrayIndex& index) : _engine(engine), _index(index) {
		if (_index.IsInvalid()) { _length = 0; return; }
		uint64 size = _engine.GetArrayData(_index).second;
		if (size % element_size != 0) { throw std::invalid_argument("bad array length"); }
		_length = size / element_size;
	}

	uint64 GetLength() const { return _length; }

	void Load(ElementType* buffer, uint64 length) const {
		if (length == 0) { return; }
		if (length > _length) { throw std::invalid_argument("invalid array length"); }
		const void* data = _engine.GetArrayData(_index).first;
		memcpy(buffer, data, length * element_size);
	}

	void Store(const ElementType* buffer, uint64 length) {
		_index = _engine.CreateArray(buffer, length * element_size);
		_length = length;
	}
};


END_NAMESPACE(StaticStore)