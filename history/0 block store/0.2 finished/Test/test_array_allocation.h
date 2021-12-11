#pragma once

#include "../DynamicStore/DynamicStore.h"

#include <random>
#include <vector>


using namespace DynamicStore;


int main() {
	std::unique_ptr<Engine> engine = Engine::Create(L"R:\\test_array_allocation.dat");

	struct Metadata { ArrayIndex index[10]; };
	Metadata metadata = engine->GetMetadata<Metadata>();

	struct Element { uint64 data[3]; };
	std::vector<Array<Element>> test_array_vector; test_array_vector.reserve(10);

	for (int i = 0; i < 10; ++i) { 
		test_array_vector.emplace_back(*engine, metadata.index[i]); 
		test_array_vector[i].SetLength(rand());
	}

	for (int i = 0; i < 10; ++i) {
		test_array_vector[i].Destroy();
	}
	test_array_vector.clear();

	engine->SetMetadata<Metadata>(metadata);
}