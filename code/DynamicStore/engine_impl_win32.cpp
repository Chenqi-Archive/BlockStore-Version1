#include "engine_impl.h"

#include <Windows.h>


BEGIN_NAMESPACE(DynamicStore)


EngineImpl::EngineImpl(const wchar file[]) :
	_file(INVALID_HANDLE_VALUE),
	_size(0),
	_file_map(NULL),
	_map_view_address(nullptr) {

	_file = CreateFileW(file, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (_file == INVALID_HANDLE_VALUE) { throw std::invalid_argument("open file error"); }
	if (GetFileSizeEx(_file, (PLARGE_INTEGER)&_size) == FALSE) { assert(false); }
	if (_size == 0) { 
		Format(); 
	} else {
		DoMapping();
		if (LoadAndCheck() == false) { 
			Format();
		}
	}
}

EngineImpl::~EngineImpl() {
	UndoMapping();
	CloseHandle(_file);
}

void EngineImpl::SetSize(uint64 size) {
	UndoMapping();
	if (SetFilePointerEx(_file, { size }, NULL, FILE_BEGIN) == false) { assert(false); }
	if (SetEndOfFile(_file) == false) { assert(false); }
	_size = size;
	DoMapping();
}

void EngineImpl::DoMapping() {
	_file_map = ::CreateFileMappingW(_file, NULL, PAGE_READWRITE, 0, 0, NULL);
	assert(_file_map != NULL);
	_map_view_address = ::MapViewOfFile(_file_map, (FILE_MAP_READ | FILE_MAP_WRITE), 0, 0, 0);
	assert(_map_view_address != nullptr);
}

void EngineImpl::UndoMapping() {
	UnmapViewOfFile(_map_view_address); _map_view_address = nullptr;
	CloseHandle(_file_map); _file_map = NULL;
}


END_NAMESPACE(DynamicStore)