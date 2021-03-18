#pragma once

#include "../StaticStore/StaticStore.h"

#include <string>


using namespace StaticStore;


int main() {
	auto engine = Engine::Create(L"R:/test_static.dat");

	ArrayIndex root_array_index = engine->GetMetadata<ArrayIndex>();

	Array<char> root_array(*engine, root_array_index);
	uint64 length = root_array.GetLength();

	std::string str(length, '\0');
	root_array.Load(str.data(), length);

	str.append("-string modified-");

	engine->Format();
	root_array.Store(str.data(), str.length());
	engine->SetMetadata<ArrayIndex>(root_array_index);
}