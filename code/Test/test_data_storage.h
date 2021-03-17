#pragma once

#include "../DynamicStore/DynamicStore.h"

#include <string>


using namespace DynamicStore;


constexpr uint64 test_data_size = 50000;


std::string InitializeTestData() {
	std::string ret; ret.reserve(test_data_size);
	uint64 number = 0;
	while (ret.size() < test_data_size) {
		ret.append(std::to_string(number++));
	}
	return ret;
}


void TestArrayLength(Array<char>& array, uint64 length) {
	static std::string test_data = InitializeTestData();
	static std::string test_data_copy(test_data_size, '\0');

	array.SetLength(length);

	for (uint64 current_begin = 0; current_begin < length; current_begin += test_data_size) {
		uint64 current_length = std::min(length - current_begin, test_data_size);
		array.Store(test_data.data(), current_length, current_begin);
		array.Load(current_begin, current_length, test_data_copy.data());
		for (uint64 i = 0; i < current_length; ++i) {
			assert(test_data[i] == test_data_copy[i]);
		}
	}
}


int main() {
	std::unique_ptr<Engine> engine = Engine::Create(L"R:\\test_data_storage.dat");

	struct Metadata { ArrayIndex index; };
	Metadata metadata = engine->GetMetadata<Metadata>();

	Array<char> test_array(*engine, metadata.index);

	engine->SetMetadata<Metadata>(metadata);

	uint64 length = test_array.GetLength();

	TestArrayLength(test_array, 0);
	TestArrayLength(test_array, 1);
	TestArrayLength(test_array, 9);
	TestArrayLength(test_array, 256);
	TestArrayLength(test_array, 1023);
	TestArrayLength(test_array, 4096);
	TestArrayLength(test_array, 5000);
	TestArrayLength(test_array, 4096 * 512 * 256 + 4096 * 500);
}