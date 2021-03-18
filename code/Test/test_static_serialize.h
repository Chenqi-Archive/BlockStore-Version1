#pragma once

#include "../StaticStore/StaticStore.h"

#include <string>


using namespace StaticStore;


void ReceiverTest(const void* raw_data, uint64 size) {
	auto engine_receiver = Engine::Create(raw_data, size);
	ArrayIndex root_array_index = engine_receiver->GetMetadata<ArrayIndex>();

	Array<char> root_array(*engine_receiver, root_array_index);
	uint64 length = root_array.GetLength();

	std::string str(length, '\0');
	root_array.Load(str.data(), length);
}


int main() {
	auto engine_sender = Engine::Create();

	ArrayIndex root_array_index = engine_sender->GetMetadata<ArrayIndex>();

	Array<char> root_array(*engine_sender, root_array_index);
	uint64 length = root_array.GetLength();

	std::string str(length, '\0');
	root_array.Load(str.data(), length);

	str.append("-string modified-");

	engine_sender->Format();
	root_array.Store(str.data(), str.length());
	engine_sender->SetMetadata<ArrayIndex>(root_array_index);

	auto [raw_data, size] = engine_sender->GetRawData();
	ReceiverTest(raw_data, size);
}