#pragma once

#include "../StaticStore/StaticStore.h"

#include <string>


using namespace StaticStore;


int main() {
	auto engine = Engine::Create(L"R:/test_static.dat");

	ArrayIndex<char> root_array_index = engine->GetMetadata<ArrayIndex<char>>();

	Array<char> root_array(*engine, root_array_index);
	auto [data, length] = root_array.Load();
	std::string str(data, length);

	str.append("-string modified-");

	engine->Format();
	root_array.Save(str.data(), str.length());
	engine->SetMetadata<ArrayIndex<char>>(root_array_index);
}