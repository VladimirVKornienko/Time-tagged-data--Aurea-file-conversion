/*
< moved to the header file. >

#include <cstdint>
// uint32_t

*/

#include "KVV_Header.h"

// const uint32_t myOverflowVal = (((uint32_t)1) << 25);
// 25 bits for a time tag; this constant defines max time displacement, after which an overflow occurs.

//const uint32_t myOverflowVal = (((uint32_t)1) << 28);
// Updated for HH T2 format: 4 bits channel (0 == overflow, 1 = ch1, 2 = ch2, etc.)

// 18.11.2021: changed to match PicoHarp Parser...
const uint32_t myOverflowVal = 210698240 ;

// 06.01.2022:
#define AUREABytesInATagName	32	// for formatting the header file.
#define AUREAGlobalResolution	1.0e-12 // in seconds; universal, everywhere here...