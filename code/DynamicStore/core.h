#pragma once

#include "../BlockStoreCore/core.h"


#ifdef DYNAMICSTORE_EXPORTS							
#define DYNAMICSTORE_API __declspec(dllexport)		
#else
#define DYNAMICSTORE_API __declspec(dllimport)		
#endif


BEGIN_NAMESPACE(DynamicStore)

using namespace BlockStoreCore;

END_NAMESPACE(DynamicStore)