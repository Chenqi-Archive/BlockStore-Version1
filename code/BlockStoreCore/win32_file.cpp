#include "win32_file.h"

#include <Windows.h>


BEGIN_NAMESPACE(BlockStoreCore)


BOOL result = TRUE;

static_assert(sizeof(LARGE_INTEGER) == sizeof(uint64));


Win32File::Win32File(const wchar file[], CreateMode create_mode, AccessMode access_mode, ShareMode share_mode) :
	_file(INVALID_HANDLE_VALUE), _size(0), _create_mode(create_mode), _access_mode(access_mode), _share_mode(share_mode),
	_file_map(NULL), _map_view_address(nullptr) {

	_file = CreateFileW(file, (DWORD)access_mode, (DWORD)share_mode, NULL, (DWORD)create_mode, FILE_ATTRIBUTE_NORMAL, NULL);
	if (_file == INVALID_HANDLE_VALUE) { throw std::invalid_argument("open file failure"); }
	result = GetFileSizeEx(_file, (PLARGE_INTEGER)&_size);
	if (result != TRUE) { CloseHandle(_file); _file = INVALID_HANDLE_VALUE; throw std::runtime_error("unknown error"); }
}

Win32File::~Win32File() {
	UndoMapping();
	CloseHandle(_file);
}

void Win32File::SetSize(uint64 size) {
	UndoMapping();
	result = SetFilePointerEx(_file, (LARGE_INTEGER&)size, NULL, FILE_BEGIN); 
	if (result != TRUE) { throw std::runtime_error("unknown error"); }
	result = SetEndOfFile(_file);
	if (result != TRUE) { throw std::runtime_error("unknown error"); }
	_size = size;
}

void Win32File::DoMapping() {
	if (IsMapped()) { return; }
	if (_size == 0) { throw std::invalid_argument("cannot map an empty file"); }
	_file_map = ::CreateFileMappingW(_file, NULL, _access_mode == AccessMode::ReadOnly ? PAGE_READONLY : PAGE_READWRITE, 0, 0, NULL);
	if (_file_map == NULL) { throw std::runtime_error("unknown error"); }
	_map_view_address = ::MapViewOfFile(_file_map, _access_mode == AccessMode::ReadOnly ? FILE_MAP_READ : (FILE_MAP_READ | FILE_MAP_WRITE), 0, 0, 0);
	if (_map_view_address == nullptr) { CloseHandle(_file_map); _file_map = NULL; throw std::runtime_error("unknown error"); }
}

void Win32File::UndoMapping() {
	if (_map_view_address != nullptr) { UnmapViewOfFile(_map_view_address); _map_view_address = nullptr; }
	if (_file_map != NULL) { CloseHandle(_file_map); _file_map = NULL; }
}


END_NAMESPACE(BlockStoreCore)