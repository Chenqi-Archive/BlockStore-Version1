#pragma once

#include "../StaticStore/StaticStore.h"

#include <string>
#include <list>
#include <vector>


using namespace StaticStore;

using std::wstring;
using std::list;
using std::vector;


Engine& GetEngine() {
	static std::unique_ptr<Engine> engine(Engine::Create(L"R:\\test_static_TextEditor.dat"));
	return *engine;
}

struct ParagraphData {
	unsigned int _indent_level = 0;
	char32_t _label = U'\0';
	ArrayIndex<wchar> _text_array_index;
};

struct TextEditorData {
	ArrayIndex<ParagraphData> _paragraph_list_index;
};

class TextEditor : public /*ListLayout,*/ TextEditorData {
private:

	class Paragraph : public /*TextBox,*/ ParagraphData {
	private:
		friend class TextEditor;
		TextEditor& _text_editor;
		list<Paragraph>::iterator _list_position;

		Array<wchar> _text_array;
		wstring _text;

	public:
		Paragraph(TextEditor& text_editor) :
			ParagraphData(), _text_editor(text_editor), _text_array(GetEngine(), _text_array_index) {
			static const wchar initial_text[] = L"Paragraph Created!";
			_text = initial_text;
		}
		Paragraph(TextEditor& text_editor, ParagraphData data) :
			ParagraphData(data), _text_editor(text_editor), _text_array(GetEngine(), _text_array_index) {
			auto [text, length] = _text_array.Load();
			_text.assign(text, length);
		}
		void Save() {
			_text_array.Save(_text.data(), _text.length());
		}
	};

private:
	Array<ParagraphData> _paragraph_array;
	list<Paragraph> _paragraph_list;

public:
	//TextEditor() : TextEditorData(), _paragraph_array(GetEngine(), _paragraph_list_index) {}
	TextEditor(TextEditorData data) : TextEditorData(data), _paragraph_array(GetEngine(), _paragraph_list_index) {
		auto [paragraph_data, length] = _paragraph_array.Load();
		for (uint64 i = 0; i < length; ++i) {
			_paragraph_list.emplace_back(*this, paragraph_data[i]);
			_paragraph_list.back()._list_position = --_paragraph_list.end();
		}
	}
	void Save() {
		vector<ParagraphData> tmp_buffer; tmp_buffer.reserve(_paragraph_list.size());
		for (auto& paragraph : _paragraph_list) {
			paragraph.Save();
			tmp_buffer.push_back(paragraph);
		}
		_paragraph_array.Save(tmp_buffer.data(), tmp_buffer.size());
	}

	void InsertParagraphBefore(const Paragraph& paragraph) {
		_paragraph_list.emplace(paragraph._list_position, *this);
	}
	void EraseParagraphs(const Paragraph& paragraph_first, const Paragraph& paragraph_last) {
		auto it_first = paragraph_first._list_position, it_last = paragraph_last._list_position; it_last++;
		for (auto it = it_first; it != it_last && it != _paragraph_list.end(); ) {
			_paragraph_list.erase(it++);
		}
	}
};


int main() {
	TextEditor text_editor(GetEngine().GetMetadata<TextEditorData>());

	// do some operations on text paragraphs


	GetEngine().Format();
	text_editor.Save();
	GetEngine().SetMetadata<TextEditorData>(text_editor);
}