#include "engine_impl.h"


#pragma comment(lib, "BlockStoreCore.lib")


BEGIN_NAMESPACE(StaticStore)


STATICSTORE_API std::unique_ptr<Engine> Engine::Create(const wchar file[]) {
	return std::make_unique<EngineImpl<Win32File>>(file);
}

STATICSTORE_API std::unique_ptr<Engine> Engine::Create(const void* data, uint64 size) {
	auto engine = std::make_unique<EngineImpl<AnonymousFile>>(L"");
	engine->SetRawData(data, size);
	return engine;
}


END_NAMESPACE(StaticStore)