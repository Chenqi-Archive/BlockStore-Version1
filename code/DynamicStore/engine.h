#pragma once

#include "core.h"


BEGIN_NAMESPACE(DynamicStore)

using Index = uint64;
constexpr Index invalid_index = (Index)-1;


class Engine {
public:
	Engine() {}
	~Engine() {}

public:
	void Load(const wchar file[]) {}

	template<class Metadata>
	void GetMetadata(Metadata& metadata) {
		// Metadata should implement Check and Init function.

		// First check metadata size.

		// If size match, load metadata and Check.

		// If check fails, initialize metadata.


	}

	template<class Metadata>
	Metadata GetMetadata() {
		Metadata metadata; GetMetadata(metadata);
		return metadata;
	}

public:
	Index CreateArray() { return 0; }
	void DestroyArray(Index index) {}

	uint64 GetArraySize(Index index) { return 0; }
	void SetArraySize(Index index, uint64 size) {}
};


template<class ElementType>
class Array {
private:
	static constexpr uint64 element_size = sizeof(ElementType);
private:
	Engine& _engine;
	Index _index;
	uint64 _length;
public:
	Array(Engine& engine, Index index) : _engine(engine), _index(index) {
		uint64 size = engine.GetArraySize(index);
		assert(size % element_size == 0);
		_length = size / element_size;
	}

	uint64 GetLength() { return _length; }

	template<class OutputIterator>
	void Load(uint64 begin, uint64 length, OutputIterator it) {
		if (length == 0) { return; }
		if (begin + length > _length) { throw std::out_of_range("subscript out of range"); }
	}

	template<class OutputIterator>
	void Load(OutputIterator it) { 
		Load(0, _length, it);
	}


	void SetLength(uint64 length) {
		_engine.SetArraySize(_index, length * element_size);
		_length = length;
	}

	template<class InputIterator>
	void Store(InputIterator it, uint64 begin, uint64 length) {
		if (length == 0) { return; }
		if (begin + length > _length) { throw std::out_of_range("subscript out of range"); }
	}

	template<class InputIterator>
	void Store(InputIterator it) {
		Store(it, 0, _length);
	}
};


END_NAMESPACE(DynamicStore)