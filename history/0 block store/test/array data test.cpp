


constexpr uint64 test_data_size = 50000;


std::vector<char> InitializeTestData() {
	std::vector<char> ret(test_data_size);
	char ch = 'a';
	for (uint64 i = 0; i < test_data_size; ++i) {
		ret[i] = ch;
		ch++; if (ch > 'z') { ch = 'a'; }
	}
	return ret;
}


void TestArrayLength(Array<char>& array, uint64 length) {
	static std::vector<char> test_data = InitializeTestData();
	static std::vector<char> test_data_copy(test_data_size);

	array.SetLength(length);

	for (uint64 current_begin = 0; current_begin < length; current_begin += test_data_size) {
		uint64 current_length = std::min(length - current_begin, test_data_size);
		array.Store(test_data.data(), current_length, current_begin);
		array.Load(current_begin, current_length, test_data_copy.data());
		for (uint64 i = 0; i < test_data_size; ++i) {
			assert(test_data[i] == test_data_copy[i]);
		}
	}
}