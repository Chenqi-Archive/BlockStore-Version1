#pragma once

#include "core.h"

#include <vector>


BEGIN_NAMESPACE(BlockStoreCore)


// AnonymouseFile: a vector<char> wrapper with interface similar to Win32File.
class AnonymousFile {
public:
	AnonymousFile(const wchar file[]) {}
	~AnonymousFile() {}
private:
	std::vector<char> _data;
public:
	uint64 GetSize() const { return _data.size(); }
	void SetSize(uint64 size) { _data.resize(size); }
public:
	void DoMapping() {}
	void UndoMapping() {}
	bool IsMapped() const { return true; }
	const void* GetMapViewAddress() const { return _data.data(); }
	void* GetMapViewAddress() { return const_cast<void*>(const_cast<const AnonymousFile*>(this)->GetMapViewAddress()); }
};


END_NAMESPACE(BlockStoreCore)