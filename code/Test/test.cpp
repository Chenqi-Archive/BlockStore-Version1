#include "../BlockStore/win32.h"

#include <iostream>


using namespace BlockStore;
using namespace std;


#ifdef _DEBUG
#pragma comment(lib, "../build/x64/Debug/BlockStore.lib")
#else
#pragma comment(lib, "../build/x64/Release/BlockStore.lib")
#endif // _DEBUG


class TxtFile : public Block {
public:
	TxtFile(Handle handle) : Block(handle) {}

	uint64 GetLength() const { return GetSize() / sizeof(wchar); }

	wchar operator[](uint64 index) const {
		wchar ch;
		Read(index * sizeof(wchar), sizeof(wchar), &ch);
		return ch;
	}

	iterator begin() {}
	iterator end() {}
};


int main() {
	Win32 system_block;
	TxtFile txt_file = system_block.OpenChild(L"test.txt");
	uint64 length = txt_file.GetLength();
	cout << length << endl;
	for (uint64 index = 0; index < length; index++) {
		cout << txt_file[index];
	}
	cout << endl;
}