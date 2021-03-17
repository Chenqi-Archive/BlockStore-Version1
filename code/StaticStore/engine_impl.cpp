#include "engine_impl.h"


#ifdef _DEBUG
#pragma comment(lib, "../build/x64/Debug/BlockStoreCore.lib")
#else
#pragma comment(lib, "../build/x64/Release/BlockStoreCore.lib")
#endif // _DEBUG


BEGIN_NAMESPACE(StaticStore)


STATICSTORE_API std::unique_ptr<Engine> Engine::Create(const wchar file[]) {
	return std::make_unique<EngineImpl>(file);
}


EngineImpl::EngineImpl(const wchar file[]) :_file(file) {}

EngineImpl::~EngineImpl() {}


END_NAMESPACE(StaticStore)