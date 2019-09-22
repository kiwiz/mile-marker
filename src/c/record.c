#include "record.h"

size_t records_count(void) {
	return persist_read_int(STORAGE_RECORDS_COUNT_KEY);
}

size_t records_save(record_t* record) {
	size_t idx = records_count();
	size_t key = idx / STORAGE_RECORDS_PER_KEY;
	size_t offset = idx % STORAGE_RECORDS_PER_KEY;

	record_t records[STORAGE_RECORDS_PER_KEY];
	persist_read_data(key, &records, sizeof(records));
	memcpy(records + offset, record, sizeof(record_t));
	persist_write_data(key, &records, sizeof(records));

	persist_write_int(STORAGE_RECORDS_COUNT_KEY, idx + 1);

	return 1;
}
size_t records_load(record_t* record, size_t idx) {
	size_t key = idx / STORAGE_RECORDS_PER_KEY;
	size_t offset = idx % STORAGE_RECORDS_PER_KEY;

	record_t records[STORAGE_RECORDS_PER_KEY];
	persist_read_data(key, &records, sizeof(records));
	memcpy(record, records + offset, sizeof(record_t));

	return 1;
}
