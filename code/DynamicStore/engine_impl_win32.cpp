#include "engine_impl.h"

#include <Windows.h>


BEGIN_NAMESPACE(DynamicStore)


EngineImpl::EngineImpl(const wchar file[]) :_file(INVALID_HANDLE_VALUE), _size(0) {
	_file = CreateFileW(file, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (_file == INVALID_HANDLE_VALUE) { throw std::invalid_argument("open file error"); }
	if (!GetFileSizeEx(_file, (PLARGE_INTEGER)&_size)) { CloseHandle(_file); assert(false); }
	if (!IsFormatted()) { Format(); }
}

EngineImpl::~EngineImpl() {
	CloseHandle(_file);
}

void EngineImpl::SetSize(uint64 size) {
	if (SetFilePointerEx(_file, { size }, NULL, FILE_BEGIN) == false) { assert(false); }
	if (SetEndOfFile(_file) == false) { assert(false); }
	_size = size;
}


END_NAMESPACE(DynamicStore)