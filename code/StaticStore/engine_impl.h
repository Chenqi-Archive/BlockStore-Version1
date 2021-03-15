#pragma once

#include "engine_impl_format.h"
#include "../BlockStoreCore/win32_file.h"


BEGIN_NAMESPACE(StaticStore)


class EngineImpl : public Engine {
public:
	EngineImpl(const wchar file[]);
	virtual ~EngineImpl() override;

	//// system file api ////
private:
	Win32File _file;



};


END_NAMESPACE(StaticStore)