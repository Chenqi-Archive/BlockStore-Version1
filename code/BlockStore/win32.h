#pragma once

#include "block.h"

#include <string>


BEGIN_NAMESPACE(BlockStore)

using std::wstring;


class Win32 : public Block {
private:
	virtual uint64 GetChildSize(uint64 child_index) {}
	virtual void ReadChild(uint64 child_index, uint64 begin, uint64 length, void* data) {}
	virtual void WriteChild(uint64 child_index, void* data, uint64 length, uint64 begin) {}
	virtual void SetChildSize(uint64 child_index, uint64 size) {}
	virtual void DestroyChild(uint64 child_index) {}
public:
	bool HasChild(const wstring& child_name) {}
	Handle CreateChild(const wstring& child_name) {}
	Handle OpenChild(const wstring& child_name) {}
	void RenameChild(uint64 child_index, const wstring& child_name) {}
};


END_NAMESPACE(BlockStore)