#pragma once
#include "_config.h"
#include <cstdint>

#ifdef __GNUC__
#define LEHMER_MAX_VAL 0xFFFFFFFFFFFFFFFF
#define LEHMER_M 0xda942042e4dd58b5
#define LEHMER_MOD 64 
typedef __uint128_t lehmerState;
typedef uint64_t next;

#else
#define LEHMER_MAX_VAL 0x7FFFFFFF
#define LEHMER_M 0x41A7
#define LEHMER_MOD 32
typedef uint64_t lehmerState;
typedef uint32_t next;
#endif


class CustomRand
{
public:
	CustomRand(uint32_t seed = 69);

	ft NextFloat(ft min, ft max);
	ui NextU32(ui min, ui max);
	ui NextUi();

protected:
private:
	lehmerState g_lehmer64_state;
	next Next();
	next procGen;
};
