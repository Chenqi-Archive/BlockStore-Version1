#pragma once

#include "../StaticStore/StaticStore.h"

#include <string>


using namespace StaticStore;


void ReceiverTest(const void* raw_data, uint64 size) {
	auto engine_receiver = Engine::Create(raw_data, size);
	ArrayIndex<char> root_array_index = engine_receiver->GetMetadata<ArrayIndex<char>>();

	Array<char> root_array(*engine_receiver, root_array_index);
	auto [data, length] = root_array.Load();
	std::string str(data, length);
}


int main() {
	auto engine_sender = Engine::Create();

	ArrayIndex<char> root_array_index = engine_sender->GetMetadata<ArrayIndex<char>>();

	Array<char> root_array(*engine_sender, root_array_index);
	auto [data, length] = root_array.Load();
	std::string str(data, length);

	str.append("-string modified-");

	engine_sender->Format();
	root_array.Store(str.data(), str.length());
	engine_sender->SetMetadata<ArrayIndex<char>>(root_array_index);

	auto [raw_data, size] = engine_sender->GetRawData();
	ReceiverTest(raw_data, size);
}