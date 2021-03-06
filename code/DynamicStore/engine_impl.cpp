#include "engine_impl.h"


BEGIN_NAMESPACE(DynamicStore)


DYNAMICSTORE_API std::unique_ptr<Engine> Engine::Create(const wchar file[]) {
	return std::make_unique<EngineImpl>(file);
}




END_NAMESPACE(DynamicStore)