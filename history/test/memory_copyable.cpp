#include <type_traits>
#include <tuple>



template<class T>
constexpr bool is_block_type_valid = !std::is_pointer_v<T> && !std::is_reference_v<T> && std::is_trivially_constructible_v<T>;


struct memory_copyable_t {};
constexpr memory_copyable_t memory_copyable = {};


template<class T>
struct layout_type {};


template<class T, class = std::enable_if_t<std::is_arithmetic_v<T>>>
constexpr auto layout(layout_type<T>) { return memory_copyable; }


template<class T, class... Ts>
constexpr auto declare(Ts T::*... member_list) { return std::make_tuple(member_list...); }


template<class T, class... Ts>
constexpr auto get_member_type_tuple(std::tuple<Ts T::*...>) -> std::tuple<Ts...> { return {}; }


template<class... Ts>
constexpr bool is_memory_copyable = (std::is_same_v<memory_copyable_t, decltype(layout(layout_type<Ts>()))> && ...);

template<class... Ts>
constexpr bool is_memory_copyable<std::tuple<Ts...>> = is_memory_copyable<Ts...>;

template<class T>
constexpr bool is_memory_copyable_indirect = is_memory_copyable<decltype(get_member_type_tuple(layout(layout_type<T>())))>;


struct Point { int x, y; };

constexpr auto layout(layout_type<Point>) { return memory_copyable; }

struct Size { unsigned width, height; };

constexpr auto layout(layout_type<Size>) { return declare(&Size::width); }

struct Rect {};


int main() {
	is_memory_copyable<int>;
	is_memory_copyable<Point>;
	is_memory_copyable<Size>;
	//is_memory_copyable<Rect>;
	//is_memory_copyable_indirect<Point>;
	is_memory_copyable_indirect<Size>;
	//is_memory_copyable_indirect<Rect>;
}