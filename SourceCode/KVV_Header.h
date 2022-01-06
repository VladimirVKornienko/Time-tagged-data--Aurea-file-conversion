#pragma once

// 2 parts: inludes and function prototypes. //

#ifndef _CRT_SECURE_NO_WARNINGS		// for using "scanf", if not defined in pre-compiler options
#define _CRT_SECURE_NO_WARNINGS
#endif

//	*	*	*	*	*	*	*	*	//
//	*	1. 'include' part:		*	//
//	*	*	*	*	*	*	*	*	//

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <cstdint>		// for "uint32_t" type_def //
#include <vector>		// for std::vector<std::string> args(argv, argv + argc);

#include <iostream>
#include <fstream>
#include <sstream>

#include <iomanip>		// for outputting the float to required precision with 'cout' etc.
#include <bitset>		// for binary representations >> debug only, most likely.
// e.g.: std::cout << "a = " << std::bitset<8>(a)  << std::endl;
// Use example:
// cout << std::fixed << std::setprecision(2);
// revert:
// file1.unsetf(ios_base::fixed);
// file1 << std::setprecision(16) << thePoint[2];

#include <math.h>       // lround
#include <algorithm>	// std::max

#ifndef NOMINMAX		// for <windows.h>: otherwise, std::max will be unavailable... //
	#define NOMINMAX
#endif
#include <windows.h>	// for Sleep(milliseconds); //


//	*	*	*	*	*	*	*	*	*	*	*	//
//	*	2. function declaration part:		*	//
//	*	*	*	*	*	*	*	*	*	*	*	//

int PreProcessAureaDataStage1(const char* fileInName, const char* fileOutNameCh1, const char* fileOutNameCh2,
	const char* fileOutNameHEADER, double* outAureaPSin1Tag,
	double* outMeasTime, uint64_t* NcntsCh1, uint64_t* NcntsCh2,
	uint64_t* OUTch1Nskipped, uint64_t* OUTch2Nskipped);



int PreProcessAureaDataStage2(const char* fileOutName, const char* fileInNameCh1, const char* fileInNameCh2,
	const char* fileInNameHEADER, double inAureaPSin1Tag,
	double inMeasTime, uint64_t Ncnts1, uint64_t Ncnts2,
	uint64_t* OUTnOVFLmarkersESTIMATE, uint64_t* OUTnOVFLmarkersREAL);


int PreProcessAureaDataStage3splitter(double TimeToSplitSEC, const char* fileOutName, const char* fileInNameCh1, const char* fileInNameCh2,
	const char* fileInNameHEADER, double inAureaPSin1Tag,
	double inMeasTime, uint64_t Ncnts1, uint64_t Ncnts2,
	uint64_t* OUTnOVFLmarkersESTIMATE, uint64_t* OUTnOVFLmarkersREAL);


int ConvertPTUtoAUREA(const char* fileInNamePTU, const char* fileOutCh1, const char* fileOutCh2

	// 20.12.: header processing will be added later.
	// , const char* fileOutNameCh2
	// , const char* fileOutNameHEADER

);


#ifndef CONSTANTS_H
#define CONSTANTS_H

extern const uint32_t myOverflowVal;

#endif