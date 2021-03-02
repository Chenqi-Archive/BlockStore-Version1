#pragma once

#include "block.h"

#include <string>


BEGIN_NAMESPACE(BlockStore)

using std::wstring;


class Win32FileSystem : public Block {
private:
	Win32FileSystem() : Block(Handle((Block*)-1, 0)) {}
private:
	virtual uint64 GetChildSize(uint64 child_index);
	virtual void ReadChild(uint64 child_index, uint64 begin, uint64 length, void* data);
	virtual void WriteChild(uint64 child_index, void* data, uint64 length, uint64 begin);
	virtual void SetChildSize(uint64 child_index, uint64 size);
	virtual void DestroyChild(uint64 child_index);
	virtual void* MapViewChild(uint64 child_index, uint64 begin, uint64 length);
	virtual void UnmapViewChild(uint64 child_index, uint64 begin, uint64 length, void* data);
public:
	static Win32FileSystem& Get();
	bool HasChild(const wstring& child_name);
	Handle CreateChild(const wstring& child_name);
	Handle OpenFile(const wstring& child_name);
	void RenameChild(uint64 child_index, const wstring& child_name);
};

extern Win32FileSystem& win32_file_system;


END_NAMESPACE(BlockStore)