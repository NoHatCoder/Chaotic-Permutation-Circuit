#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "cpc.h"

#define keystretchround(i0,i1,i4,i7,offset) \
	i1 = cpc_xor(i1, i0); \
	i4 = cpc_twice(i4, i0, *(cpc_r*)(inblock + offset)); \
	i7 = cpc_twice(i7, *(cpc_r*)(inblock + 16 + offset), i0)

int keystretch(void* out, size_t outlength, const void* key, size_t keylength, const void* nonce, size_t noncelength, uint64_t serial) {
	if (outlength == 0 || keylength == 0) {
		return 1;
	}
	uint8_t inblock[256] = { 0 };
	size_t keyblocks = (keylength + 127) / 128;
	size_t nonceblocks = (noncelength + 63) / 64;
	size_t lastkeyblocksize = (keylength + 127) % 128 + 1;
	size_t lastnonceblocksize = (noncelength + 63) % 64 + 1;
	if (noncelength == 0) {
		nonceblocks = 1;
		lastnonceblocksize = 0;
		nonce = (void*)inblock;
	}
	size_t outblocks = (outlength + 15) / 16;
	size_t lastoutblocksize = (outlength + 15) % 16 + 1;
	cpc_r r0, r1, r2, r3, r4, r5, r6, r7;
	r0 = r1 = r2 = r3 = r4 = r5 = r6 = r7 = _mm_setr_epi32(0, 0, 0, 0);
	*(uint64_t*)(inblock + 192 + 0) = keylength;
	*(uint64_t*)(inblock + 192 + 16) = noncelength;
	*(uint64_t*)(inblock + 192 + 32) = serial;
	size_t counter;
	size_t initblocks = 4;
	if (keyblocks > 4) {
		initblocks = keyblocks;
	}
	size_t loops = outblocks + initblocks;
	for (counter = 0; counter < loops; counter++) {
		*(uint64_t*)(inblock + 192 + 48) = counter;
		size_t keyblock = counter % keyblocks;
		size_t nonceblock = counter % nonceblocks;
		if (keyblock + 1 == keyblocks) {
			memcpy((void*)(inblock + 0), (void*)((uint8_t*)key + keyblock * 128), lastkeyblocksize);
			memset((void*)(inblock + 0 + lastkeyblocksize), 0, 128 - lastkeyblocksize);
		}
		else {
			memcpy((void*)(inblock + 0), (void*)((uint8_t*)key + keyblock * 128), 128);
		}
		if (nonceblock + 1 == nonceblocks) {
			memcpy((void*)(inblock + 128), (void*)((uint8_t*)key + nonceblock * 64), lastnonceblocksize);
			memset((void*)(inblock + 128 + lastnonceblocksize), 0, 64 - lastnonceblocksize);
		}
		else {
			memcpy((void*)(inblock + 128), (void*)((uint8_t*)key + nonceblock * 64), 64);
		}
		keystretchround(r0, r1, r4, r7, 0);
		keystretchround(r1, r2, r5, r0, 32);
		keystretchround(r2, r3, r6, r1, 64);
		keystretchround(r3, r4, r7, r2, 96);
		keystretchround(r4, r5, r0, r3, 128);
		keystretchround(r5, r6, r1, r4, 160);
		keystretchround(r6, r7, r2, r5, 192);
		keystretchround(r7, r0, r3, r6, 224);
		if (counter >= initblocks) {
			if (counter + 1 == loops) {
				memcpy((void*)((uint8_t*)out + (counter - initblocks) * 16), (void*)(&r0), lastoutblocksize);
			}
			else {
				((cpc_r*)out)[counter - initblocks] = r0;
			}
		}
	}
	return 0;
}

