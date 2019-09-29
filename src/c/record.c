#include "record.h"

record_t* record_create(void) {
	record_t* record = calloc(sizeof(record_t), 1);
	record->timestamp = time(NULL);
	record->latitude = NAN;
	record->longitude = NAN;

	return record;
}

size_t records_count(void) {
	return persist_read_int(STORAGE_RECORDS_COUNT_KEY);
}

void records_clear(void) {
	persist_write_int(STORAGE_RECORDS_COUNT_KEY, 0);
}

bool records_save(record_t* record) {
	size_t idx = records_count();
	// check if limit has been reached
	if(idx >= STORAGE_RECORDS_MAX) {
		return false;
	}

	size_t key = idx / STORAGE_RECORDS_PER_KEY;
	size_t offset = idx % STORAGE_RECORDS_PER_KEY;

	// merge record with chunk
	record_t records[STORAGE_RECORDS_PER_KEY];
	persist_read_data(key, &records, sizeof(records));
	memcpy(records + offset, record, sizeof(record_t));

	// save to storage
	persist_write_data(key, &records, sizeof(records));
	persist_write_int(STORAGE_RECORDS_COUNT_KEY, idx + 1);

	return true;
}
bool records_load(record_t* record, size_t idx) {
	// check if idx exists
	if(idx >= records_count()) {
		return false;
	}
	size_t key = idx / STORAGE_RECORDS_PER_KEY;
	size_t offset = idx % STORAGE_RECORDS_PER_KEY;

	// retrieve from storage
	record_t records[STORAGE_RECORDS_PER_KEY];
	persist_read_data(key, &records, sizeof(records));
	memcpy(record, records + offset, sizeof(record_t));

	return true;
}
