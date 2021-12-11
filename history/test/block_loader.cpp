#include <type_traits>
#include <tuple>
#include <stdexcept>

#include <string>
#include <vector>
#include <list>
#include <array>
#include <variant>

// layout definition

struct memcpy_initializable_t {};
constexpr memcpy_initializable_t memcpy_initializable = {};

template<class T>
struct layout_type {};

template<class T, class = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr auto layout(layout_type<T>) { return memcpy_initializable; }

template<class T, class... Ts>
constexpr auto declare(Ts T::*... member_list) { return std::make_tuple(member_list...); }

template<class T, class... Ts>
constexpr auto member_type_tuple(std::tuple<Ts T::*...>) -> std::tuple<Ts...> { return {}; }

template<class T, class = void>
constexpr bool is_memcpy_initializable = false;

template<class T>
constexpr bool is_memcpy_initializable<T, std::enable_if_t<std::is_same_v<memcpy_initializable_t, decltype(layout(layout_type<T>()))>>> = true;


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
		constexpr size_t size_first = block_size_t<T>::value;
		if (size_first == block_size_dynamic) { return block_size_dynamic; }
		constexpr size_t size_rest = block_size_t<std::tuple<Ts...>>::value;
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


template<class T>
constexpr bool is_block_size_fixed = !is_memcpy_initializable<T> && block_size<T> != block_size_dynamic;

template<class T>
constexpr bool is_block_size_dynamic = !is_memcpy_initializable<T> && block_size<T> == block_size_dynamic;


class BlockLoader {
public:
	std::string buffer;

	// load
	const char* Read(size_t index, size_t length) { return buffer.c_str() + index; }


	template<class T>
	void Copy(const char*& data, T& value, std::enable_if_t<is_memcpy_initializable<T>, int> = 0) {
		constexpr size_t size = sizeof(T); memcpy(&value, data, size); data += size;
	}

	template<class T>
	void Copy(const char*& data, T& value, std::enable_if_t<!is_memcpy_initializable<T>, int> = 0) {
		std::apply([&](auto... member) { (Copy(data, value.*member), ...); }, layout(layout_type<T>()));
	}


	template<class T>
	void Load(size_t& index, T& value, std::enable_if_t<is_memcpy_initializable<T>, int> = 0) {
		constexpr size_t size = sizeof(T); memcpy(&value, Read(index, size), size); index += size;
	}

	template<class T>
	void Load(size_t& index, T& value, std::enable_if_t<is_block_size_fixed<T>, int> = 0) {
		constexpr size_t size = block_size<T>; const char* data = Read(index, size); Copy(data, value);
	}

	template<class T>
	void Load(size_t& index, T& value, std::enable_if_t<is_block_size_dynamic<T>, int> = 0) {
		std::apply([&](auto... member) { (Load(index, value.*member), ...); }, layout(layout_type<T>()));
	}


	// basic_string

	template<class T, class = std::enable_if_t<std::is_trivial_v<T>>>
	void Load(size_t& index, std::basic_string<T>& value) {
		size_t length; Load(index, length); value.resize(length);
		size_t size = length * sizeof(T); memcpy(value.data(), Read(index, size), size); index += size;
	}


	// vector

	template<class T>
	void Load(size_t& index, std::vector<T>& value, std::enable_if_t<is_memcpy_initializable<T>, int> = 0) {
		size_t length; Load(index, length); value.resize(length);
		size_t size = length * sizeof(T); memcpy(value.data(), Read(index, size), size); index += size;
	}

	template<class T>
	void Load(size_t& index, std::vector<T>& value, std::enable_if_t<is_block_size_fixed<T>, int> = 0) {
		size_t length; Load(index, length); value.resize(length);
		size_t size = length * sizeof(T); const char* data = Read(index, size);
		for (T& item : value) { Copy(data, item); }
	}

	template<class T>
	void Load(size_t& index, std::vector<T>& value, std::enable_if_t<is_block_size_dynamic<T>, int> = 0) {
		size_t length; Load(index, length); value.resize(length);
		for (T& item : value) { Load(index, item); }
	}


	// array

	template<class T, size_t length>
	void Load(size_t& index, std::array<T, length>& value, std::enable_if_t<is_memcpy_initializable<T>, int> = 0) {
		size_t size = length * sizeof(T); memcpy(value.data(), Read(index, size), size); index += size;
	}

