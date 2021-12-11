#pragma once

#include "../BlockStoreCore/core.h"


#ifdef STATICSTORE_EXPORTS							
#define STATICSTORE_API __declspec(dllexport)		
#else
#define STATICSTORE_API __declspec(dllimport)		
#endif


BEGIN_NAMESPACE(StaticStore)

using namespace BlockStoreCore;

END_NAMESPACE(StaticStore)