#pragma once

#include <list>
#include <vector>
#include <stdexcept>
#include <memory>
#include <string>

#define pure = 0

using namespace std;

class Block {

};

class StorageSystem {
private:
	StorageSystem* parent_system;


public:
	// For myself.
	size_t GetSize();
	virtual void Resize(size_t new_size);  // Enlarge or Shrink.

	virtual void Read(size_t offset, size_t length, void* data) throw (std::invalid_argument);
	virtual void Write(size_t offset, size_t length, void* data) throw (std::invalid_argument);

protected:
	// For child management.
	virtual void ResizeChild();
	virtual void CreateChild();
	virtual void RemoveChild();
};


// Storage system using system APIs.
class SysStorageSystem : StorageSystem {

};


class StorageSingleBlockConstantSize : StorageSystem {

};

using uint = unsigned int;


template<class Index>
class BlockStoreBase {
public:
	using IndexType = Index;  // could be uint, string, etc
	using Handle = void*;  // 

	BlockStoreBase parent;
	Handle handle;  // used by parent.
	uint size;
	IndexType index;

public:
	virtual unique_ptr<BlockStoreBase> Create(uint initial_length);
	virtual unique_ptr<BlockStoreBase> Open(IndexType index);
};

class SystemBlockStore : public BlockStoreBase<wstring> {

};


class CgaStore : public BlockStoreBase<uint> {



};


void load_file_system() {
	static SystemBlockStore win32_file_system;
	CgaStore cga_file = win32_file_system.Open(L"store.cga");
	BlockTable block_table = cga_file.Open(block_table_index);


}



class Win32FileSystem {
	using IndexType = wstring;

	void Open();
};


class FileStore {

};

class BinaryStore {

};

class BinaryStore;
class BinStore;
class BinStore;
class BinaryStore;
BinaryStore;
namespace BinStore {

}
namespace BinaryStore {

}


}


////
#include <type_traits>
#include <variant>



template<class T>
struct Ref {};


template<class T1, class T2>
struct Pair {};


//using Node = Pair<Ref<Node>, Ref<Node>>; //error

//struct Node;
//struct Node : public Pair<Ref<Node>, Ref<Node>> {}; //ok


struct Node {
	using storage = Pair<Ref<Node>, Ref<Node>>;
	void store() {

	}

	Node* left;
	Node* right;
};


int main() {

}