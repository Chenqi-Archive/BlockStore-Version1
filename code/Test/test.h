#pragma once

#include "../DynamicStore/DynamicStore.h"

#include <string>
#include <list>
#include <vector>
#include <iterator>


using namespace DynamicStore;

using std::wstring;
using std::list;
using std::vector;


Engine& GetEngine() {
	static std::unique_ptr<Engine> engine(Engine::Create(L"test.dat"));
	return *engine;
}

struct ListData {
	ArrayIndex list_index;
	void Initialize(Engine& engine) { list_index = engine.CreateArray(); }
	bool CheckError() const { return list_index == invalid_array_index; }
};


class TextEditor /*: public ListLayout*/ {
private:
	struct ParagraphData {
		unsigned int indent_level = 0;
		char32_t label = U'\0';
		ArrayIndex text_array_index = invalid_array_index;
	};
	class Paragraph /*: public TextBox*/ {
	private:
		friend class TextEditor;
		list<Paragraph>::iterator _index;
		ParagraphData _data;
		Array<wchar> _text_array;
		///*
		wstring _text;
		wstring& GetText() { return _text; }
		//*/
	public:
		Paragraph(ParagraphData data): _data(data), _text_array(GetEngine(), data.text_array_index) {
			_text_array.Load(0, _text_array.GetLength(), std::back_inserter(GetText()));
		}
		~Paragraph() {
			_text_array.SetLength(GetText().length());
			_text_array.Store(GetText().begin(), _text_array.GetLength(), 0);
		}
		operator ParagraphData() { return _data; }
	};
private:
	Array<ParagraphData> _paragraph_array;
	list<Paragraph> _paragraph_list;
public:
	TextEditor(ListData data) : _paragraph_array(GetEngine(), data.list_index) {
		vector<ParagraphData> vector; vector.reserve(_paragraph_array.GetLength());
		_paragraph_array.Load(std::back_inserter(vector));
		for (auto& data : vector) {
			_paragraph_list.emplace_back(data);
			_paragraph_list.back()._index = --_paragraph_list.end();
		}
		// Create the first paragraph if there's no paragraph.
		if (_paragraph_list.empty()) {
			ParagraphData data;
			data.text_array_index = GetEngine().CreateArray();
			_paragraph_list.emplace_back(data);
		}
	}
	~TextEditor() {
		_paragraph_array.SetLength(_paragraph_list.size());
		_paragraph_array.Store(_paragraph_list.begin());
	}
};


int main() {
	Engine& engine = GetEngine();
	ListData data = engine.GetMetadata<ListData>();
	TextEditor text_editor(data);
}