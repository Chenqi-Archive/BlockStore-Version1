#include "test_static.h"


#ifdef _DEBUG
#pragma comment(lib, "../build/x64/Debug/StaticStore.lib")
#else
#pragma comment(lib, "../build/x64/Release/StaticStore.lib")
#endif // _DEBUG