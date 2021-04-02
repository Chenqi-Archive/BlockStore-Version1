#pragma once

#include "engine_impl_format.h"
#include "../BlockStoreCore/win32_file.h"
#include "../BlockStoreCore/anonymous_file.h"

#include <vector>


BEGIN_NAMESPACE(StaticStore)

using std::vector;


template<class FileType>
class EngineImpl : public Engine {
public:
	EngineImpl(const wchar file[]) : _file(file) {
		_file.DoMapping(); UpdateStaticMetadataAddress();
		if (LoadAndCheck() == false) { Format(); }
	}

	virtual ~EngineImpl() override {
		uint64 file_size = _file.GetSize();
		if (file_size > _static_metadata->used_size) {
			_file.SetSize(_static_metadata->used_size);
		}
	}


	//// system file api ////
private:
	FileType _file;
public:
	virtual std::pair<const void*, uint64> GetRawData() const override {
		return { _file.GetMapViewAddress(), _static_metadata->used_size };
	}
	void SetRawData(const void* raw_data, uint64 size) {
		_file.SetSize(size);
		_file.DoMapping(); UpdateStaticMetadataAddress();
		memcpy(_file.GetMapViewAddress(), raw_data, size);
		if (LoadAndCheck() == false) { Format(); }
	}


	//// format ////
public:
	virtual void Format() override {
		_file.SetSize(static_metadata_size);
		_file.DoMapping(); UpdateStaticMetadataAddress();
		_static_metadata->used_size = AlignOffset(_file.GetSize());
		_static_metadata->index_table_entry = { invalid_array_offset, 0 };
		_static_metadata->user_metadata_entry = { invalid_array_offset, 0 };
		_index_entry_table.clear();
	}
private:
	bool CheckIndexEntry(IndexEntry entry) {
		if (entry.array_size == 0) { return true; }
		uint64 used_size = _static_metadata->used_size;
		if (entry.array_offset >= used_size || entry.array_offset + entry.array_size > used_size) { return false; }
		return true;
	}
	bool LoadAndCheck() {
		uint64 file_size = _file.GetSize();
		if (file_size < static_metadata_size) { return false; }
		if (file_size < _static_metadata->used_size) { return false; }
		if (CheckIndexEntry(_static_metadata->index_table_entry) == false) { return false; }
		if (CheckIndexEntry(_static_metadata->user_metadata_entry) == false) { return false; }
		auto [data, size] = GetIndexEntryData(_static_metadata->index_table_entry);
		if (size % index_entry_size != 0) { return false; }
		_index_entry_table.resize(size / index_entry_size);
		memcpy(_index_entry_table.data(), data, size);
		return true;
	}


	//// static metadata management ////
private:
	StaticMetadata* _static_metadata = nullptr;
private:
	void UpdateStaticMetadataAddress() { _static_metadata = (StaticMetadata*)_file.GetMapViewAddress(); }
private:
	std::pair<const void*, uint64> GetIndexEntryData(IndexEntry entry) const {
		if (entry.array_offset == invalid_array_offset) { return { nullptr, 0 }; }
		return { (char*)_file.GetMapViewAddress() + entry.array_offset, entry.array_size };
	}
	const IndexEntry AllocateArray(const void* data, uint64 size) {
		uint64 offset = _static_metadata->used_size;
		uint64 file_size = _file.GetSize();
		if (offset + size > file_size) {
			_file.SetSize(RoundFileSizeUp(offset + size));
			_file.DoMapping(); UpdateStaticMetadataAddress();
		}
		_static_metadata->used_size = AlignOffset(offset + size);
		if (data != nullptr) { memcpy((char*)_file.GetMapViewAddress() + offset, data, size); }
		return { offset, size };
	}
private:
	bool IsMetadataSet() const {
		return _static_metadata->user_metadata_entry.array_offset != invalid_array_offset;
	}
public:
	virtual std::pair<const void*, uint64> GetMetadata() const override {
		return GetIndexEntryData(_static_metadata->user_metadata_entry);
	}
	virtual void SetMetadata(const void* metadata, uint64 metadata_size) override {
		if (IsMetadataSet()) { throw std::invalid_argument("set metadata twice"); }
		_static_metadata->user_metadata_entry = AllocateArray(metadata, metadata_size);
		_static_metadata->index_table_entry = AllocateArray(_index_entry_table.data(), _index_entry_table.size() * index_entry_size);
	}


	//// indexed arrays management ////
private:
	vector<IndexEntry> _index_entry_table;
public:
	virtual uint64 CreateArray(const void* data, uint64 size) override {
		if (IsMetadataSet()) { throw std::invalid_argument("create array after setting metadata"); }
		uint64 array_index = _index_entry_table.size();
		_index_entry_table.push_back(AllocateArray(data, size));
		return array_index;
	}
	virtual std::pair<const void*, uint64> GetArrayData(uint64 array_index) const override {
		if (array_index >= _index_entry_table.size()) { throw std::invalid_argument("invalid array index"); }
		IndexEntry entry = _index_entry_table[array_index];
		return GetIndexEntryData(entry);
	}
};


END_NAMESPACE(StaticStore)