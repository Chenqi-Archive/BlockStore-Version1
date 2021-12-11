#pragma once

#ifdef DYNAMICSTORE_EXPORTS							
#define DYNAMICSTORE_API __declspec(dllexport)		
#else
#define DYNAMICSTORE_API __declspec(dllimport)		
#endif


#define __ToString(name) #name
#define _ToString(name) __ToString(name)
#define Remark	__FILE__ "(" _ToString(__LINE__) "): [" __FUNCTION__ "] Remark: "


#define BEGIN_NAMESPACE(name) namespace name {
#define END_NAMESPACE(name)   }
#define Anonymous


#define ABSTRACT_BASE _declspec(novtable)
#define pure = 0


#include <stdexcept>
#include <cassert>


BEGIN_NAMESPACE(DynamicStore)


template<class T>
using ref_ptr = T*;

template<class T>
using alloc_ptr = T*;


using uint = unsigned int;
using uint64 = unsigned long long;
using wchar = wchar_t;


END_NAMESPACE(DynamicStore)