#ifndef _NVS_H_
#define _NVS_H_

#define IP_FLAG     "ipflag"
#define SWITCH_FLAG "swflag"
#define KEY_OPEN    "op"
#define NOMRAL      "nomral_c"

int8_t NVS_READ_INT8(const char* key);
void NVS_WRITE_INT8(const char* key,int8_t value);
int8_t NVS_READ_BLOB(const char* key);
void NVS_WRITE_BLOB(const char* key,const void* value,int len);

time_t NVS_READ_INT64(const char* key);
void NVS_WRITE_INT64(const char* key,int64_t value);

#endif