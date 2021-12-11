#pragma once

#include "core.h"


BEGIN_NAMESPACE(BlockStore)

class Block;


class Handle {
private:
	ref_ptr<Block> parent;
	uint64 index;

	alloc_ptr<void> mapped_data;  /* the data may be allocated by parent */
	uint64 mapped_begin;
	uint64 mapped_length;

public:
	Handle(ref_ptr<Block> parent, uint64 index) :
		parent(parent), index(index), mapped_data(nullptr), mapped_begin(-1), mapped_length(0) {
		
	}
	~Handle() { assert(!IsMapped()); }

	bool IsValid() const { return parent != nullptr; }
	uint64 GetSize() const;
	void SetSize(uint64 size);
	void Read(uint64 begin, uint64 length, void* data) const;
	void Write(void* data, uint64 length, uint64 begin) const;

	bool IsMapped() const { return mapped_data != nullptr; }
	void MapView(uint64 begin, uint64 length);
	void UnmapView();
};


class Block /*: public Uncopyable*/ {
protected:
	Block(Handle handle) : handle(handle) { if (!handle.IsValid()) { throw std::invalid_argument("invalid handle"); } }
	~Block() { if (IsMapped()) { UnmapView(); } }

protected:
	Handle handle;

protected:
	uint64 GetSize() const { return handle.GetSize(); }
	bool IsEmpty() const { return GetSize() == 0; }
	void SetSize(uint64 size) { handle.SetSize(size); }
	void Read(uint64 begin, uint64 length, void* data) const { handle.Read(begin, length, data); }
	void Write(void* data, uint64 length, uint64 begin) const { handle.Write(data, length, begin); }

	bool IsMapped() const { return handle.IsMapped(); }
	void MapView(uint64 begin, uint64 length) { handle.MapView(begin, length); }
	void MapView() { MapView(0, GetSize()); }
	void UnmapView() { handle.UnmapView(); }

private:
	friend class Handle;
	virtual uint64 GetChildSize(uint64 child_index) {}
	virtual void SetChildSize(uint64 child_index, uint64 size) {}
	virtual void ReadChild(uint64 child_index, uint64 begin, uint64 length, void* data) {}
	virtual void WriteChild(uint64 child_index, void* data, uint64 length, uint64 begin) {}
#error provide by ?
	virtual void DestroyChild(uint64 child_index) {} 
	virtual void* MapViewChild(uint64 child_index, uint64 begin, uint64 length) { return nullptr; }
#error overwrite dirty data
	virtual void UnmapViewChild(uint64 child_index, uint64 begin, uint64 length, void* data) {}
};


inline uint64 Handle::GetSize() const { return parent->GetChildSize(index); }
inline void Handle::SetSize(uint64 size) { parent->SetChildSize(index, size); }
inline void Handle::Read(uint64 begin, uint64 length, void* data) const { 
	if (IsMapped()) {

	}

	// Read the remaining data.
	parent->ReadChild(index, begin, length, data);
}
inline void Handle::Write(void* data, uint64 length, uint64 begin) const { 
	if (IsMapped()) {

#error dirty tag and flush
	}

	parent->WriteChild(index, data, length, begin); 
}
inline void Handle::MapView(uint64 begin, uint64 length) {
	if (IsMapped() && begin == mapped_begin && length == mapped_length) { return; }
	UnmapView();
	mapped_data = parent->MapViewChild(index, begin, length); 
}
inline void Handle::UnmapView() { 
	if (!IsMapped()) { return; }
	parent->UnmapViewChild(index, mapped_begin, mapped_length, mapped_data); 
}



template<class IndexType>
class BlockType : public Block {
public:
	BlockType(Handle handle) : Block(handle) {}

public:
	Handle CreateChild(IndexType index) { return Handle(nullptr); }
};


END_NAMESPACE(BlockStore)