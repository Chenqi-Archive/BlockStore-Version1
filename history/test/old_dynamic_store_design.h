#pragma once
#include <string>
#include <vector>


using namespace std;

using wchar = wchar_t;
using uint = unsigned int;


class Manager {

protected:
	void OpenFile(const wchar file_name[]) {

	}
	void LoadMetaBlock(Block& block) {
		LoadBlock(0, block);
	}

	void TraceAllBlocks() {
		// Do Garbage Collection
		// depth search and mark
		// clean those unmarked blocks
	}
private:
	using BlockIndex = uint;
	void LoadBlock(BlockIndex index, Block& block) {
		//
	}
};


struct BlockClassTracker {
	void* class_id;  // the on_register virtual function address as key
	// if the virtual function is identical, then the class is no different in the view of Block

	vector<uint> child_index_offsets;
};


// if block contain child blocks (referenced in containers or unique pointers) (shared pointer?)
// the child block is stored as index
// else the block is a leaf block

class Block {
private:
	BlockIndex index;


public:
	virtual void OnRegister() {

	}

	void GetSize() {

	}

	bool IsInlined() {

	}

protected:
	void Register(wstring& text, wstring initial_text = L"") {
		AddIndex(Offset(text, this));

	}
};



class Page : public Block {
	wstring title;
	uint32_t time;
	vector<wstring> paragraphs;

	virtual void OnRegister() override {
		Register(title, L"Untitled");
		//Register(time, GetCurrentTime());
		//Register(paragraphs, { "Empty Paragraph" });
	}
};


class NoteBook : public Block {
	wstring title;
	vector<Page> pages;

	virtual void OnRegister() override {
		Register(title, L"Anonymous");
		//Register(pages, { Page() });

	}
	virtual void OnLoad() override {
		// load all at 
		// partial load
	}

	void LoadPage(uint page) {
		
	}
};



class GlobalManager : Manager {
	NoteBook notebook;

	void Load() {
		LoadMetaBlock(notebook);
	}
};