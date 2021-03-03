#pragma once

#include "../DynamicStore/engine.h"

#include <string>


using namespace DynamicStore;

using std::wstring;


class Paragraph : public Array<wchar> {

};


class ParagraphList : public Array<Paragraph> {
	

};


class StoreEngine : public Engine {
public:

};


int main() {
	StoreEngine engine(L"text.dat");
	if (engine.IsEmpty()) {
		engine.CreateArray()
	}
}