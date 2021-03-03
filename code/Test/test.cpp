#include "test.h"


#ifdef _DEBUG
#pragma comment(lib, "../build/x64/Debug/DynamicStore.lib")
#else
#pragma comment(lib, "../build/x64/Release/DynamicStore.lib")
#endif // _DEBUG