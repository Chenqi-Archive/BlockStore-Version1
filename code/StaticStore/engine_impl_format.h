#pragma once

#include "engine.h"


BEGIN_NAMESPACE(StaticStore)


struct IndexEntry {
	uint64 array_offset;
	uint64 array_size;
};


struct Metadata {
	uint64 file_size;
	IndexEntry user_metadata_entry;
	IndexEntry index_table_entry;
};


END_NAMESPACE(StaticStore)