#pragma once

#include "engine_impl_format.h"
#include "../BlockStoreCore/win32_file.h"


BEGIN_NAMESPACE(StaticStore)


class EngineImpl : public Engine {
public:
	EngineImpl(const wchar file[]);
	virtual ~EngineImpl() override;

	//// system file api ////
private:
	Win32File _file;


private:
	virtual void GetMetadata(void* data, uint64 size) const override {}
	virtual uint64 GetArraySize(ArrayIndex index) const override { return 0; }
	virtual void ReadArray(ArrayIndex index, uint64 offset, uint64 size, void* data) const override {}


	//// save operations ////
private:
	virtual void Format(const void* metadata, uint64 size) override {}
	virtual ArrayIndex CreateArray(uint64 array_size) override { return {}; }
	virtual void WriteArray(ArrayIndex index, const void* data, uint64 size, uint64 offset) override {}
};


END_NAMESPACE(StaticStore)