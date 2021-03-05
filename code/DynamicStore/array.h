#pragma once

#include "engine.h"

#include <algorithm>


BEGIN_NAMESPACE(DynamicStore)


template<class ElementType>
class Array {
private:
	static constexpr uint64 element_size = sizeof(ElementType);

private:
	Engine& _engine;
	ArrayIndex _index;
	uint64 _length;

public:
	Array(Engine& engine, ArrayIndex index) : _engine(engine), _index(index) {
		uint64 size = engine.GetArraySize(index);
		if (size % element_size != 0) { throw std::invalid_argument("bad array length"); }
		_length = size / element_size;
	}

	uint64 GetLength() const { 
		return _length; 
	}

	void SetLength(uint64 length) {
		_engine.SetArraySize(_index, length * element_size);
		_length = length;
	}

	template<class OutputIterator>
	void Load(uint64 begin, uint64 length, OutputIterator it) {
		if (length == 0) { return; }
		if (begin + length > _length) { throw std::out_of_range("subscript out of range"); }
		ElementType* temp_array = new ElementType[length];
		Load(begin, length, temp_array);
		std::copy(temp_array, temp_array + length, it);
		delete[] temp_array;
	}

	template<>
	void Load<ElementType*>(uint64 begin, uint64 length, ElementType* data_array) {
		_engine.ReadArray(_index, begin * element_size, length * element_size, data_array);
	}

	template<class OutputIterator>
	void Load(OutputIterator it) {
		Load(0, _length, it);
	}

	template<class InputIterator>
	void Store(InputIterator it, uint64 length, uint64 begin) {
		if (length == 0) { return; }
		if (begin + length > _length) { throw std::out_of_range("subscript out of range"); }
		ElementType* temp_array = new ElementType[length];
	#error end iterator
		std::copy(temp_array, temp_array + length, it);
		Store(temp_array, length, begin);
		delete[] temp_array;
	}

	template<>
	void Store<ElementType*>(ElementType* data_array, uint64 length, uint64 begin) {
		_engine.WriteArray(_index, data_array, length * element_size, begin * element_size);
	}

	template<class InputIterator>
	void Store(InputIterator it) {
		Store(it, _length, begin);
	}
};


END_NAMESPACE(DynamicStore)