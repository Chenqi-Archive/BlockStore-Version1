#pragma once

#include "block.h"

#include <array> // temp

BEGIN_NAMESPACE(BlockStore)


// An array of class T blocks. (T must be copyable)
template<class T>
class ArrayBlock : private Block {
public:
	ArrayBlock(Handle handle) : Block(handle) {}

public:
	using Block::IsEmpty;
	using Block::MapView;
	using Block::UnMapView;

	uint64 GetLength() const { return GetSize() / sizeof(T); }
	void SetLength(uint64 length) { SetSize(length * sizeof(T)); }

	wchar operator[](uint64 index) const {
		wchar ch;
		Read(index * sizeof(wchar), sizeof(wchar), &ch);
		return ch;
	}

	void Append(const T& value, uint64 count = 1) {
		SetLength(GetLength() + count);
		Write()
	}
	void Append(ref_ptr<const T> value_array, uint64 count) {

	}
	void Insert(...) {}
	void Erase() {}

public:
	class Iterator {

	};

	Iterator begin() {}
	Iterator end() {}
};


END_NAMESPACE(BlockStore)