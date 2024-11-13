#ifndef UTILS_H
#define UTILS_H

// 64-bit unsigned integer handling
#define HI(x) ((uint32_t)((x) >> 32))
#define LO(x) ((uint32_t)(x))

#define TO64(hi, lo) ((((uint64_t)(hi)) << 32) | (lo))

#endif