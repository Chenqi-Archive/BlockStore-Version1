#pragma once

#include "../DynamicStore/DynamicStore.h"

#include <vector>


using namespace DynamicStore;


int main() {
	std::unique_ptr<Engine> engine = Engine::Create(L"R:\\test_array_length.dat");

	struct Metadata { ArrayIndex index; };
	Metadata metadata = engine->GetMetadata<Metadata>();

	Array<char> test_array(*engine, metadata.index);
	uint64 length = test_array.GetLength();

	test_array.SetLength(0);
	test_array.SetLength(1);
	test_array.SetLength(9);
	test_array.SetLength(256);
	test_array.SetLength(1023);
	test_array.SetLength(4096);
	test_array.SetLength(500);
	test_array.SetLength(2000);
	test_array.SetLength(0);
	test_array.SetLength(80);

	// L4096Plus level test							   level 0       1       2       3       4
	test_array.SetLength(5000);							  // 2	     1	    	       	     
	test_array.SetLength(4096 * 512);					  // 512     1	    	       	     
	test_array.SetLength(4096 * 512 * 200 + 1);			  // 102401	 201     1	    	       	     
	test_array.SetLength(4096 * 512 * 256 + 1);			  // 131073	 257     1	    	       	     
	test_array.SetLength(4096 * 512 * 256 + 4096 * 500);  // 131573	 257     1	    	       	     
	
	test_array.SetLength(4096 * 512 * 256 + 1);			  // 131073	 257     1	    	       	     
	test_array.SetLength(4096 * 512 * 200 + 1);			  // 102401	 201     1	    	       	     
	test_array.SetLength(4096 * 512);					  // 512     1	    	       	     
	test_array.SetLength(5000);							  // 2	     1	    	       	     
	
	test_array.SetLength(80);
	test_array.SetLength(4096 * 512 * 256 + 4096 * 500);  // 131573	 257     1	    	       	     
	test_array.SetLength(2000);

	engine->SetMetadata<Metadata>(metadata);
}