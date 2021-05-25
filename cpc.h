#ifndef CHAOTIC_PERMUTATION_CIRCUIT_GUARD
#define CHAOTIC_PERMUTATION_CIRCUIT_GUARD
#if _MSC_VER
#include <intrin.h>
#else
#include <x86intrin.h>
#endif
#define cpc_r __m128i
#define cpc_64 unsigned long long
#define cpc_xor(a,b) _mm_xor_si128((a),(b))
#define cpc_or(a,b) _mm_or_si128((a),(b))
#define cpc_shl(a,b) _mm_slli_epi64((a),(b))
#define cpc_shr(a,b) _mm_srli_epi64((a),(b))
#define cpc_shuf(a,b) _mm_shuffle_epi8((a),(b))
#define cpc_interleave _mm_setr_epi8(13,1,15,3,12,0,14,2,5,9,7,11,4,8,6,10)
//#define cpc_interleave _mm_setr_epi8(8,0,9,1,10,2,11,3,12,4,13,5,14,6,15,7)
//#define cpc_interleave _mm_setr_epi8(0,8,1,9,2,10,3,11,4,12,5,13,6,14,7,15)
#define cpc_pi1 _mm_set_epi8(0x32, 0x43, 0xF6, 0xA8, 0x88, 0x5A, 0x30, 0x8D, 0x32, 0x43, 0xF6, 0xA8, 0x88, 0x5A, 0x30, 0x8D)
#define cpc_pi2 _mm_set_epi8(0x32, 0x43, 0xF6, 0xA8, 0x88, 0x5A, 0x30, 0x8D, 0x31, 0x31, 0x98, 0xA2, 0xE0, 0x37, 0x07, 0x34)
/*
#define cpc_pi1 _mm_set_epi8(0x32, 0x43, 0xF6, 0xA8, 0x88, 0x5A, 0x30, 0x8D, 0x32, 0x43, 0xF6, 0xA8, 0x88, 0x5A, 0x30, 0x8D)
#define cpc_pi2 _mm_set_epi8(0x31, 0x31, 0x98, 0xA2, 0xE0, 0x37, 0x07, 0x34, 0x31, 0x31, 0x98, 0xA2, 0xE0, 0x37, 0x07, 0x34)
*/

#define cpc_step_left4(r) r=cpc_xor(cpc_xor(r,cpc_shl(r,4)),cpc_or(cpc_shl(r,1),cpc_shl(r,2)))
#define cpc_step_right10(r) r=cpc_xor(cpc_xor(r,cpc_shr(r,10)),cpc_or(cpc_shr(r,1),cpc_shr(r,2)))
#define cpc_step_left7(r) r=cpc_xor(cpc_xor(r,cpc_shl(r,7)),cpc_or(cpc_shl(r,1),cpc_shl(r,2)))

#define cpc_block(r) \
	r = cpc_shuf(r, cpc_interleave); \
	cpc_step_left4(r); \
	cpc_step_right10(r); \
	cpc_step_left7(r);

#define cpc_4blocks(r) \
	r=cpc_xor(r,cpc_pi1); \
	cpc_block(r); \
	r=cpc_xor(r,cpc_pi1); \
	cpc_block(r); \
	r=cpc_xor(r,cpc_pi2); \
	cpc_block(r); \
	r=cpc_xor(r,cpc_pi1); \
	cpc_block(r)

cpc_r cpc(cpc_r a, cpc_r b) {
	cpc_4blocks(a);
	return cpc_xor(a, b);
}

cpc_r cpc_scalar(cpc_r a, cpc_r b) {
	cpc_64 a0 = ((cpc_64*)&a)[0];
	cpc_64 a1 = ((cpc_64*)&a)[1];
	cpc_64 b0 = ((cpc_64*)&b)[0];
	cpc_64 b1 = ((cpc_64*)&b)[1];
	cpc_r pi = cpc_pi2;
	cpc_64 pi0 = ((cpc_64*)&pi)[0];
	cpc_64 pi1 = ((cpc_64*)&pi)[1];
	int i;
	for (i = 0; i < 4; i++) {
		if (i == 2) {
			a0 ^= pi0;
		}
		else {
			a0 ^= pi1;
		}
		a1 ^= pi1;
		cpc_64 ta0 = (a0 & 0x00000000ff00ff00LL) | ((a0 & 0x0000000000ff00ffLL) << 40) | (a1 & 0x00ff00ff00000000LL) | ((a1 & 0xff00ff0000000000LL) >> 40);
		cpc_64 ta1 = (a1 & 0x00000000ff00ff00LL) | ((a1 & 0x0000000000ff00ffLL) << 40) | (a0 & 0x00ff00ff00000000LL) | ((a0 & 0xff00ff0000000000LL) >> 40);
		a0 = ta0;
		a1 = ta1;
		a0 = (a0 ^ (a0 << 4)) ^ ((a0 << 1) | (a0 << 2));
		a1 = (a1 ^ (a1 << 4)) ^ ((a1 << 1) | (a1 << 2));
		a0 = (a0 ^ (a0 >> 10)) ^ ((a0 >> 1) | (a0 >> 2));
		a1 = (a1 ^ (a1 >> 10)) ^ ((a1 >> 1) | (a1 >> 2));
		a0 = (a0 ^ (a0 << 7)) ^ ((a0 << 1) | (a0 << 2));
		a1 = (a1 ^ (a1 << 7)) ^ ((a1 << 1) | (a1 << 2));
	}
	a0 ^= b0;
	a1 ^= b1;
	((cpc_64*)&a)[0] = a0;
	((cpc_64*)&a)[1] = a1;
	return a;
}
/*
cpc_r cpc_twice(cpc_r a, cpc_r b, cpc_r c) {
	return cpc(cpc(a, b), c);
}
*/
cpc_r cpc_twice(cpc_r a, cpc_r b, cpc_r c) {
	return cpc_scalar(cpc_scalar(a, b), c);
}

#endif