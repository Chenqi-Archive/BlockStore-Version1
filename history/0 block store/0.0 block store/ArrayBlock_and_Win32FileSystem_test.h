#pragma once

#include "../BlockStore/win32_file_system.h"
#include "../BlockStore/array_block.h"

#include <iostream>


using namespace BlockStore;
using namespace std;


class Utf16TextFile : public ArrayBlock<wchar> {
public:
	Utf16TextFile(Handle handle) : ArrayBlock(handle) {
		if (IsEmpty()) { Initialize(); }
	}

private:
	void Initialize() {
		static const wstring initial_content = L"initialize";
		Append(initial_content.data(), initial_content.length());
	}
};


int main() {
	Utf16TextFile txt_file = win32_file_system.OpenFile(L"test.txt");

	uint64 length = txt_file.GetLength();
	cout << length << endl;

	for (auto& ch : txt_file) { cout << ch; }
	cout << endl;

	wstring content_to_append = L"append";
	txt_file.Append(content_to_append.data(), content_to_append.length());
	for (uint64 index = 0; index < length; index++) { cout << txt_file[index]; }
	cout << endl;
}