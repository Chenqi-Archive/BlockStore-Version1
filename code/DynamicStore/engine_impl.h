#pragma once

#include "engine_impl_format.h"


BEGIN_NAMESPACE(DynamicStore)


class EngineImpl : public Engine {
public:
	EngineImpl(const wchar file[]);
	virtual ~EngineImpl() override;


	// system file api
private:
	using HANDLE = void*;
	HANDLE _file;
	uint64 _size;
private:
	void SetSize(uint64 size);


	//// format and metadata ////
private:
	bool IsFormatted() {}
public:
	virtual void Format() override {}
private:
	StaticMetadata _metadata;
private:
	virtual void LoadMetadata(void* data, uint64 size) const override {}
	virtual void StoreMetadata(const void* data, uint64 size) override {}


	//// array ////	
public:
	virtual ArrayIndex CreateArray() override { return 0; }
	virtual void DestroyArray(ArrayIndex index) override {}
	virtual uint64 GetArraySize(ArrayIndex index) const override { return 0; }
	virtual void SetArraySize(ArrayIndex index, uint64 size) override {}
	virtual void ReadArray(ArrayIndex index, uint64 offset, uint64 size, void* data) const override {}
	virtual void WriteArray(ArrayIndex index, const void* data, uint64 size, uint64 offset) override {}
};


END_NAMESPACE(DynamicStore)