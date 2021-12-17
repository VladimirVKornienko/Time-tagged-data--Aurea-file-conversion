#pragma once
// int PreProcessAureaData(const char* fileInName, const char* fileOutName);

//int PreProcessAureaDataStage1(const char* fileInName, const char* fileOutNameCh1, const char* fileOutNameCh2,
//	const char* fileOutNameHEADER, double* outAureaPSin1Tag);

#define NOMINMAX
// << otherwise, std::max will be unavailable...
#include <windows.h>
// for Sleep(milliseconds); //

// VKORN_TUESDAY >>

//int PreProcessAureaDataStage1(const char* fileInName, const char* fileOutNameCh1, const char* fileOutNameCh2,
//	const char* fileOutNameHEADER, double* outAureaPSin1Tag,
//	uint64_t* outMeasTime, uint64_t* NcntsCh1, uint64_t* NcntsCh2,
//	uint64_t* OUTch1Nskipped, uint64_t* OUTch2Nskipped);

int PreProcessAureaDataStage1(const char* fileInName, const char* fileOutNameCh1, const char* fileOutNameCh2,
	const char* fileOutNameHEADER, double* outAureaPSin1Tag,
	double* outMeasTime, uint64_t* NcntsCh1, uint64_t* NcntsCh2,
	uint64_t* OUTch1Nskipped, uint64_t* OUTch2Nskipped);
// << VKORN_TUESDAY


//int PreProcessAureaDataStage2(const char* fileOutName, const char* fileInNameCh1, const char* fileInNameCh2,
//	const char* fileInNameHEADER, double inAureaPSin1Tag);

//int PreProcessAureaDataStage2(const char* fileOutName, const char* fileInNameCh1, const char* fileInNameCh2,
//	const char* fileInNameHEADER, double inAureaPSin1Tag,
//	uint64_t outMeasTime, uint64_t NcntsTOT);

// new! gives 2 more arguments: estimated and real number of markers.
// currently, EST is added to the file header...


// VKORN_TUESDAY >>
//int PreProcessAureaDataStage2(const char* fileOutName, const char* fileInNameCh1, const char* fileInNameCh2,
//	const char* fileInNameHEADER, double inAureaPSin1Tag,
//	uint64_t outMeasTime, uint64_t NcntsTOT,
//	uint64_t* OUTnOVFLmarkersESTIMATE, uint64_t* OUTnOVFLmarkersREAL);

int PreProcessAureaDataStage2(const char* fileOutName, const char* fileInNameCh1, const char* fileInNameCh2,
	const char* fileInNameHEADER, double inAureaPSin1Tag,
	double inMeasTime, uint64_t Ncnts1, uint64_t Ncnts2,
	uint64_t* OUTnOVFLmarkersESTIMATE, uint64_t* OUTnOVFLmarkersREAL);


int PreProcessAureaDataStage3splitter(double TimeToSplitSEC, const char* fileOutName, const char* fileInNameCh1, const char* fileInNameCh2,
	const char* fileInNameHEADER, double inAureaPSin1Tag,
	double inMeasTime, uint64_t Ncnts1, uint64_t Ncnts2,
	uint64_t* OUTnOVFLmarkersESTIMATE, uint64_t* OUTnOVFLmarkersREAL);
// << VKORN_TUESDAY


#ifndef CONSTANTS_H
#define CONSTANTS_H

extern const uint32_t myOverflowVal;

#endif