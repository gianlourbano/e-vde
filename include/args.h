#ifndef CONFIG_H
#define CONFIG_H

/**
 * Option layout: same as struct option, but var field is
 * 
 * | 10 bit     | 6 bit  | 16 bit           |
 * | ---------- | ------ | ---------------- |
 * | json opts  | mod_id | actual val       |
 * 
 * instead of just an int. This way i can use the same option
 * for both json and command line options ( i have 2^16 possible values, should be enough)
 */

// 0000 0000 10 | 00 0000 | 0000 0000 0110 0011 json string value, with 'c' as the short option

#define no_argument 0
#define required_argument 1
#define optional_argument 2

#define JSON_INT 1
#define JSON_STRING 2
#define JSON_BOOL 4
#define JSON_ARRAY(x) (8 | (x))
#define JSON_OBJECT(x) (16 | (x))

#define GET_VAL(x) ((x) & 0xFFFF)
#define GET_MOD(x) ((x >> 16) & 0x3F)
#define GET_JSON(x) ((x >> 22) & 0x3FF)

#define SET_JSON(x) ((x) << 22)
#define SET_MOD(x) ((x) << 16)
#define SET_VAL(x) ((x) & 0xFFFF)

int process_options(int argc, char **argv);
int process_json_config(const char* pathname);

int parse_global_opt(int c, void* optarg);

#endif