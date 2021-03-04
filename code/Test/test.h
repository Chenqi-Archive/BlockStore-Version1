#pragma once

#include "../DynamicStore/engine.h"

#include <string>
#include <list>
#include <vector>
#include <iterator>


using namespace DynamicStore;

using std::wstring;
using std::list;
using std::vector;


Engine engine;

struct ListData {
	Index list_index;
	bool Check() const {
		return list_index != invalid_index;
	}
	void Init(Engine& engine) {
		list_index = engine.CreateArray();
	}
};


class TextEditor /*: public ListLayout*/ {
private:
	struct ParagraphData {
		unsigned int indent_level = 0;
		char32_t label = U'\0';
		Index text_array_index = invalid_index;
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
		Paragraph(ParagraphData data): _data(data), _text_array(engine, data.text_array_index) {
			_text_array.Load(0, _text_array.GetLength(), std::back_inserter(GetText()));
		}
		~Paragraph() {
			_text_array.SetLength(GetText().length());
			_text_array.Store(GetText().begin(), 0, _text_array.GetLength());
		}
		operator ParagraphData() { return _data; }
	};
private:
	list<Paragraph> _paragraph_list;
	Array<ParagraphData> _paragraph_array;
public:
	TextEditor(ListData data) : _paragraph_array(engine, data.list_index) {
		vector<ParagraphData> vector; vector.reserve(_paragraph_array.GetLength());
		_paragraph_array.Load(std::back_inserter(vector));
		for (auto& data : vector) {
			_paragraph_list.emplace_back(data);
			_paragraph_list.back()._index = --_paragraph_list.end();
		}
		// Create the first paragraph if there's no paragraph.
		if (_paragraph_list.empty()) {
			ParagraphData data;
			data.text_array_index = engine.CreateArray();
			_paragraph_list.emplace_back(data);
		}
	}
	~TextEditor() {
		_paragraph_array.SetLength(_paragraph_list.size());
		_paragraph_array.Store(_paragraph_list.begin());
	}
};


int main() {
	engine.Load(L"text.dat");
	ListData list_data = engine.GetMetadata<ListData>();
	TextEditor text_editor(list_data);
}