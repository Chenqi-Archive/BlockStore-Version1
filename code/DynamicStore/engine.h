#pragma once

#include "core.h"
#include "uncopyable.h"

#include <memory>


BEGIN_NAMESPACE(DynamicStore)

using ArrayIndex = uint64;
const ArrayIndex invalid_array_index = (ArrayIndex)-1;


struct ABSTRACT_BASE Engine {
public:
	DYNAMICSTORE_API static std::unique_ptr<Engine> Create(const wchar file[]);
	virtual ~Engine() pure;

	//// format and metadata ////
public:
	virtual void Format() pure;
public:
	static constexpr uint64 max_metadata_size = 256;
private:
	virtual void LoadMetadata(void* data, uint64 size) const pure;
	virtual void StoreMetadata(void* data, uint64 size) pure;
public:
	// Metadata must implement "void Initialize(Engine& engine)" and "bool CheckError()" function.
	template<class Metadata>
	void GetMetadata(Metadata& metadata) const;
	template<class Metadata>
	void SetMetadata(const Metadata& metadata);
	template<class Metadata>
	Metadata GetMetadata() const { Metadata metadata; GetMetadata(metadata); return metadata; }

	//// array operations ////
public:
	virtual ArrayIndex CreateArray() pure;
	virtual void DestroyArray(ArrayIndex index) pure;
	virtual uint64 GetArraySize(ArrayIndex index) const pure;
	virtual void SetArraySize(ArrayIndex index, uint64 size) pure;
	virtual void ReadArray(ArrayIndex index, uint64 offset, uint64 size, void* data) const pure;
	virtual void WriteArray(ArrayIndex index, void* data, uint64 size, uint64 offset) pure;
};


template<class Metadata>
inline void Engine::GetMetadata(Metadata& metadata) const {
	static_assert(sizeof(Metadata) <= max_metadata_size);
	LoadMetadata(&metadata, sizeof(Metadata));
	if (metadata.CheckError() == true) {
		Format();
		metadata.Initialize();
		SetMetadata(metadata);
	}
}

template<class Metadata>
inline void Engine::SetMetadata(const Metadata& metadata) {
	static_assert(sizeof(Metadata) <= max_metadata_size);
	StoreMetadata(&metadata, sizeof(Metadata));
}


END_NAMESPACE(DynamicStore)