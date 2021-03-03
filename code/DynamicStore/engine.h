#pragma once

#include "core.h"

#include <array>
#include <vector>


BEGIN_NAMESPACE(DynamicStore)

using Index = uint64;
using Length = uint64;

constexpr Index invalid_index = (Index)-1;


class Engine {
public:

	bool IsEmpty() {

	}

	template<class ElementType>
	Index CreateArray(ref_ptr<ElementType> data, Length length) {

	}
};


template<class _ElementType>
class Array {
private:
	using ElementType = _ElementType == Array ? Index : _ElementType;


private:
	Engine& engine;
	Index index;
private:
	static constexpr uint64 element_size = sizeof(ElementType);
	Length length;
public:
	Array(Engine& engine) : engine(engine), index(invalid_index), length(0) {}
	Array(Engine& engine, Index index, Length length) : engine(engine), index(index), length(length) {}
};


END_NAMESPACE(DynamicStore)