void crypt1(void* out, const void* in, size_t length, uint8_t key[80], uint64_t streamstart) {
	if (length == 0) {
		return;
	}
	cpc_r counter = _mm_setr_epi32(streamstart / 16, streamstart >> 36, 0, 0);
	cpc_r one = _mm_setr_epi32(1, 0, 0, 0);
	size_t firstlength = 16 - streamstart % 16;
	size_t lastlength = ((length - firstlength + 31) % 16) + 1;
	cpc_r buffer;
	cpc_r stream;
	cpc_r* keytyped = (cpc_r*)key;
	if (length < firstlength + lastlength) {
		stream = cpc_twice(cpc_twice(cpc_xor(counter, keytyped[0]), keytyped[1], keytyped[2]), keytyped[3], keytyped[4]);
		memcpy((void*)((uint8_t*)(&buffer) + 16 - firstlength), in, firstlength + lastlength - 16);
		buffer = cpc_xor(buffer, stream);
		memcpy(out, (void*)((uint8_t*)(&buffer) + 16 - firstlength), firstlength + lastlength - 16);
	}
	else {
		size_t blockcount = (length - firstlength - lastlength) / 16 + 2;
		size_t i;
		const cpc_r* intyped;
		cpc_r* outtyped;
		for (i = 0; i < blockcount; i++) {
			stream = cpc_twice(cpc_twice(cpc_xor(counter, keytyped[0]), keytyped[1], keytyped[2]), keytyped[3], keytyped[4]);
			if (i == 0) {
				memcpy((void*)((uint8_t*)(&buffer) + 16 - firstlength), in, firstlength);
				buffer = cpc_xor(buffer, stream);
				memcpy(out, (void*)((uint8_t*)(&buffer) + 16 - firstlength), firstlength);
				intyped = (cpc_r*)((uint8_t*)in + firstlength);
				outtyped = (cpc_r*)((uint8_t*)out + firstlength);
			}
			else if (i == blockcount - 1) {
				memcpy((void*)(&buffer), (uint8_t*)in + length - lastlength, lastlength);
				buffer = cpc_xor(buffer, stream);
				memcpy((uint8_t*)out + length - lastlength, (void*)(&buffer), lastlength);
			}
			else {
				*outtyped = cpc_xor(*intyped, stream);
				intyped++;
				outtyped++;
			}
			counter = _mm_add_epi64(counter, one);
		}
	}
}

void print_r(cpc_r r) {
	int a;
	for (a = 0; a < 16; a++) {
		printf("%.2X ", ((uint8_t*)(&r))[a]);
	}
	printf("\n");
}

void print_s(uint8_t* s, int len) {
	int a;
	for (a = 0; a < len; a++) {
		if (s[a] < 32 || s[a]>126) {
			printf("\\x%.2X", s[a]);
		}
		else if (s[a] == '\\') {
			printf("\\\\");
		}
		else {
			printf("%c", s[a]);
		}
	}
}

int main() {
	int a;
	cpc_r keytest[5];
	keystretch(keytest, 80, "Secret key", 10, "Nonce", 5, 0);
	for (a = 0; a < 5; a++) {
		print_r(keytest[a]);
	}
	printf("\n");
	char* teststring = "3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679821480865132823066470938446095505822317253594081284811174502841027019385211055596446229489549303819644288109756659334461284756482337867831652712019091456485669234603486104543266482133936072602491412737245870066063155881748815209209628292540917153643678925903600113305305488204665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733624406566430860213949463952247371907021798609437027705392171762931767523846748184676694051320005681271452635608277857713427577896091736371787214684409012249534301465495853710507922796892589235420199561121290219608640344181598136297747713099605187072113499999983729780499510597317328160963185950244594553469083026425223082533446850352619311881710100031378387528865875332083814206171776691473035982534904287554687311595628638823537875937519577818577805321712268066130019278766111959092164201989";
	char testbuffer[1003];
	strcpy(testbuffer, teststring);
	print_s(testbuffer, 1002);
	printf("\n\n");
	crypt1(testbuffer, testbuffer, 1002, keytest, 0);
	print_s(testbuffer, 1002);
	printf("\n\n");
	crypt1(testbuffer, testbuffer, 99, keytest, 0);
	crypt1(testbuffer+99, testbuffer+99, 415, keytest, 99);
	crypt1(testbuffer+514, testbuffer+514, 3, keytest, 514);
	crypt1(testbuffer+517, testbuffer+517, 485, keytest, 517);
	print_s(testbuffer, 1002);
	(void)getchar();
}