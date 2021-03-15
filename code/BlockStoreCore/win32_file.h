#pragma once

#include "core.h"
#include "uncopyable.h"


BEGIN_NAMESPACE(BlockStoreCore)


class Win32File : Uncopyable {
public:
	enum class CreateMode {	        //	| already existing	|  not existing	|  
		CreateAlways = 2,			//	| 		clear		|	  create	|
		OpenAlways = 4,				//	| 		open		|	  create	|
		CreateNew = 1,				//	| 	    error		|	  create	|
		OpenExisting = 3,			//	| 		open		|	  error		|
		TruncateExisting = 5		//	| 		clear		|	  error		|
	};
	enum class AccessMode {
		ReadOnly = 0x80000000L,		// GENERIC_READ
		ReadWrite = 0xC0000000L,	// GENERIC_READ | GENERIC_WRITE
	};
	enum class ShareMode {
		None = 0x00000000,			// NULL
		ReadOnly = 0x00000001,		// FILE_SHARE_READ
		ReadWrite = 0x00000003,		// FILE_SHARE_READ | FILE_SHARE_WRITE
	};

public:
	Win32File(const wchar file[],
			  CreateMode create_mode = CreateMode::OpenAlways,
			  AccessMode access_mode = AccessMode::ReadWrite,
			  ShareMode share_mode = ShareMode::None);
	~Win32File();

private:
	using HANDLE = void*;
	HANDLE _file;
	uint64 _size;
	CreateMode _create_mode;
	AccessMode _access_mode;
	ShareMode _share_mode;
public:
	uint64 GetSize() const { return _size; }
	void SetSize(uint64 size);


	//// file map view cache management ////

	/* not implemented yet, just try mapping the entire file. */

private:
	HANDLE _file_map;
	void* _map_view_address;
public:
	void DoMapping();
	void UndoMapping();
	bool IsMapped() const { return _map_view_address != nullptr; }
	void* GetMapViewAddress() const { return _map_view_address; }
};


END_NAMESPACE(BlockStoreCore)