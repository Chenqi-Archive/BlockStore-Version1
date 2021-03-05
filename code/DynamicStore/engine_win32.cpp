#include "engine.h"

#include <Windows.h>


BEGIN_NAMESPACE(DynamicStore)


Engine::Engine() :
	_file(INVALID_HANDLE_VALUE),
	_size(0) {
}

bool Engine::HasOpened() const { 
	return _file != INVALID_HANDLE_VALUE; 
}

void Engine::Open(const wchar file[]) {
	if (HasOpened()) { Close(); }
	_file = CreateFileW(file,
						GENERIC_READ | GENERIC_WRITE,
						NULL,
						NULL,
						OPEN_ALWAYS,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
	if (_file == INVALID_HANDLE_VALUE) { return; }
	if (!GetFileSizeEx(_file, (PLARGE_INTEGER)&_size)) { Close(); return; }
}

void Engine::Close() {
	if (_file != INVALID_HANDLE_VALUE) { CloseHandle(_file); _file = INVALID_HANDLE_VALUE; }
	_size = 0;
}

void Engine::SetSize(uint64 size) {
	assert(HasOpened());
	if (SetFilePointerEx(_file, { size }, NULL, FILE_BEGIN) == false) { assert(false); }
	if (SetEndOfFile(_file) == false) { assert(false); }
	_size = size;
}


END_NAMESPACE(DynamicStore)