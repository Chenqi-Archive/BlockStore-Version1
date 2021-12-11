#include <type_traits>
#include <tuple>

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

template<class T>
constexpr bool is_memcpy_initializable = std::is_same_v<memcpy_initializable_t, decltype(layout(layout_type<T>()))>;


constexpr size_t block_size_dynamic = -1;

template<class T, class = void>
struct block_size_t {
	static constexpr size_t value = block_size_dynamic;
};

template<class T>
struct block_size_t<T, std::enable_if_t<std::is_same_v<memcpy_initializable_t, decltype(layout(layout_type<T>()))>>> {
	static constexpr size_t value = sizeof(T);
};

template<class T, class... Ts>
struct block_size_t<std::tuple<T, Ts...>> {
	static constexpr size_t calculate_size() {
		constexpr size_t size_first = block_size_t<T>::value;
		if constexpr (size_first == block_size_dynamic) { return block_size_dynamic; }
		constexpr size_t size_rest = block_size_t<std::tuple<Ts...>>::value;
		if constexpr (size_rest == block_size_dynamic) { return block_size_dynamic; }
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


/*
// load
const void* Read(size_t index, size_t length) { return nullptr; }

template<class T, class = std::enable_if_t<is_memcpy_initializable<T>>>
void Copy(const void* data, T& value) {

}

template<class T, class = std::enable_if_t<is_memcpy_initializable<T>>>
void Load(size_t& index, T& value) {
	constexpr size_t size = sizeof(T);
	memcpy(&value, Read(index, size), size); index += size;
}

template<class T, class = std::enable_if_t<is_memory_copyable_indirect<T>>>
void Load(size_t& index, T& value) {
	constexpr size_t size = block_size<T>; const void* data = Read(index, size);
	std::apply([](auto... member) { (memcpy(&value.*member, data, sizeof()), ...); }, layout(layout_type<T>()));
}

template<class T, class = std::enable_if_t<!is_memory_copyable_indirect<T>>>
void Load(size_t& index, T& value) {
	std::apply([](auto... member) { (Load(index, value.*member), ...); }, layout(layout_type<T>()));
}

template<class E, >
void Load(size_t& index, std::basic_string<E>& value) {
	size_t size; Load(index, size); value.clear(); value.resize(size);
	std::for_each(value.begin(), value.end(), [](T& value) { Load(index, value); });
}

template<class T>
void Load(size_t& index, std::vector<T>& value) {
	size_t size; Load(index, size); value.clear(); value.resize(size);
	memcpy(value.data(), Read(index, ))
		std::for_each(value.begin(), value.end(), [](T& value) { Load(index, value); });
}

template<size_t I, class... Ts>
std::variant<Ts...> load_variant(size_t& index, std::size_t tag) {
	if constexpr (I < sizeof...(Ts)) {
		if (tag == I) { return Load<std::variant_alternative_t<I, std::variant<Ts...>>>(index); }
		return load_variant<I + 1, Ts...>(index, tag);
	}
	throw std::runtime_error("invalid union index");
}

template<class... Ts>
void Load(size_t& index, std::variant<Ts...>& value) {
	size_t tag = Load<size_t>(index); value = load_variant<0, Ts...>(index, tag);
}

template<class T> T Load(size_t& index) { T t; Load(index, t); return t; }

template<class T> T LoadBlock(size_t index) { return Load<T>(index); }
template<class T> T LoadRootBlock() { return LoadBlock<T>(0); }
*/

// types
using Int = int;
using Point = struct { int x, y; };
using Size = struct { unsigned width, height; };
using Rect = struct { Point point; Size size; };
using TextBox = struct { std::wstring text; Point point; };
using ListLayout = struct { std::vector<TextBox> child_list; Point point; };

constexpr auto layout(layout_type<Int>) { return memcpy_initializable; }
constexpr auto layout(layout_type<Point>) { return memcpy_initializable; }
constexpr auto layout(layout_type<Size>) { return declare(&Size::width); }
constexpr auto layout(layout_type<Rect>) { return declare(&Rect::point, &Rect::size); }
constexpr auto layout(layout_type<TextBox>) { return declare(&TextBox::text, &TextBox::point); }
constexpr auto layout(layout_type<ListLayout>) { return declare(&ListLayout::child_list, &ListLayout::point); }


int main() {
	/*
	Int i = LoadBlock<Int>(0);
	Point p = LoadBlock<Point>(0);
	Size s = LoadBlock<Size>(0);
	Rect r = LoadBlock<Rect>(0);
	TextBox t = LoadBlock<TextBox>(0);
	ListLayout l = LoadBlock<ListLayout>(0);
	*/

	block_size<Int>;
	block_size<Point>;
	block_size<Size>;
	block_size<Rect>;
	block_size<TextBox>;
	block_size<ListLayout>;
}