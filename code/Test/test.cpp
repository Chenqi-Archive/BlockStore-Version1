#include "test_static_serialize.h"


#ifdef _DEBUG
#pragma comment(lib, "../build/x64/Debug/DynamicStore.lib")
#pragma comment(lib, "../build/x64/Debug/StaticStore.lib")
#else
#pragma comment(lib, "../build/x64/Release/DynamicStore.lib")
#pragma comment(lib, "../build/x64/Release/StaticStore.lib")
#endif // _DEBUG