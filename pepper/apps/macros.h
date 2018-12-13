#pragma once

#define LOG_LESS_THAN_2(n) ((n >= 1) ? 1 : 0)
#define LOG_LESS_THAN_4(n) (LOG_LESS_THAN_2(n) + (n >= 2) ? 1 : 0)
#define LOG_LESS_THAN_8(n) (LOG_LESS_THAN_4(n) + (n >= 4) ? 1 : 0)
#define LOG_LESS_THAN_16(n) (LOG_LESS_THAN_8(n) + (n >= 8) ? 1 : 0)
#define LOG_LESS_THAN_32(n) (LOG_LESS_THAN_16(n) + (n >= 16) ? 1 : 0)
#define LOG_LESS_THAN_64(n) (LOG_LESS_THAN_32(n) + (n >= 32) ? 1 : 0)
#define LOG_LESS_THAN_128(n) (LOG_LESS_THAN_64(n) + (n >= 64) ? 1 : 0)
#define LOG_LESS_THAN_256(n) (LOG_LESS_THAN_128(n) + (n >= 128) ? 1 : 0)
#define LOG_LESS_THAN_512(n) (LOG_LESS_THAN_256(n) + (n >= 256) ? 1 : 0)
#define LOG_LESS_THAN_1024(n) (LOG_LESS_THAN_512(n) + (n >= 512) ? 1 : 0)
#define LOG_LESS_THAN_2048(n) (LOG_LESS_THAN_1024(n) + (n >= 1024) ? 1 : 0)
#define LOG_LESS_THAN_4096(n) (LOG_LESS_THAN_2048(n) + (n >= 2048) ? 1 : 0)
#define LOG_LESS_THAN_8192(n) (LOG_LESS_THAN_4096(n) + (n >= 4096) ? 1 : 0)
#define LOG_LESS_THAN_16384(n) (LOG_LESS_THAN_8192(n) + (n >= 8192) ? 1 : 0)
#define LOG_LESS_THAN_32768(n) (LOG_LESS_THAN_16384(n) + (n >= 16384) ? 1 : 0)

// n < 65536
#define BITS_TO_REPRESENT(n) (LOG_LESS_THAN_32768(n) + (n >= 65536) ? 1 : 0)