	template<class T, size_t length>
	void Load(size_t& index, std::array<T, length>& value, std::enable_if_t<is_block_size_fixed<T>, int> = 0) {
		size_t size = length * sizeof(T); const char* data = Read(index, size);
		for (T& item : value) { Copy(data, item); }
	}

	template<class T, size_t length>
	void Load(size_t& index, std::array<T, length>& value, std::enable_if_t<is_block_size_dynamic<T>, int> = 0) {
		for (T& item : value) { Load(index, item); }
	}


	// list

	template<class T>
	void Load(size_t& index, std::list<T>& value, std::enable_if_t<is_block_size_fixed<T>, int> = 0) {
		size_t length; Load(index, length); value.resize(length);
		size_t size = length * sizeof(T); const char* data = Read(index, size);
		for (T& item : value) { Copy(data, item); }
	}

	template<class T>
	void Load(size_t& index, std::list<T>& value, std::enable_if_t<is_block_size_dynamic<T>, int> = 0) {
		size_t length; Load(index, length); value.resize(length);
		for (T& item : value) { Load(index, item); }
	}


	// variant

	template<size_t I, class... Ts>
	std::variant<Ts...> load_variant(size_t& index, std::size_t tag) {
		if constexpr (I < sizeof...(Ts)) {
			if (tag == I) { return Load<std::variant_alternative_t<I, std::variant<Ts...>>>(index); }
			return load_variant<I + 1, Ts...>(index, tag);
		}
		throw std::runtime_error("invalid variant tag");
	}

	template<class... Ts>
	void Load(size_t& index, std::variant<Ts...>& value) {
		size_t tag = Load<size_t>(index); value = load_variant<0, Ts...>(index, tag);
	}

	template<class T> T Load(size_t& index) { T t; Load(index, t); return t; }

	template<class T> T LoadBlock(size_t index) { return Load<T>(index); }
	template<class T> T LoadRootBlock() { return LoadBlock<T>(0); }
};

namespace A {

// types
using Int = int;
using Point = struct Point { int x = 0, y = 0; };
using Size = struct Size { unsigned width = 0, height = 0; };
using Rect = struct Rect { Point point; Size size; };
using TextBox = struct { std::string text; Point point; };
using ListLayout = struct { std::vector<TextBox> child_list; Rect rect; };

constexpr auto layout(layout_type<Int>) { return memcpy_initializable; }
constexpr auto layout(layout_type<Point>) { return memcpy_initializable; }
constexpr auto layout(layout_type<Size>) { return declare(&Size::width); }
constexpr auto layout(layout_type<Rect>) { return declare(&Rect::point, &Rect::size); }
constexpr auto layout(layout_type<TextBox>) { return declare(&TextBox::text, &TextBox::point); }
constexpr auto layout(layout_type<ListLayout>) { return declare(&ListLayout::child_list, &ListLayout::rect); }

}

using namespace A;


template<class T>
std::string bit_convert(T size) {
	return std::string((char*)&size, sizeof(T));
}

std::string build_test_string() {
	return
		bit_convert<size_t>(3) +  // 3 text_boxes
		bit_convert<size_t>(5) + "abcde" + bit_convert<int>(10) + bit_convert<int>(20) +
		bit_convert<size_t>(2) + "gd" + bit_convert<int>(20) + bit_convert<int>(-40) +
		bit_convert<size_t>(10) + "abaferecde" + bit_convert<int>(7) + bit_convert<int>(0) +
		bit_convert<int>(7) + bit_convert<int>(0) + bit_convert<unsigned>(7);
}


int main() {
	BlockLoader block_loader; block_loader.buffer = build_test_string();
	auto i = block_loader.LoadBlock<Int>(0);
	auto p = block_loader.LoadBlock<Point>(0);
	auto s = block_loader.LoadBlock<Size>(0);
	auto r = block_loader.LoadBlock<Rect>(0);
	auto t = block_loader.LoadBlock<TextBox>(0);
	auto l = block_loader.LoadBlock<ListLayout>(0);
	auto v = block_loader.LoadBlock<std::variant<Int, Point, Size, TextBox>>(0);


	//block_size<Int>;
	//block_size<Point>;
	//block_size<Size>;
	//block_size<Rect>;
	//block_size<TextBox>;
	//block_size<ListLayout>;
}