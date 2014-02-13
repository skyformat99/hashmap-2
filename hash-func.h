#ifndef __HASH_FUNC_H__
#define __HASH_FUNC_H__

unsigned long simple_hash11(const char* begin, unsigned len);
unsigned long simple_hash31(const char* begin, unsigned len);
unsigned long djb_hash(const char* begin, unsigned len);
unsigned long sdbm_hash(const char* begin, unsigned len);
unsigned long dek_hash(const char* begin, unsigned len);
unsigned long bp_hash(const char* begin, unsigned len);
unsigned long pjw_hash(const char* begin, unsigned len);

#endif
