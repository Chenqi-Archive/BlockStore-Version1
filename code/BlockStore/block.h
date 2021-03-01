#pragma once

#include "core.h"


BEGIN_NAMESPACE(BlockStore)

class Block;


class Handle {
private:
	ref_ptr<Block> parent;
	uint64 index;

public:
	Handle(nullptr_t) :parent(nullptr), index((uint64)-1) {}
	Handle(ref_ptr<Block> parent, uint64 index) : parent(parent), index(index) {}

	bool IsValid() const { return parent != nullptr; }
	uint64 GetSize() const;
	void Read(uint64 begin, uint64 length, void* data) const;
	void Write(void* data, uint64 length, uint64 begin) const;

	void SetSize(uint64 size);
	void Destroy();
};


class ABSTRACT_BASE Block {
protected:
	Block(Handle handle) : handle(handle) { if (!handle.IsValid()) { throw std::invalid_argument("invalid child index"); } }
	virtual ~Block() pure {}

protected:
	Handle handle;

protected:
	bool IsValid() const { return handle.IsValid(); }
	uint64 GetSize() const { return handle.GetSize(); }
	void Read(uint64 begin, uint64 length, void* data) const { handle.Read(begin, length, data); }
	void Write(void* data, uint64 length, uint64 begin) const { handle.Write(data, length, begin); }

	void SetSize(uint64 size) { handle.SetSize(size); }
	void Destroy() { handle.Destroy(); }

private:
	friend class Handle;
	virtual uint64 GetChildSize(uint64 child_index) {}
	virtual void ReadChild(uint64 child_index, uint64 begin, uint64 length, void* data) {}
	virtual void WriteChild(uint64 child_index, void* data, uint64 length, uint64 begin) {}
	virtual void SetChildSize(uint64 child_index, uint64 size) {}
	virtual void DestroyChild(uint64 child_index) {}
};


inline uint64 Handle::GetSize() const { return parent->GetChildSize(index); }
inline void Handle::Read(uint64 begin, uint64 length, void* data) const { parent->ReadChild(index, begin, length, data); }
inline void Handle::Write(void* data, uint64 length, uint64 begin) const { parent->WriteChild(index, data, length, begin); }

inline void Handle::SetSize(uint64 size) { parent->SetChildSize(index, size); }
inline void Handle::Destroy() { parent->DestroyChild(index); *this = Handle(nullptr); }


template<class IndexType>
class BlockType : public Block {
public:
	BlockType(Handle handle) : Block(handle) {}

public:
	Handle CreateChild(IndexType index) { return Handle(nullptr); }
};


END_NAMESPACE(BlockStore)