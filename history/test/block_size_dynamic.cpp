#pragma once

#include "block_layout.h"


BEGIN_NAMESPACE(BlockStore)


constexpr size_t block_size_dynamic = -1;


template<class T, class = void>
struct block_size_t {
	static constexpr size_t value = block_size_dynamic;
};

template<class T>
struct block_size_t<T, std::enable_if_t<is_memcpy_initializable<T>>> {
	static constexpr size_t value = sizeof(T);
};

template<class T, class... Ts>
struct block_size_t<std::tuple<T, Ts...>> {
	static constexpr size_t calculate_size() {
		size_t size_first = block_size_t<T>::value;
		if (size_first == block_size_dynamic) { return block_size_dynamic; }
		size_t size_rest = block_size_t<std::tuple<Ts...>>::value;
		if (size_rest == block_size_dynamic) { return block_size_dynamic; }
		return size_first + size_rest;
	}
	static constexpr size_t value = calculate_size();
};

template<class T>
struct block_size_t<std::tuple<T>> : block_size_t<T> {};

template<class T>
struct block_size_t<T, decltype(member_type_tuple(layout(layout_type<T>())), void())> : block_size_t<decltype(member_type_tuple(layout(layout_type<T>())))> {};

template<class T>
constexpr size_t block_size = block_size_t<T>::value;


template<class T, size_t length>
struct block_size_t<std::array<T, length>> {
	static constexpr size_t calculate_size() {
		size_t size = block_size_t<T>::value;
		if (size == block_size_dynamic) { return block_size_dynamic; }
		return size * length;
	}
	static constexpr size_t value = calculate_size();
};


template<class T, class... Ts>
struct block_size_t<std::variant<T, Ts...>> {
	static constexpr size_t calculate_size() {
		size_t size_first = block_size_t<T>::value;
		if (size_first == block_size_dynamic) { return block_size_dynamic; }
		size_t size_rest = block_size_t<std::variant<Ts...>>::value;
		if (size_rest == block_size_dynamic) { return block_size_dynamic; }
		if (size_first + sizeof(size_t) != size_rest) { return block_size_dynamic; }
		return size_rest;
	}
	static constexpr size_t value = calculate_size();
};

template<class T>
struct block_size_t<std::variant<T>> {
	static constexpr size_t calculate_size() {
		size_t size = block_size_t<T>::value;
		if (size == block_size_dynamic) { return block_size_dynamic; }
		return size + sizeof(size_t);
	}
	static constexpr size_t value = calculate_size();
};


END_NAMESPACE(BlockStore)