

	template<class OutputIterator>
	void Load(uint64 begin, uint64 length, OutputIterator it) {
		if (length == 0) { return; }
		if (begin + length > _length) { throw std::out_of_range("subscript out of range"); }
	#error get unwrapped iterator (for vector or array)
		ElementType* temp_array = new ElementType[length];
		Load(begin, length, temp_array);
		std::copy(temp_array, temp_array + length, it);
		delete[] temp_array;
	}


	template<class OutputIterator>
	void Load(OutputIterator it) {
		Load(0, _length, it);
	}

	template<class InputIterator>
	void Store(InputIterator it_first, InputIterator it_last, uint64 begin) {
		std::vector<ElementType> tmp;
		std::copy(it_first, it_last, std::back_inserter(tmp));
		Store(tmp.data(), tmp.size(), begin);
	}

	template<class InputIterator>
	void Store(InputIterator it_first, InputIterator it_last) {
		Store(it_first, it_last, 0);
	}

	void Store(ElementType* buffer, uint64 length) {
		Store(buffer, length, 0);
	}
	8
	void Store(ElementType* buffer) {
		Store(buffer, _length, 0);
	}