#pragma once

#define LOG_2(n) ((n > 1) ? 1 : 0)
#define LOG_4(n) ((n > 2) ? 2 : LOG_2(n))
#define LOG_8(n) ((n > 4) ? 3 : LOG_4(n))
#define LOG_16(n) ((n > 8) ? 4 : LOG_8(n))
#define LOG_32(n) ((n > 16) ? 5 : LOG_16(n))
#define LOG_64(n) ((n > 32) ? 6 : LOG_32(n))
#define LOG_128(n) ((n > 64) ? 7 : LOG_64(n))
#define LOG_256(n) ((n > 128) ? 8 : LOG_128(n))
#define LOG_512(n) ((n > 256) ? 9 : LOG_256(n))
#define LOG_1024(n) ((n > 512) ? 10 : LOG_512(n))
#define LOG_2048(n) ((n > 1024) ? 11 : LOG_1024(n))
#define LOG_4096(n) ((n > 2048) ? 12 : LOG_2048(n))
#define LOG_8192(n) ( (n > 4096) ? 13 : LOG_4096(n))
#define LOG_16384(n) ((n > 8192) ? 14 : LOG_8192(n))
#define LOG_32768(n) ((n > 16384) ? 15 : LOG_16384(n))

// n < 65536
#define BITS_TO_REPRESENT(n) ((n > 65536) ? 16 : LOG_32768(n))

#define BITS_PER_ACCOUNT BITS_TO_REPRESENT(ACCOUNTS)
#define BITS_PER_TOKEN BITS_TO_REPRESENT(TOKENS)
#define BITS_PER_DECIMAL 100
#define BITS_PER_ORDER (BITS_PER_ACCOUNT + (2 * BITS_PER_TOKEN) + (2 * BITS_PER_DECIMAL))