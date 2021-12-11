#include "win32_file_system.h"


BEGIN_NAMESPACE(BlockStore)


Win32FileSystem& win32_file_system = Win32FileSystem::Get();

Win32FileSystem& Win32FileSystem::Get() {
	static Win32FileSystem win32_file_system;
	return win32_file_system;
}


uint64 Win32FileSystem::GetChildSize(uint64 child_index) {}

void Win32FileSystem::ReadChild(uint64 child_index, uint64 begin, uint64 length, void* data) {}

void Win32FileSystem::WriteChild(uint64 child_index, void* data, uint64 length, uint64 begin) {}

void Win32FileSystem::SetChildSize(uint64 child_index, uint64 size) {}

void Win32FileSystem::DestroyChild(uint64 child_index) {}

void* Win32FileSystem::MapViewChild(uint64 child_index, uint64 begin, uint64 length) { return nullptr; }

void Win32FileSystem::UnmapViewChild(uint64 child_index, uint64 begin, uint64 length, void* data) {}

bool Win32FileSystem::HasChild(const wstring& child_name) {}

Handle Win32FileSystem::CreateChild(const wstring& child_name) {}

Handle Win32FileSystem::OpenFile(const wstring& child_name) {}

void Win32FileSystem::RenameChild(uint64 child_index, const wstring& child_name) {}


END_NAMESPACE(BlockStore)