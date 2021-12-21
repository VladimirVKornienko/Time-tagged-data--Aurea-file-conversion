/*
*		Aurea measurement files' pre-processing tool.
*
*   From "header,data" create a "new_header,HH_format_data" file and store.
*
*/


//	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	//
//		Stage1 (+) splits the file in three + extracts PSin1Tag, MeasTime, Nch1, Nch2 from the file.		//
//		Stage2 (+) gets some of those parameters as inputs, and makes a *.PTU file in reqired format.		//
//		Stage3 (not ready / unstable) is same to Stage2, but splits the initial file in chunks of			//
//																		<TimeToSplitSEC> time chunks each.	//
//	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	//





// VKORN_TUESDAY >>
// <<< Label for the recent changes in file I/O: from "write" to "<<".


#define AUREABytesInATagName	32	// for formatting the header file.
#define AUREAGlobalResolution	1.0e-12 // in seconds; universal, everywhere here...

// Nov.20: for "Stage2", added rounding to the nearest ps value: value = round( value * 1000.0 ) / 1000.0;

// [<MeasurementTime>] == milliseconds ??!? //

#ifndef AureaProcessorPart2OverflowDebuggingMessages
//#define AureaProcessorPart2OverflowDebuggingMessages
// << uncomment to use debugging on first 10 records. //
#endif

#ifndef AureaReverseChannelAndTimeData
//#define AureaReverseChannelAndTimeData

// << uncomment to write time tag first, and only then the channel data. //
#endif


#ifndef AureaNoHeaderToResultFile
// #define AureaNoHeaderToResultFile
// << uncomment for DEBUG MODE (ONLY). //
#endif

#ifndef stage4_Write_Overflow_Flags_ToFile
#define stage4_Write_Overflow_Flags_ToFile
// (un)comment for writing (skipping) overflow markers when de-coding back from PTU format to Aurea format.
// will be used in [stage3] also, if
//				<stage3_DEBUG_re_create_files_with_text_tag_time_values> is defined.
#endif

#ifndef stage3_DEBUG_re_create_files_with_text_tag_time_values
#define stage3_DEBUG_re_create_files_with_text_tag_time_values

// uncomment to create 2 add. files with tags and times, as seen in stage 3.
// (Not from PTU file, as in Stage 4).

// this checks for the consistency of aux. files, generated suring the stage 1.
// it has been proof-checked, it is working finely.

// NB! works only for the last file in a batch (matter of simplicity, and enough for debugging).
#endif

#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include <iostream>

#include <fstream>

#include <sstream>
#include <string>

#include "KVV_Header.h"

#include <bitset>		// for binary representations >> debug only, most likely.

#include <iomanip>		// for outputting the floast to required precision with 'cout' etc.

#include <math.h>       /* lround */

#include <algorithm>		// std::max

// #include <windows.h> 
// for Sleep(milliseconds); //

using namespace std;

// #include <filesystem>
// namespace fs = std::filesystem;	// DOES NOT WORK :: TO YOUNG...
// << for copying files...
// << DOES NOT WORK :: TOO YOUNG AN UPDATE, <FILESYSTEM>




//int PreProcessAureaDataStage1(const char* fileInName, const char* fileOutNameCh1, const char* fileOutNameCh2,
//	const char* fileOutNameHEADER, double* outAureaPSin1Tag,
//	uint64_t* outMeasTime, uint64_t* outNCh1, uint64_t* outNCh2, uint64_t* OUTch1Nskipped, uint64_t* OUTch2Nskipped)
int PreProcessAureaDataStage1(const char* fileInName, const char* fileOutNameCh1, const char* fileOutNameCh2,
	const char* fileOutNameHEADER, double* outAureaPSin1Tag,
	double* outMeasTime, uint64_t* outNCh1, uint64_t* outNCh2, uint64_t* OUTch1Nskipped, uint64_t* OUTch2Nskipped)

	// outMeasTime : Total measurement time in milliseconds, to be returned.

{
	//	*	*	*	*	*	*	*	*	//
	//	*		  PROCESSOR			*	//
	//	*	*	*	*	*	*	*	*	//
	
	//		Split to 3 files (header, ch1, ch2) and convert <time(ns, float)> to <time(ps, uint64)t>.		//
	//		Also, skip bad entries (with either '0' or very large values
	//				(5.58344e+7 and 5.58345e+7 for 625 kHz freq (divider 1 = none)) ). //
	
	//   >>>   DEFINE THESE !!  >>>   //
	const char myFreqLine[] = "%   Sync freq:";	// header prefix before the Sync. freq.
	double myUpperDiscrLevelFraction = 1.00;	// a fraction of the (1/freq), will be calculated later
												// all delays above that value will be treated as erroneous records.

	double myLowerDiscrLevel = 1.0e-6;			// entered manually
	//   <<<   DEFINE THESE !!  <<<   //

	uint64_t ch1Nskipped = 0;
	uint64_t ch2Nskipped = 0;		// bad records not passing the discriminator
									// see also <myUpperDiscrLevel> and <myLowerDiscrLevel>

	uint64_t NtotCh1 = 0;
	uint64_t NtotCh2 = 0;
	
	double myUpperDiscrLevel = 0; // ns (!) // Will mainly be used for checking the wrong values of Times... //
	double myAureaFreq = 0; // to be more flexible... though unsigned long should be enough.

	// >>>>>>>>>>
	// The line below can be used (it is believed to speed up the execution), 
	//    but from my experience it complicates the output and mixed it up.
	// Can probably be used with file output streams.
	
	// std::ios_base::sync_with_stdio(false);
	
	// closing statement is below, in the same function.
	// <<<<<<<<<<

	// KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK
	// Getting total experiment time: >>
	
	// LABEL EVENING TYPES >>>
	//uint64_t	maxTAG = 0;
	int64_t	maxTAG = 0;

	// Will be +1 after last entry. (An approximation, but quite accurate one!).
	// <<<


	// open files >> //
	// VKORN DESPERATE FLAG >>
	/*
	std::ifstream fileInHandle(fileInName);
	if (!fileInHandle.is_open()) { return 1; }

	//std::ofstream fileOut1(fileOutNameCh1);
	std::ofstream fileOut1(fileOutNameCh1, std::ios::out | std::ios::binary);
	if (!fileOut1.is_open()) {		fileInHandle.close(); return 1;		}
	
	//std::ofstream fileOut2(fileOutNameCh2);
	std::ofstream fileOut2(fileOutNameCh2, std::ios::out | std::ios::binary);
	if (!fileOut1.is_open()) { fileInHandle.close(); fileOut1.close(); return 1; }

	std::ofstream fileOut3header(fileOutNameHEADER);
	if (!fileOut3header.is_open())
	{
		fileInHandle.close(); fileOut1.close(); fileOut2.close(); return 1;
	}
	*/

	// FILE* fileInHandle;
	std::ifstream fileInHandle(fileInName);
	if (!fileInHandle.is_open()) { return 1; }

	FILE* fileOut1;
	FILE* fileOut2;
	// FILE* fileOut3header;

	fileOut1 = fopen(fileOutNameCh1, "wb");
	if (fileOut1 == NULL) {
		fileInHandle.close();
		cout << "Error opening file for Ch.1" << std::endl;
		return 1;
	}

	fileOut2 = fopen(fileOutNameCh2, "wb");
	if (fileOut2 == NULL) {
		cout << "Error opening file for Ch.2" << std::endl;
		fileInHandle.close();
		fclose(fileOut1);
		return 1;
	}

	fileOut2 = fopen(fileOutNameCh2, "wb");
	if (fileOut2 == NULL) {
		cout << "Error opening file for Ch.2" << std::endl;
		fileInHandle.close();
		fclose(fileOut1);
		return 1;
	}

	std::ofstream fileOut3header(fileOutNameHEADER);
	if (!fileOut3header.is_open())
	{
		cout << "Error opening file for Header." << std::endl;
		fileInHandle.close(); fclose(fileOut1); fclose(fileOut2); return 1;
	}

	// << VKORN DESPERATE FLAG
	
	// cout << std::endl << "Splitting the file in three (header, ch1, ch2):" << std::endl << std::endl;
	// Readout has 3 parts: read header, read 2 channel data ("   ch1tag   ch1time   ch2tag   ch2time"),
	// then read 1 channel data ("   ch?tag   ch?time") and determine channel number.
	// delimeter is three spaces.

	bool runFlag;
	std::string line = "%";		// currently, reading from file is implemented via a file stream. not the fastest.
	std::istringstream issTMP;	// main string_stream for processing the file line-by-line
	// should be initialized with "line" above: "" issTMP.str(line); "" //

	// part 1: read the header: each line of the header starts with a '%'symbol.
	
	std::size_t StrSearchPos;		// for .find() method in std::string.

	runFlag = true;
	{
		bool FirstRun = true;

		while ((runFlag) && (std::getline(fileInHandle, line))) {
			if (line.at(0) == '%')
			{
				// DEBUG print: >>>
				// printf("%s\n", line.c_str());
				// <<<
							   
				if (FirstRun)	// So as not to introduce additional empty lines... //
				{
					FirstRun = false;
					fileOut3header << line.c_str();
				}
				else
				{
					fileOut3header << std::endl << line.c_str();
				}

				StrSearchPos = line.find(myFreqLine);
				if (StrSearchPos != string::npos)
				{	// this record will be used to find the upper discrimination level.
					
					line = line.substr(StrSearchPos + strlen(myFreqLine));
					
					issTMP.str(line);
					if (issTMP >> myAureaFreq)
					{
						// cout << "success!" << std::endl;
					}
					else
					{
						myAureaFreq = 1.0; // Hz
						// cout << "error getting the Freq value!!!" << std::endl;
					}

					// for uint64_t and ps ::
					// myUpperDiscrLevel = (uint64_t)(llround((1.0 / myAureaFreq) * myUpperDiscrLevelFraction * 1e+12)); // ps
					
					// for double and ns:
					// myUpperDiscrLevel = 1e-3 * round((1.0 / myAureaFreq) * myUpperDiscrLevelFraction * 1e+12); // ns
					// with accuracy of 1 ps //
					myUpperDiscrLevel = (1.0 / myAureaFreq) * myUpperDiscrLevelFraction * 1e+9; // ns
					*outAureaPSin1Tag = (1.0 / myAureaFreq) * 1e+12; // ns

					// 1e+12 = conversion (1/Hz) -> (ps) //
										
					// cout << "found one!" << std::endl;
					// cout << "Start pos is" << StrSearchPos << std::endl;
					// cout << "Residual line is: >" << line.c_str() << std::endl;
					cout << "freq == " << myAureaFreq << std::endl;
					cout << "discr level (Max. Time, ns) == " << myUpperDiscrLevel << std::endl;
				}
			}
			else {
				runFlag = false;
			}

		}
	}
	fileOut3header.close();
	// <<< header file is completed.
		
	// part 2:  4 numbers total, 2 channels data is present
	
	// LABEL EVENING TYPES >>>
	//
	//uint64_t ch1tag = 0;
	//double ch1time = 0;
	//uint64_t ch2tag = 0;
	//double ch2time = 0;

	//// 20.12.2021: VKORN >>>
	//uint64_t PREVch1tag = 0;
	//double PREVch1time = 0;
	//uint64_t PREVch2tag = 0;
	//double PREVch2time = 0;
	//// <<<<

	int64_t ch1tag = 0;
	double ch1time = 0.0;
	int64_t ch2tag = 0;
	double ch2time = 0.0;

	// 20.12.2021: VKORN >>>
	int64_t PREVch1tag = 0;
	double PREVch1time = 0.0;
	int64_t PREVch2tag = 0;
	double PREVch2time = 0.0;
	// <<<<

	// LABEL EVENING TYPES <<<<
	// <<<<<<<<<<<<<<< 

	// first record - parse manually, from an std::string
	// using StringStream

	// std::istringstream issTMP(line);
	// Better way (if >1 assignment is needed): >>
	//std::istringstream issTMP;	// << definition moved to above //
	issTMP.str(line);
	// <<< ;

	// issTMP >> ch1tag >> ch1time >> ch2tag >> ch2time;
	/*
	// debug message:
	cout << std::endl << std::endl << "First record is parsed as:" << std::endl;
	cout << ch1tag << std::endl << ch1time << std::endl << ch2tag << std::endl << ch2time;
	*/
	// :: now the processing of this line is inside the 'while' loop, below...;
	

	// second and consequent record - until 2 numbers instead of 4: >> //
	// [ Well, now - first and all the rest...];
	// Writing to a binary file:
	// <file_handle>.write((<type>) &<elelment>, <size> (e.g. "sizeof(<element>)") );

	int i = 0;	// DEBUG ONLY

	runFlag = true;
	


	int numArgsRead = 0; // should be 4 at first, then switch to 2 at some moment;
	// it is reading from STRING, and NOT from a file!
	// that's why <int> here, not <std::size_t> //
	
	// LABEL EVENING TYPES >>>
	//uint64_t ch1LastTag = 0;	// used for determining the correct channel in 'part3'.	
	//uint64_t ch2LastTag = 0;
	int64_t ch1LastTag = 0;	// used for determining the correct channel in 'part3'.	
	int64_t ch2LastTag = 0;


	// VKORN_TUESDAY >>
	//unsigned int const cSizeOfTAG  = sizeof(uint64_t);
	//unsigned int const cSizeOfTIME = sizeof(double);

	std::size_t	cSizeOfTIME = sizeof(double);
	// LABEL EVENING TYPES >>>
	// std::size_t	cSizeOfTAG = sizeof(uint64_t);
	std::size_t	cSizeOfTAG = sizeof(int64_t);
	// <<< VKORN_TUESDAY

	// DEBUG >>>
	// cout << std::endl << std::endl << "Defined constants are:\n";
	//cout << cSizeOfTAG << std::endl << cSizeOfTIME << std::endl;
	// <<< DEBUG <<<

	// VKORN DESPERATE FLAG >>
	std::size_t unityForFWRITE = 1;
	// << VKORN DESPERATE FLAG

	// while ((i < 2) && (runFlag) && (std::getline(fileInHandle, line)))
	// DEBUG::
	// while ((i < 20) && (runFlag))
	while (runFlag)
		// new line is read from file at the very end of this 'while' loop.;
	{
		// 20.12.2021: VKORN >>   Check if the tags are monotonic.
		// at first step: zeroes.
		
		// UUUUUUUUU
		// moved to other parts of the instruction!

		// need to reject all below last [typically: many], 
		//						not only non-monotonic [typically: one point]!

		// PREVch1tag = ch1tag;
		// PREVch1time = ch1time;
		// PREVch2tag = ch2tag;
		// PREVch2time = ch2time;
		// <<<
		
		// unsigned long long int == "%llu", double in e-format == "%le". // (I hope!) //
		// numArgsRead = sscanf(line, "%llu %lf %llu %lf", &ch1tag, &ch1time, &ch2tag, &ch2time);
		
		// LABEL EVENING TYPES >>>
		// numArgsRead = sscanf(line.c_str(), "%llu %lf %llu %lf", &ch1tag, &ch1time, &ch2tag, &ch2time);		
		numArgsRead = sscanf(line.c_str(), "%lld %lf %lld %lf", &ch1tag, &ch1time, &ch2tag, &ch2time);
		
		
		// debug message:
		// cout << std::endl << "number of arguments read: " << numArgsRead << std::endl;
		// cout << std::endl << "record is parsed as:" << std::endl;
		// cout << ch1tag << std::endl << ch1time << std::endl << ch2tag << std::endl << ch2time << std::endl;
		// cout << "N_args: " << numArgsRead << std::endl;

		if (numArgsRead == 4)
		{	// normal processing: output to both channels:;
			// process the string
			
			// REMOVING BAD RECORDS: //
			if ((ch1time > myLowerDiscrLevel) && (ch1time < myUpperDiscrLevel) && (  (ch1tag > PREVch1tag) || ((ch1tag == PREVch1tag)&&(ch1time > PREVch1time))) )	// from 0.001 ps to UpperDiscrLevel //
			{
				// UUUUUUUUU
				// 20.12.2021: VKORN >>   Check if the tags are monotonic.
				// update the PREV value iff it is smaller than current entry:
				PREVch1tag = ch1tag;
				PREVch1time = ch1time;
				// <<<
				
				// VKORN_TUESDAY >>
				// OLD:
				// fileOut1.write((const char*)& ch1tag, cSizeOfTAG);
				// fileOut1.write((const char*)& ch1time, cSizeOfTIME);
				
				// Alas, text only...:
				// fileOut1 << ch1tag;
				// fileOut1 << ch1time;

				// NewTry:
				// VKORN DESPERATE FLAG >>
				//fileOut1.write(reinterpret_cast<char*>(&ch1tag), cSizeOfTAG);
				//fileOut1.write(reinterpret_cast<char*>(&ch1time), cSizeOfTIME);

				fwrite(&ch1tag, cSizeOfTAG, unityForFWRITE, fileOut1);
				fwrite(&ch1time, cSizeOfTIME, unityForFWRITE, fileOut1);

				// << VKORN DESPERATE FLAG
				// << VKORN_TUESDAY

				ch1LastTag = ch1tag;
				
				NtotCh1 = NtotCh1 + ((uint64_t)1);
			}
			else
			{
				ch1Nskipped = ch1Nskipped + ((uint64_t)1);
			}

			if ((ch2time > myLowerDiscrLevel) && (ch2time < myUpperDiscrLevel) && (  (ch2tag > PREVch2tag) || ((ch2tag == PREVch2tag)&&(ch2time > PREVch2time))) )
			{
				// UUUUUUUUU
				// 20.12.2021: VKORN >>   Check if the tags are monotonic.
				// update the PREV value iff it is smaller than current entry:
				PREVch2tag = ch2tag;
				PREVch2time = ch2time;
				// <<<
				
				// VKORN_TUESDAY >>
				// OLD:
				// fileOut2.write((const char*)& ch2tag, cSizeOfTAG);
				// fileOut2.write((const char*)& ch2time, cSizeOfTIME);
				
				// NEW: works for text mode only...
				// fileOut2 << ch2tag;
				// fileOut2 << ch2time;

				// NEW:
				// VKORN DESPERATE FLAG >>
				// fileOut2.write(reinterpret_cast<char*>(&ch2tag), cSizeOfTAG);
				// fileOut2.write(reinterpret_cast<char*>(&ch2time), cSizeOfTIME);
				// << VKORN DESPERATE FLAG
				fwrite(&ch2tag, cSizeOfTAG, unityForFWRITE, fileOut2);
				fwrite(&ch2time, cSizeOfTIME, unityForFWRITE, fileOut2);
				// << VKORN DESPERATE FLAG
				// << VKORN_TUESDAY

				ch2LastTag = ch2tag;
				
				NtotCh2 = NtotCh2 + ((uint64_t)1);
			}
			else
			{
				ch2Nskipped = ch2Nskipped + ((uint64_t)1);
			}

			// debug print;
			// cout << "wrote to file..." << std::endl;
			
			runFlag = runFlag && (std::getline(fileInHandle, line));
			// << read a new line //
		}
		else { // stop processing; determine the channel (1 or 2); then continue to part 3;
			
			// 20.12.2021: just proceed to processing 2 arguments, if needed.
			
			// if (numArgsRead == 2)
			// {
				runFlag = false;
			// }
			// else
			// {
			// 	cout << "unknown record encountered (stage 2), aborting..." << std::endl;
			// 	runFlag = false;
			// }
		}
	
	// i++; // overflow DEBUG control //
	}
	
	fflush(fileOut1);
	fflush(fileOut2);
	
	// part 3 :: 2 numbers total, only one channel left.
	runFlag = true;

	// DEBUG: >>
	// i = 0; // <<

	if (ch1LastTag > ch2LastTag)
	{	// ch2 lacks some of the ticks - thus, select channel 2 as output;
		// fileOut1.close();
		cout << "ch2 selected, file 1 WILL BE closed (LATER!)." << std::endl;
		
		// while ((runFlag) && (i < 5000)) // << DEBUG << //
		while (runFlag)
		{
			// YYYYYYYYYYYYYYYYYYYYY
			//// 20.12.2021: VKORN: for checking if monotonic:
			//PREVch2tag = ch2tag;
			//PREVch2time = ch2time;
			//// <<<

			numArgsRead = sscanf(line.c_str(), "%lld %lf", &ch2tag, &ch2time);
			if (numArgsRead == 2)
			{	// normal processing: output to both channels:;
				
				if ((ch2time > myLowerDiscrLevel) && (ch2time < myUpperDiscrLevel) && (  (ch2tag > PREVch2tag) || ((ch2tag == PREVch2tag)&&(ch2time > PREVch2time))) )	// from 0.001 ps to UpperDiscrLevel //
				{
					// 20.12.2021: VKORN: for checking if monotonic:
					PREVch2tag = ch2tag;
					PREVch2time = ch2time;
					// <<<
					
					// VKORN_TUESDAY >>
					// OLD: binary
					// fileOut2.write((const char*)& ch2tag, cSizeOfTAG);
					// fileOut2.write((const char*)& ch2time, cSizeOfTIME);
					
					// NEW: text
					// fileOut2 << ch2tag;
					//fileOut2 << ch2time;

					// NEW: binary
					// VKORN DESPERATE FLAG >>
					//fileOut2.write(reinterpret_cast<char*>(&ch2tag), cSizeOfTAG);
					//fileOut2.write(reinterpret_cast<char*>(&ch2time), cSizeOfTIME);
					
					fwrite(&ch2tag, cSizeOfTAG, unityForFWRITE, fileOut2);
					fwrite(&ch2time, cSizeOfTIME, unityForFWRITE, fileOut2);
					
					// << VKORN DESPERATE FLAG
					// << VKORN_TUESDAY
					
					ch2LastTag = ch2tag;

					NtotCh2 = NtotCh2 + ((uint64_t)1);
				}
				else
				{
					ch2Nskipped = ch2Nskipped + ((uint64_t)1);
				}

				runFlag = runFlag && (std::getline(fileInHandle, line));
				// << read a new line;
				
				// i++; // << DEBUG << //
			}
			else
			{
				cout << "unknown record encountered at stage 3, aborting..." << std::endl;
				runFlag = false;
			}
		}
	}
	else
	{	// else, select channel 1 as output;
		// fileOut2.close();
		cout << "ch1 selected, file 2 WILL BE closed (LATER!)." << std::endl;
		
		// while ((runFlag) && (i < 5000)) // << DEBUG << //
		while (runFlag)
		{
			// YYYYYYYYYYYYYYYYYYYYY
			//// 20.12.2021: VKORN: for checking if monotonic:
			//PREVch1tag = ch1tag;
			//PREVch1time = ch1time;
			//// <<<

			numArgsRead = sscanf(line.c_str(), "%lld %lf", &ch1tag, &ch1time);
			if (numArgsRead == 2)
			{	// normal processing: output to both channels:;
				if ((ch1time > myLowerDiscrLevel) && (ch1time < myUpperDiscrLevel) && (  (ch1tag > PREVch1tag) || ((ch1tag == PREVch1tag)&&(ch1time > PREVch1time))) )	// from 0.001 ps to UpperDiscrLevel //
				{
					// 20.12.2021: VKORN: for checking if monotonic:
					PREVch1tag = ch1tag;
					PREVch1time = ch1time;
					// <<<
					
					// VKORN_TUESDAY >>
					// OLD:
					// fileOut1.write((const char*)& ch1tag, cSizeOfTAG);
					// fileOut1.write((const char*)& ch1time, cSizeOfTIME);

					// NEW: text
					// fileOut1 << ch1tag;
					// fileOut1 << ch1time;

					// NEW: binary
					// VKORN DESPERATE FLAG >>
					// fileOut1.write(reinterpret_cast<char*>(&ch1tag), cSizeOfTAG);
					// fileOut1.write(reinterpret_cast<char*>(&ch1time), cSizeOfTIME);
					// << VKORN DESPERATE FLAG

					fwrite(&ch1tag, cSizeOfTAG, unityForFWRITE, fileOut1);
					fwrite(&ch1time, cSizeOfTIME, unityForFWRITE, fileOut1);

					// << VKORN_TUESDAY

					ch1LastTag = ch1tag;

					NtotCh1 = NtotCh1 + ((uint64_t)1);
				}
				else
				{
					ch1Nskipped = ch1Nskipped + ((uint64_t)1);
				}

				runFlag = runFlag && (std::getline(fileInHandle, line));
				// << read a new line;

				// i++; // << DEBUG << //
			}
			else
			{
				cout << "unknown record encountered at stage 3, aborting..." << std::endl;
				runFlag = false;
			}
		}
	}

	*outNCh1 = NtotCh1;
	*outNCh2 = NtotCh2;
	*OUTch1Nskipped = ch1Nskipped;
	*OUTch2Nskipped = ch2Nskipped;
	
	// 17.12.2021: to be removed at some point... >>>
	maxTAG = std::max(ch1LastTag, ch2LastTag);
	// maxTAG++;
	maxTAG = maxTAG + ((int64_t)1);
	// cout << "Of " << ch1LastTag << " and " << ch2LastTag << " , max (+1) chosen: " << maxTAG << std::endl;

	// KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK
	// ToDo:
	//if (   (ch1LastTag > ch2LastTag) || 
	//	   ((ch1LastTag == ch2LastTag) && (ch1LastTime))   )
	// {
	//	*outMeasTime = ((1.0 / myAureaFreq) * (double)maxTAG) * (1.0e+3); // ms
	// }
	// <<<<
	
	*outMeasTime = ((1.0 / myAureaFreq) * (double)maxTAG) * (1.0e+3); // ms
	// *outMeasTime = llround(ceil(    ((1.0 / myAureaFreq) * maxTAG)   * (1.0e+3)   )); // ms
	// changed to double due to faulty operations at small acquisition times.

	// VKORN DESPERATE FLAG >>
	// fileOut1.flush();
	// fileOut2.flush();
	// fileOut1.close();
	// fileOut2.close();

	fflush(fileOut1);
	fflush(fileOut2);

	fclose(fileOut1);
	fclose(fileOut2);

	// << VKORN DESPERATE FLAG

	fileInHandle.close();
	
	
	// VKORN_TUESDAY >>
	// ***
	// Closing statement:
	// std::ios_base::sync_with_stdio(true);
	// ***
	// << VKORN_TUESDAY

	// fclose(fileInHandle);
	// fclose(fileOutHandle);

	return 0;
}

// <<< VER 2.


//	*	*	*	*	*	*	*	*	*			STAGE 2			*	*	*	*	*	*	*	*	*	*	//


//int PreProcessAureaDataStage2(const char* fileOutName, const char* fileInNameCh1, const char* fileInNameCh2,
//	const char* fileInNameHEADER, double inAureaPSin1Tag,
//	uint64_t inMeasTime, uint64_t NcntsTOT,
//	uint64_t* OUTnOVFLmarkersESTIMATE, uint64_t* OUTnOVFLmarkersREAL)
int PreProcessAureaDataStage2(const char* fileOutName, const char* fileInNameCh1, const char* fileInNameCh2,
	const char* fileInNameHEADER, double inAureaPSin1Tag,
	double inMeasTime, uint64_t Ncnts1, uint64_t Ncnts2,
	uint64_t* OUTnOVFLmarkersESTIMATE, uint64_t* OUTnOVFLmarkersREAL)

// COMBINE MULTIPLE CHANNEL FILES INTO ONE FILE //
// [<MeasTime>] == milliseconds!! //
{

	// ToDo: add buffer structure following readTTTRR*.c //

	// we use uint32_t variables for buffering!
	// also, overflow value is defined by the size of a time tag: 15 bits => 2^15.

	// cout << "myOverflowVal == " << myOverflowVal << std::endl;

	uint64_t nOVFLmarkersESTIMATE = (uint64_t)(trunc( inMeasTime*1.0e-3 / ((double)myOverflowVal*1.0e-12)     ));

	
	*OUTnOVFLmarkersESTIMATE = nOVFLmarkersESTIMATE;

	uint64_t nOVFLmarkersREAL = 0;
	// will be updated during the file processing...

	// open files >> //
	
	// VER1 :: header processing :: as a string :: 
	// std::ofstream fileOutHandle(fileOutName);
	// VER2 :: binary, from the start:
	std::ofstream fileOutHandle(fileOutName, std::ios::out | std::ios::binary);
	if (!fileOutHandle.is_open())
	{
		cout << std::endl << "Error opening output file." << std::endl;
		return 1;
	}
	// <<<
	
	// VKORN DESPERATE FLAG >>

	uint64_t CH1left = Ncnts1;
	uint64_t CH2left = Ncnts2;

	// std::ifstream fileInCh1Handle(fileInNameCh1, std::ios::in | std::ios::binary);
	// std::ifstream fileInCh2Handle(fileInNameCh2, std::ios::in | std::ios::binary);
	FILE* fileInCh1Handle;
	FILE* fileInCh2Handle;
	
	std::size_t unityForFWRITE = 1;

	std::ifstream fileInHEADERHandle(fileInNameHEADER);
	if (!fileInHEADERHandle.is_open())
	{
		fileOutHandle.close();
		cout << std::endl << "Error opening 'header' file." << std::endl;
		return 1;
	}
	// << VKORN DESPERATE FLAG

	

	
	/*
	else
	{

		if (!fileInCh1Handle.is_open())
		{
			fileOutHandle.close();
			cout << std::endl << "Error opening 'Ch1' file." << std::endl;
			return 1;
		}
		else
		{

			if (!fileInCh2Handle.is_open())
			{
				fileOutHandle.close();
				fileInCh1Handle.close();
				cout << std::endl << "Error opening 'Ch2' file." << std::endl;
				return 1;
			}
			else
			{

				if (!fileInHEADERHandle.is_open())
				{
					fileOutHandle.close();
					fileInCh1Handle.close();
					fileInCh2Handle.close();
					cout << std::endl << "Error opening 'header' file." << std::endl;
					return 1;
				}
				else
				{
					cout << std::endl << "Files opened succesfully." << std::endl;
				}
			}
		}
	}
	*/

	bool bFirstRun = true;

#ifdef AureaNoHeaderToResultFile
	// No fiddling with any headers...
#else

	// VKORN DESPERATE FLAG >>
	//	THIS PART IS SIMPLY DUPLICATED IN BELOW, TO REWRITE FILE AND GET CORRECT DATA TO IT
	// << VKORN DESPERATE FLAG
	
	// Pre-format header and write it to file: >>>
	

	// VER1: simply duplicate the header to output file;

	// std::string line = "";
	// std::istringstream issTMP;	// main string_stream for processing the file line-by-line
	// should be initialized with "line" above: "" issTMP.str(line); "" //

	// while (std::getline(fileInHEADERHandle, line))
//	{
//		if (bFirstRun)
//		{
//			bFirstRun = false;		
//			fileOutHandle << line.c_str();		// no endline symbol in front;
//		}
//		else
//		{
//			fileOutHandle << std::endl << line.c_str();
//		}
//	}
	
	// VER2 ::
	//			ToDo:	Process all the header file fields appropriately
	// Current version: just five necessary fields, manually:
	
	// TagRECTYPE includes first 16 bytes of the header.
	// When all the fields from the header are parsed, can be changed to standard PQ notation.
	
	
	std::string TagPREHEADER16bytes = "***AUREA FILE***";
	
	// Now: need to append Idx (int32_t), Type (uint32_t), and Value (8 bytes).
	std::string TagRECTYPE = "TTResultFormat_TTTRRecType";	// HydraHarp, T2
	std::string TagNUMREC = "TTResult_NumberOfRecords"; // + append (uint64) <NcntsTOT>
	std::string TagGLOBRES = "MeasDesc_GlobalResolution"; // 1 ps == standard
	std::string TagACQTIME = "MeasDesc_AcquisitionTime"; // + append <inMeasTime>
	std::string TagHEADEREND = "Header_End";

	//std::size_t stringPrinterSize = 32;
	// tag heading should be a null-terminated string, padded with zeroes.
	
	TagRECTYPE.append(((std::size_t)AUREABytesInATagName) - TagRECTYPE.length() +1, '\0');
	TagNUMREC.append(((std::size_t)AUREABytesInATagName) - TagNUMREC.length() +1, '\0');
	TagGLOBRES.append(((std::size_t)AUREABytesInATagName) - TagGLOBRES.length() +1, '\0');
	TagACQTIME.append(((std::size_t)AUREABytesInATagName) - TagACQTIME.length() +1, '\0');
	TagHEADEREND.append(((std::size_t)AUREABytesInATagName) - TagHEADEREND.length() +1, '\0');

	//		Demo for adding values (convert LONG to BYTE ARRAY):		//
	//char bytesOUTPUT[sizeof(NcntsTOT)];
	//std::copy(static_cast<const char*>(static_cast<const void*>(&NcntsTOT)),
	//	static_cast<const char*>(static_cast<const void*>(&NcntsTOT)) + sizeof NcntsTOT,
	//	bytesOUTPUT);
	//cout << NcntsTOT << "\t\t>" << bytesOUTPUT << std::endl;

#ifdef AureaProcessorPart2OverflowDebuggingMessages
	cout << TagRECTYPE << " \t" << TagRECTYPE.length() << std::endl;
	cout << TagNUMREC << " \t" << TagNUMREC.length() << std::endl;
	cout << TagGLOBRES << " \t" << TagGLOBRES.length() << std::endl;
	cout << TagACQTIME << " \t" << TagACQTIME.length() << std::endl;
	cout << TagHEADEREND << " \t" << TagHEADEREND.length() << std::endl;
#endif

	int32_t outputIdx = 1;
	uint32_t outputType;	// tyInt8 = 0x10000008; tyFloat8 = 0x20000008. //
	uint64_t outputValueUINT64 = 0;
	double outputValueDOUBLE = 0.0;

//	char bytesOUTPUT[sizeof(NcntsTOT)];	// do we need it?
// std::copy(static_cast<const char*>(static_cast<const void*>(&NcntsTOT)),
//	static_cast<const char*>(static_cast<const void*>(&NcntsTOT)) + sizeof NcntsTOT,
//	bytesOUTPUT);

	fileOutHandle.write((const char*)TagPREHEADER16bytes.c_str(), (TagPREHEADER16bytes.length()) * sizeof(char));
	
	// we want: rtPicoHarpT2     = 0x00010203,  # (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T2), HW: $03 (PicoHarp)
	// fileOutHandle.write((const char*)TagRECTYPE.c_str(), (TagRECTYPE.length())*sizeof(char));
	fileOutHandle.write((const char*)TagRECTYPE.c_str(), AUREABytesInATagName * sizeof(char));
	fileOutHandle.write((const char*)&outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
	// tyAnsiString  = 0x4001FFFF, tyBinaryBlob  = 0xFFFFFFFF
	fileOutHandle.write((const char*)&outputType, sizeof(uint32_t));
	outputValueUINT64 = 0x00010203; // PicoHarp, T2
	// rtHydraHarpT2 = 0x00010204, # (SubID = $00, RecFmt: $01) (V1), T - Mode: $02(T2), HW : $04(HydraHarp)
	fileOutHandle.write((const char*)& outputValueUINT64, sizeof(uint64_t));
	
	// fileOutHandle.write((const char*)TagNUMREC.c_str(), (TagNUMREC.length()) * sizeof(char));
	fileOutHandle.write((const char*)TagNUMREC.c_str(), AUREABytesInATagName * sizeof(char));
	fileOutHandle.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
	fileOutHandle.write((const char*)& outputType, sizeof(uint32_t));

	uint64_t ui64TempOutput = 0;
	ui64TempOutput = (Ncnts1 + Ncnts2) + nOVFLmarkersESTIMATE;
	fileOutHandle.write((const char*)&ui64TempOutput, sizeof(uint64_t));

	// fileOutHandle.write((const char*)TagGLOBRES.c_str(), (TagGLOBRES.length()) * sizeof(char));
	fileOutHandle.write((const char*)TagGLOBRES.c_str(), AUREABytesInATagName * sizeof(char));
	fileOutHandle.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x20000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
	fileOutHandle.write((const char*)& outputType, sizeof(uint32_t));
	outputValueDOUBLE = ((double)AUREAGlobalResolution);	// in seconds; DEFAULT!
	fileOutHandle.write((const char*)& outputValueDOUBLE, sizeof(double));

	// fileOutHandle.write((const char*)TagACQTIME.c_str(), (TagACQTIME.length()) * sizeof(char));
	fileOutHandle.write((const char*)TagACQTIME.c_str(), AUREABytesInATagName * sizeof(char));
	fileOutHandle.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // uint64_t ??? picoseconds??? //
	fileOutHandle.write((const char*)& outputType, sizeof(uint32_t));
	
	// VKORN_TUESDAY >>	
	// fileOutHandle.write((const char*)& inMeasTime, sizeof(uint64_t));

	// outputValueUINT64 = (uint64_t)inMeasTime;
	outputValueUINT64 = (uint64_t)(trunc(inMeasTime));
	fileOutHandle.write((const char*)& outputValueUINT64, sizeof(uint64_t));
	// << VKORN_TUESDAY

	// fileOutHandle.write((const char*)TagHEADEREND.c_str(), (TagHEADEREND.length()) * sizeof(char));
	fileOutHandle.write((const char*)TagHEADEREND.c_str(), AUREABytesInATagName * sizeof(char));
	fileOutHandle.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008. // here -- any.
	fileOutHandle.write((const char*)&outputType, sizeof(uint32_t));
	outputValueUINT64 = 0;
	fileOutHandle.write((const char*)&outputValueUINT64, sizeof(outputValueUINT64));
	// << Ver2 //

	// VKORN_TUESDAY >>
	// VKORN DESPERATE FLAG >>
	fileOutHandle.flush();

	// fflush(fileOutHandle);
	// << VKORN DESPERATE FLAG
	
	// VKORN DESPERATE FLAG >>
	fileOutHandle.close();

	Sleep(100);
	

	FILE* NEWOutHandle = fopen(fileOutName, "ab");
	if (NEWOutHandle == NULL) {
		cout << "Error re-opening the output file in binary mode" << std::endl;
		fileInHEADERHandle.close();
		return 1;
	}

	fileInCh1Handle = fopen(fileInNameCh1, "rb");
	if (fileInCh1Handle == NULL) {
		cout << "Error re-opening the input file for Ch.1" << std::endl;
		fileInHEADERHandle.close();
		fclose(NEWOutHandle);
		return 1;
	}

	fileInCh2Handle = fopen(fileInNameCh2, "rb");
	if (fileInCh2Handle == NULL) {
		cout << "Error re-opening the input file for Ch.2" << std::endl;
		fileInHEADERHandle.close();
		fclose(NEWOutHandle);
		fclose(fileInCh1Handle);
		return 1;
	}

	std::size_t freadRESULT;

	// <<< (Pre-formatting header).

#endif	// (AureaNoHeaderToResultFile).

	// VER1 :: header processing :: as a string :: 
	// >>>>>
	// Then reopen in binary mode for putting the numbers there... //
	//fileOutHandle.close();
	//fileOutHandle.open(fileOutName, ios::out | std::ios_base::app | ios::binary);
	//if (!fileOutHandle.is_open())
	//{
	//	fileInCh1Handle.close();
	//	fileInCh2Handle.close();
	//	fileInHEADERHandle.close();
	//	cout << std::endl << "Error opening 'header' file." << std::endl;
	//	return 1;
	//}
	//fileOutHandle.seekp(ios_base::end); // move to the end position in a file;

	// VER2 :: binary, from the start:
	// No additional action required!

	// Combine channels, convert from float(ns) to unsigned(25bit)(ps), add overflow tags.
	
	bool runFlag = true;
	
	// HydraHarp T2 mode record structure (in bits): //
	// OLD::: overflow_mark (1 bit)(MSB) || channel_No. (6 bits) || time_tag (25 bits) //
	// NEW::: overflow_mark OR channel_No. (4 bits)(MSB) || time_tag (28 bits) //
	// 0b0000 == 0 == ch1; 0b0001 == 1 == ch2; etc. //
	// 0b1111 == 15 == overflow event; //
	
	uint32_t	CurrTime = 0;	// incremental time for a loop;

	// VKORN_TUESDAY >>
	// NB! AFTER AN OVERFLOW, RECALCULATE TIME FOR OTHER CHANNEL ALSO!!
	// UPD: no, that's not the case... ((
	// bool		bOverflowFlag = false; // true if CurrTime exceeds myOverflowVal;
	// << VKORN_TUESDAY

#ifdef AureaReverseChannelAndTimeData
	uint32_t	myCh1Mask = ((uint32_t)0); // number of channel (from 0) NOT shifted by N_bits_TimeTag to the left.
	uint32_t	myCh2Mask = ((uint32_t)1); // number of channel (from 0) NOT shifted by N_bits_TimeTag to the left.
	uint32_t	myOVFLMask = ((uint32_t)15);  // LSB;	// = 28 times 0b0, then 0b1111.
#else
	uint32_t	myCh1Mask = ((uint32_t)0) << 28; // number of channel (from 0) shifted by N_bits_TimeTag to the left.
	uint32_t	myCh2Mask = ((uint32_t)1) << 28; // number of channel (from 0) shifted by N_bits_TimeTag to the left.
	// OLD: uint32_t	myOVFLMask = ((uint32_t)1) << 31; // MSB;
	// NEW:
	uint32_t	myOVFLMask = ((uint32_t)15) << 28; // MSB;	// = 0b1111 + 28 more bit values.
#endif

	// DEBUG PRINT: >>
	cout << "Defined masks (Ch1 // Ch2 // OverflowBit):" << std::endl;
	
	cout << std::bitset<32>(myCh1Mask) << std::endl;
	cout << std::bitset<32>(myCh2Mask) << std::endl;
	cout << std::bitset<32>(myOVFLMask) << std::endl;

	// <<

	// inAureaPSin1Tag :: double, ps in 1 Tag;
	// debug print: >>
	// cout << "PS in 1 Tag: " << inAureaPSin1Tag << std::endl;

	bool bCh1empty = false;
	bool bCh2empty = false;

	uint64_t	ch1tag = 0;		// tags, see <inAureaPSin1Tag>
	double		ch1time = 0.0;	// ns
	uint64_t	ch2tag = 0;		// tags, see <inAureaPSin1Tag>
	double		ch2time = 0.0;	// ns

	// Used to cope with overflow events; stores the last {time,tag} for an overflow event;
	// (add <myOverflowVal> (ps) and convert it to time and tag)
	uint64_t	lastOVFLtag = 0;		// tags
	double		lastOVFLtime = 0.0;		// ns

	uint64_t	hlpOVFLtrunc = ((uint64_t)trunc(((double)myOverflowVal) / inAureaPSin1Tag));
	// aux for calculations;
	// convert current excess time into {Tag, Time} pair.
				
	// <<

	uint32_t	convertedCH1timePS = 0; // after subtracting <lastOVFL> and converting to ps //
	uint32_t	convertedCH2timePS = 0;

	std::size_t	helpSizeUINT32 = sizeof(uint32_t); // for I/O operations //
	std::size_t	helpSizeDOUBLE = sizeof(double);
	std::size_t	helpSizeUINT64 = sizeof(uint64_t);

	// fileInCh1Handle >> ch1tag >> ch1time;
	// fileInCh2Handle >> ch2tag >> ch2time;

	// VKORN_TUESDAY >>
	// fileInCh1Handle.read((char*)&ch1tag, helpSizeUINT64);
	// fileInCh1Handle.read((char*)&ch1time, helpSizeDOUBLE);
	// fileInCh2Handle.read((char*)&ch2tag, helpSizeUINT64);
	// fileInCh2Handle.read((char*)&ch2time, helpSizeDOUBLE);

	// fileInCh1Handle >> ch1tag;
	// fileInCh1Handle >> ch1time;
	// fileInCh2Handle >> ch2tag;
	// fileInCh2Handle >> ch2time;

	// VKORN DESPERATE FLAG >>
	// fileInCh1Handle.read(reinterpret_cast<char*>(&ch1tag), helpSizeUINT64);
	// fileInCh1Handle.read(reinterpret_cast<char*>(&ch1time), helpSizeDOUBLE);
	// fileInCh2Handle.read(reinterpret_cast<char*>(&ch2tag), helpSizeUINT64);
	// fileInCh2Handle.read(reinterpret_cast<char*>(&ch2time), helpSizeDOUBLE);

	fread(&ch1tag, helpSizeUINT64, unityForFWRITE, fileInCh1Handle);
	fread(&ch1time, helpSizeDOUBLE, unityForFWRITE, fileInCh1Handle);
	fread(&ch2tag, helpSizeUINT64, unityForFWRITE, fileInCh2Handle);
	fread(&ch2time, helpSizeDOUBLE, unityForFWRITE, fileInCh2Handle);

	CH1left = CH1left - (uint64_t)1;
	CH2left = CH2left - (uint64_t)1;

	// << VKORN DESPERATE FLAG
	// << VKORN_TUESDAY

// #ifdef AureaProcessorPart2OverflowDebuggingMessages
	cout << "First record:" << std::endl;
	cout << "ch1tag: " << ch1tag << " \tch1time: " << ch1time << std::endl;
	cout << "ch2tag: " << ch2tag << " \tch2time: " << ch2time << std::endl;
// #endif

#ifdef AureaProcessorPart2OverflowDebuggingMessages
	long int i = 0;
#endif
	
	cout << std::endl << std::endl << "entering the output loop:" << std::endl;

#ifdef AureaProcessorPart2OverflowDebuggingMessages
	while ((runFlag) && (i<10))
#else
	while (runFlag)
#endif
	{
		if ((!bCh2empty) && (   (bCh1empty) || (ch1tag > ch2tag) || ((ch1tag == ch2tag) && (ch1time > ch2time))   ))
// use CH2 if (a) it is not empty AND (b1) no values are left in CH1 or (b2) if CH1 curr. record time > CH2 curr. record time //
		{	
					// write CH2, keep CH1 //
#ifdef AureaProcessorPart2OverflowDebuggingMessages
			cout << "ch2 write" << std::endl;
#endif
			// convertedCH2timePS = (uint32_t)lround(((ch2tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch2time - lastOVFLtime));
			convertedCH2timePS = (uint32_t)round(((ch2tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch2time - lastOVFLtime));

			// 1e3: ns -> ps //
			// AVERAGE TO UNITS OF PICOSECONDS //

			// Overflow check:
			// OLD: // if (convertedCH2timePS > myOverflowVal)
			// NOW SHOULD BE CAPABLE OF ADDING MULTIPLE OVERFLOW EVENTS //
			// WITHOUT RUINING UP THE TIME VALUE //
			while (convertedCH2timePS > myOverflowVal)
			{
				// increase OVFL marker, recalculate value
				// hlpOVFLtrunc = ((uint64_t)trunc(  ((double)myOverflowVal) / inAureaPSin1Tag  ));
				// << defined before <while> loop.

#ifdef AureaProcessorPart2OverflowDebuggingMessages
				cout << "old EVENT time: " << convertedCH2timePS << std::endl;
				cout << "old OVFL tag = " << lastOVFLtag << " \t" << "old OVFL time = " << lastOVFLtime << std::endl;
#endif
				lastOVFLtag = lastOVFLtag + hlpOVFLtrunc;
				// OLD: // lastOVFLtime = lastOVFLtime + (1e-3) * (((double)myOverflowVal) - (hlpOVFLtrunc * inAureaPSin1Tag));
				
				// lastOVFLtime = lastOVFLtime + (1e-3)*(((double)myOverflowVal) - (((double)hlpOVFLtrunc) * inAureaPSin1Tag));
				// Nov.20: round to <ps> level:
				lastOVFLtime = (1.0e-3) * round(lastOVFLtime*(1.0e3) + (((double)myOverflowVal) - (((double)hlpOVFLtrunc) * inAureaPSin1Tag)));
				// 1e-3: ps -> ns;
				// NOW: AVERAGED TO UNITS OF PICOSECONDS //

// TODO: 
// here, a WHILE loop should be...

				if (lastOVFLtime > (inAureaPSin1Tag * (1.0e-3))) // can occur only once...
				{
					lastOVFLtag = lastOVFLtag + (uint32_t)1;
					// lastOVFLtime = lastOVFLtime - (inAureaPSin1Tag * (1e-3));
					lastOVFLtime = (1.0e-3) * round(lastOVFLtime*(1.0e3) - inAureaPSin1Tag);
					// NOW: AVERAGED TO UNITS OF PICOSECONDS //
				}
				
#ifdef AureaProcessorPart2OverflowDebuggingMessages
				cout << "new OVFL tag = " << lastOVFLtag << " \t" << "new OVFL time = " << lastOVFLtime << std::endl;
#endif
				// Write an overflow event:
				convertedCH2timePS = ((uint32_t)0) | myOVFLMask;
				
				// VKORN_TUESDAY >>
				// OLD:
				// fileOutHandle.write((const char*)& convertedCH2timePS, helpSizeUINT32); // OVFL marker written;

				// NEW: text only ((
				// fileOutHandle << convertedCH2timePS;

				// NEW: binary:
				// VKORN DESPERATE FLAG >>
				// fileOutHandle.write(reinterpret_cast<char*>(&convertedCH2timePS), helpSizeUINT32); // OVFL marker written;
				fwrite(&convertedCH2timePS, helpSizeUINT32, unityForFWRITE, NEWOutHandle);
				// << VKORN DESPERATE FLAG
				// << VKORN_TUESDAY
				
				// increase the counter of overflow events;
				nOVFLmarkersREAL = nOVFLmarkersREAL + ((uint64_t)1);

				// decrease time by 1 "overflow value":
				// convertedCH2timePS = (uint32_t)lround(((ch2tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch2time - lastOVFLtime));
				convertedCH2timePS = (uint32_t)round(((ch2tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch2time - lastOVFLtime));
				
				// 1e-3: ps -> ns;
#ifdef AureaProcessorPart2OverflowDebuggingMessages
				cout << "new EVENT time: " << convertedCH2timePS << std::endl;
#endif

			}

			// Apply the channel mask:
#ifdef AureaReverseChannelAndTimeData
			convertedCH2timePS = (convertedCH2timePS << 4) | myCh2Mask; // first Time, then Channel
#else
			convertedCH2timePS = convertedCH2timePS | myCh2Mask; // first Channel (MSB), then Time
#endif
			
			// fileOutHandle << convertedCH2timePS;
			// VKORN_TUESDAY >>
			// OLD:
			// fileOutHandle.write((const char*)&convertedCH2timePS, helpSizeUINT32);

			// NEW: text only ((
			// fileOutHandle << convertedCH2timePS;

			// NEW: binary
			// VKORN DESPERATE FLAG >>
			//fileOutHandle.write(reinterpret_cast<char*>(&convertedCH2timePS), helpSizeUINT32);
			fwrite(&convertedCH2timePS, helpSizeUINT32, unityForFWRITE, NEWOutHandle);
			// << VKORN DESPERATE FLAG
			// VKORN_TUESDAY >>

			// Get new value from file for CH2:  (mark file end if needed:)
			//if (fileInCh2Handle >> ch2tag >> ch2time)

			// VKORN_TUESDAY >>
			// fileInCh2Handle.read((char*)& ch2tag, helpSizeUINT64);
			// fileInCh2Handle.read((char*)& ch2time, helpSizeDOUBLE);
			
			// fileInCh2Handle >> ch2tag;
			// fileInCh2Handle >> ch2time;

			// VKORN DESPERATE FLAG >>
			//fileInCh2Handle.read(reinterpret_cast<char*>(&ch2tag), helpSizeUINT64);
			//fileInCh2Handle.read(reinterpret_cast<char*>(&ch2time), helpSizeDOUBLE);

			fread(&ch2tag, helpSizeUINT64, unityForFWRITE, fileInCh2Handle);
			freadRESULT = fread(&ch2time, helpSizeDOUBLE, unityForFWRITE, fileInCh2Handle);

			CH2left = CH2left - (uint64_t)1;
			
			// << VKORN_TUESDAY

			// if (fileInCh2Handle.good())
			// if ((freadRESULT == helpSizeDOUBLE) && (CH2left != 0))
			if (CH2left > 0)
			// << VKORN DESPERATE FLAG
			{
				// O.K.
#ifdef AureaProcessorPart2OverflowDebuggingMessages
				cout << "ch2tag: " << ch2tag << " \tch2time: " << ch2time << std::endl;
#endif
			}
			else
			{
				bCh2empty = true;
				if (bCh1empty == true)
				{
					runFlag = false;
				}
			}
		}
		else   // CH2 is empty, OR (CH1 not empty and CH1 time is less than in CH2) //
		{
			// write CH1, keep CH2 //
#ifdef AureaProcessorPart2OverflowDebuggingMessages
			cout << "ch1 write" << std::endl;
#endif

			// convertedCH1timePS = (uint32_t)lround(((ch1tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch1time - lastOVFLtime));
			convertedCH1timePS = (uint32_t)round(((ch1tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch1time - lastOVFLtime));
			// 1e3: ns -> ps //

			// Overflow check:
			// OLD: // if (convertedCH1timePS > myOverflowVal)
			// NOW SHOULD BE CAPABLE OF ADDING MULTIPLE OVERFLOW EVENTS //
			// WITHOUT RUINING UP THE TIME VALUE //
			while (convertedCH1timePS > myOverflowVal)
			{
				// increase OVFL marker, recalculate value
				// hlpOVFLtrunc = ((uint64_t)trunc(((double)myOverflowVal) / inAureaPSin1Tag));
				// << defined before <while> loop.
				
#ifdef AureaProcessorPart2OverflowDebuggingMessages
				cout << "old EVENT time: " << convertedCH1timePS << std::endl;
				cout << "old OVFL tag = " << lastOVFLtag << " \t" << "old OVFL time = " << lastOVFLtime << std::endl;
#endif

				lastOVFLtag = lastOVFLtag + hlpOVFLtrunc;
				// OLD: // lastOVFLtime = lastOVFLtime + (1e-3) * (((double)myOverflowVal) - (hlpOVFLtrunc * inAureaPSin1Tag));
				
				// lastOVFLtime = lastOVFLtime + (1e-3) * (((double)myOverflowVal) - (((double)hlpOVFLtrunc) * inAureaPSin1Tag));
				// Nov.20: round to <ps> level:
				lastOVFLtime = (1.0e-3) * round(lastOVFLtime * 1.0e3 + (((double)myOverflowVal) - (((double)hlpOVFLtrunc) * inAureaPSin1Tag)));
				// NOW: AVERAGED TO UNITS OF PICOSECONDS //
				// 1e-3: ps -> ns;

				if (lastOVFLtime > (inAureaPSin1Tag * (1.0e-3))) // can occur only once...
				{
					lastOVFLtag = lastOVFLtag + (uint32_t)1;
					// lastOVFLtime = lastOVFLtime - (inAureaPSin1Tag * (1e-3));
					lastOVFLtime = (1.0e-3) * round(lastOVFLtime * (1.0e3) - inAureaPSin1Tag);
					// NOW: AVERAGED TO UNITS OF PICOSECONDS //
				}
				
#ifdef AureaProcessorPart2OverflowDebuggingMessages
				cout << "new OVFL tag = " << lastOVFLtag << " \t" << "new OVFL time = " << lastOVFLtime << std::endl;
#endif
				// Write an overflow event:
				convertedCH1timePS = ((uint32_t)0) | myOVFLMask;

				// VKORN_TUESDAY >>
				// OLD:
				// fileOutHandle.write((const char*)& convertedCH1timePS, helpSizeUINT32); // OVFL marker written;

				// NEW: text only ((
				// fileOutHandle << convertedCH1timePS;

				// NEW: binary
				// VKORN DESPERATE FLAG >>
				// fileOutHandle.write(reinterpret_cast<char*>(&convertedCH1timePS), helpSizeUINT32); // OVFL marker written;
				// << VKORN DESPERATE FLAG
				fwrite(&convertedCH1timePS, helpSizeUINT32, unityForFWRITE, NEWOutHandle);
				// << VKORN_TUESDAY

				// increase the counter of overflow events;
				nOVFLmarkersREAL = nOVFLmarkersREAL + ((uint64_t)1);

				// decrease time by 1 "overflow value":
				// convertedCH1timePS = (uint32_t)lround(((ch1tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch1time - lastOVFLtime));
				convertedCH1timePS = (uint32_t)round(((ch1tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch1time - lastOVFLtime));

#ifdef AureaProcessorPart2OverflowDebuggingMessages
				cout << "new EVENT time: " << convertedCH1timePS << std::endl;
#endif

			}
			
			// Just apply the channel mask:
#ifdef AureaReverseChannelAndTimeData
			convertedCH1timePS = (convertedCH1timePS << 4) | myCh1Mask; // first Time (MSB), then channel No.
#else
			convertedCH1timePS = convertedCH1timePS | myCh1Mask; // first Channel (MSB), then Time
#endif

			// fileOutHandle << convertedCH1timePS;
			// VKORN_TUESDAY >>
			// OLD:
			// fileOutHandle.write((const char*)&convertedCH1timePS, helpSizeUINT32);
			
			// NEW: text only ((
			// fileOutHandle << convertedCH1timePS;

			// NEW: binary
			// VKORN DESPERATE FLAG >>
			// fileOutHandle.write(reinterpret_cast<char*>(&convertedCH1timePS), helpSizeUINT32);
			// << VKORN DESPERATE FLAG
			fwrite(&convertedCH1timePS, helpSizeUINT32, unityForFWRITE, NEWOutHandle);
			// << VKORN_TUESDAY

			// Get new value from file for CH1:  (mark file end if needed:)
			// if (fileInCh1Handle >> ch1tag >> ch1time)
			
			// VKORN_TUESDAY >>
			// fileInCh1Handle.read((char*)& ch1tag, helpSizeUINT64);
			// fileInCh1Handle.read((char*)& ch1time, helpSizeDOUBLE);
			
			// fileInCh1Handle >> ch1tag;
			// fileInCh1Handle >> ch1time;

			// VKORN DESPERATE FLAG >>
			// fileInCh1Handle.read(reinterpret_cast<char*>(&ch1tag), helpSizeUINT64);
			// fileInCh1Handle.read(reinterpret_cast<char*>(&ch1time), helpSizeDOUBLE);

			fread(&ch1tag, helpSizeUINT64, unityForFWRITE, fileInCh1Handle);
			freadRESULT = fread(&ch1time, helpSizeDOUBLE, unityForFWRITE, fileInCh1Handle);

			CH1left = CH1left - (uint64_t)1;

			// << VKORN_TUESDAY
			// if (fileInCh1Handle.good())

			// if ((freadRESULT != helpSizeDOUBLE) && (CH1left != 0))
			if (CH1left > 0)
					// << VKORN DESPERATE FLAG
			{
				// O.K.
#ifdef AureaProcessorPart2OverflowDebuggingMessages
				cout << "ch1tag: " << ch1tag << " \tch1time: " << ch1time << std::endl;
#endif
			}
			else
			{
				bCh1empty = true;
				if (bCh2empty == true)
				{
					runFlag = false;
				}
			}
			
		} // << CH1 <<

#ifdef AureaProcessorPart2OverflowDebuggingMessages
		i++;
#endif

	} // while (runFlag);

	*OUTnOVFLmarkersREAL = nOVFLmarkersREAL;
	cout << "Overflows: \tEST = " << nOVFLmarkersESTIMATE << " \tREAL = " << nOVFLmarkersREAL << std::endl;

	cout << "closing the files" << std::endl;

	// VKORN DESPERATE FLAG >>
	// fileOutHandle.flush();
	// fileOutHandle.close();

	fflush(NEWOutHandle);
	fclose(NEWOutHandle);
	
	// fileInCh1Handle.close();
	// fileInCh2Handle.close();
	fclose(fileInCh1Handle);
	fclose(fileInCh2Handle);
	// << VKORN DESPERATE FLAG

	fileInHEADERHandle.close();


	// >>>>>>>>>>>>>>>>>>>>>>>>>
	// >>>>>>>>>>>>>>>>>>>>>>>>>

	Sleep(1000);

	// fileOutHandle.open(fileInNameHEADER, std::ifstream::in);
	
	// fileOutHandle.open(fileOutName, std::ios::out | std::ios::app);
	// fileOutHandle.open(fileOutName);
	
	// fileOutHandle.open(fileOutName, ios::out);
	//if (!fileOutHandle.is_open())
	
	
	// NEWOutHandle = fopen(fileOutName, "w");
	// fseek(NEWOutHandle, 0, SEEK_SET);
	// if (NEWOutHandle == NULL)

	// WORKS ! >>>>
	
	std::fstream OutOutOuttemp(fileOutName); // use option std::ios_base::binary if necessary
	// std::ios_base::out | std::ios_base::in
	OutOutOuttemp.seekp(0, std::ios_base::beg);
	
	if (OutOutOuttemp.fail())
	{
		cout << std::endl << "Error opening output file." << std::endl;
		return 1;
	}

	// <<<< WORKS !

	// fileOutHandle.seekp(0);

	// fileOutHandle.write((const char*)TagPREHEADER16bytes.c_str(), (TagPREHEADER16bytes.length()) * sizeof(char));

	//fwrite((const void*)TagPREHEADER16bytes.c_str(),
	//	(TagPREHEADER16bytes.length()) * sizeof(char), unityForFWRITE, NEWOutHandle);
	//fclose(NEWOutHandle);
	//return 0;

	OutOutOuttemp.write((const char*)TagPREHEADER16bytes.c_str(), (TagPREHEADER16bytes.length()) * sizeof(char));
	
	// we want: rtPicoHarpT2     = 0x00010203,  # (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T2), HW: $03 (PicoHarp)
	OutOutOuttemp.write((const char*)TagRECTYPE.c_str(), AUREABytesInATagName * sizeof(char));
	OutOutOuttemp.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
	// tyAnsiString  = 0x4001FFFF, tyBinaryBlob  = 0xFFFFFFFF
	OutOutOuttemp.write((const char*)& outputType, sizeof(uint32_t));
	outputValueUINT64 = 0x00010203; // PicoHarp, T2
	// rtHydraHarpT2 = 0x00010204, # (SubID = $00, RecFmt: $01) (V1), T - Mode: $02(T2), HW : $04(HydraHarp)
	OutOutOuttemp.write((const char*)& outputValueUINT64, sizeof(uint64_t));

	OutOutOuttemp.write((const char*)TagNUMREC.c_str(), AUREABytesInATagName * sizeof(char));
	OutOutOuttemp.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
	OutOutOuttemp.write((const char*)& outputType, sizeof(uint32_t));

	// HERE >>
	// ui64TempOutput = (Ncnts1 + Ncnts2) + nOVFLmarkersESTIMATE;
	ui64TempOutput = (Ncnts1 + Ncnts2) + nOVFLmarkersREAL; // 'cause we know that now.
	// <<<
	
	OutOutOuttemp.write((const char*)& ui64TempOutput, sizeof(uint64_t));

	OutOutOuttemp.write((const char*)TagGLOBRES.c_str(), AUREABytesInATagName * sizeof(char));
	OutOutOuttemp.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x20000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
	OutOutOuttemp.write((const char*)& outputType, sizeof(uint32_t));
	outputValueDOUBLE = ((double)AUREAGlobalResolution);	// in seconds; DEFAULT!
	OutOutOuttemp.write((const char*)& outputValueDOUBLE, sizeof(double));

	OutOutOuttemp.write((const char*)TagACQTIME.c_str(), AUREABytesInATagName * sizeof(char));
	OutOutOuttemp.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // uint64_t ??? picoseconds??? //
	OutOutOuttemp.write((const char*)& outputType, sizeof(uint32_t));

	// VKORN_TUESDAY >>	
	// outputValueUINT64 = (uint64_t)inMeasTime;
	outputValueUINT64 = (uint64_t)(trunc(inMeasTime));
	OutOutOuttemp.write((const char*)& outputValueUINT64, sizeof(uint64_t));
	// << VKORN_TUESDAY

	OutOutOuttemp.write((const char*)TagHEADEREND.c_str(), AUREABytesInATagName * sizeof(char));
	OutOutOuttemp.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008. // here -- any.
	OutOutOuttemp.write((const char*)& outputType, sizeof(uint32_t));
	outputValueUINT64 = 0;
	OutOutOuttemp.write((const char*)& outputValueUINT64, sizeof(outputValueUINT64));
	// << Ver2 //

	// VKORN_TUESDAY >>
	// VKORN DESPERATE FLAG >>
	OutOutOuttemp.flush();

	Sleep(500);

	// fflush(fileOutHandle);
	// << VKORN DESPERATE FLAG

	// VKORN DESPERATE FLAG >>
	OutOutOuttemp.close();

	

	// <<<<<<<<<<<<<<<<<<<<<<<<<
	// <<<<<<<<<<<<<<<<<<<<<<<<<

	return 0;
}















//	*	*	*	*	*	*	*	*	*			STAGE 3			*	*	*	*	*	*	*	*	*	*	//












// ??? why is nOVFLmarkersESTIMATE value (that is output every time) changes?
// it should be constant, from the code...


// Stage 2 is obsolete and will be removed... //


// Copied from Stage2 on Nov.23, modified SIGNIFICANTLY (!).
int PreProcessAureaDataStage3splitter(double TimeToSplitSEC, const char* fileOutName, const char* fileInNameCh1, const char* fileInNameCh2,
	const char* fileInNameHEADER, double inAureaPSin1Tag,
	double inMeasTime, uint64_t Ncnts1, uint64_t Ncnts2,
	uint64_t* OUTnOVFLmarkersESTIMATE, uint64_t* OUTnOVFLmarkersREAL)
	// [<MeasTime>] == milliseconds!! //
{
	// ToDo: add buffer structure following readTTTRR*.c //

	
	*OUTnOVFLmarkersREAL = 0; // will be summed up;
	uint64_t nOVFLmarkersREAL = 0;
	// will be updated during the file processing...

	// QQQQQQQQQQQQQQQQQQQQ
	
	// int BUBnFiles = (int)trunc( (1.0e-3) * inMeasTime / TimeToSplitSEC);
	int BUBnFiles;
	BUBnFiles = (int)(std::ceil( (double)(1.0e-3) * inMeasTime / TimeToSplitSEC));

	cout << "BUBnFiles: " << BUBnFiles << " \tinMeasTime: " << inMeasTime << " \tTimeToSplitSEC: ";
	cout << TimeToSplitSEC << std::endl;

	int BUBcurrFileN = 1;
	std::string BUBcurrOutputFileName; // will be used with <fileOutName>
	std::string BUBtmpName; // will be used with <fileOutName>
	cout << "N files to create: " << BUBnFiles << std::endl;
	// ms / s .

	uint64_t nOVFLmarkersESTIMATE = (uint64_t)(trunc(
		(inMeasTime / ((double)BUBnFiles))	* 1.0e-3 / ((double)myOverflowVal * 1.0e-12)
	));
	
	// some demo code changes...

	*OUTnOVFLmarkersESTIMATE = nOVFLmarkersESTIMATE;

	// open files >> //

	// VER1 :: header processing :: as a string :: 
	// std::ofstream fileOutHandle(fileOutName);
	// VER2 :: binary, from the start:
	
	// BUBBUBBUB
	BUBcurrOutputFileName = fileOutName;
	BUBcurrOutputFileName.append(std::to_string(BUBcurrFileN));
	BUBcurrOutputFileName.append(".dat");
	std::ifstream BUBcopyFROM;
	std::ofstream BUBcopyTO;

	uint64_t BUBtotalItems;
	// for counting for individual files.

	// cout << "File name demo: " << BUBcurrOutputFileName.c_str() << std::endl;

	std::ofstream fileOutHandle(BUBcurrOutputFileName.c_str(), std::ios::out | std::ios::binary);
	if (!fileOutHandle.is_open())
	{
		cout << std::endl << "Error opening output file." << std::endl;
		return 1;
	}
	// <<<

	// VKORN DESPERATE FLAG >>

	// std::ifstream fileInCh1Handle(fileInNameCh1, std::ios::in | std::ios::binary);
	// std::ifstream fileInCh2Handle(fileInNameCh2, std::ios::in | std::ios::binary);
	FILE* fileInCh1Handle;
	FILE* fileInCh2Handle;

	std::size_t unityForFWRITE = 1;

	std::ifstream fileInHEADERHandle(fileInNameHEADER);
	if (!fileInHEADERHandle.is_open())
	{
		fileOutHandle.close();
		cout << std::endl << "Error opening 'header' file." << std::endl;
		return 1;
	}
	// << VKORN DESPERATE FLAG

	bool bFirstRun = true;

	// Pre-format header and write it to file: >>>

	// VER2 ::
	//			ToDo:	Process all the header file fields appropriately
	// Current version: just five necessary fields, manually:

	
	// When all the fields from the header are parsed, can be changed to standard PQ notation.

	// Very bad solution: cast all the others to tyInt8 = 0x10000008, //

	std::string TagPREHEADER16bytes = "***AUREA FILE***";
	
	//  Firt 16 bytes of the PTU file are a header.

	// Now: need to append Idx (int32_t), Type (uint32_t), and Value (8 bytes).
	std::string TagRECTYPE = "TTResultFormat_TTTRRecType";	// HydraHarp, T2
	std::string TagNUMREC = "TTResult_NumberOfRecords"; // + append (uint64) <NcntsTOT>
	std::string TagGLOBRES = "MeasDesc_GlobalResolution"; // 1 ps == standard
	std::string TagACQTIME = "MeasDesc_AcquisitionTime"; // + append <inMeasTime>
	std::string TagHEADEREND = "Header_End";

	//std::size_t stringPrinterSize = 32;
	// tag heading should be a null-terminated string, padded with zeroes.

	// +1 :: obsolete; can be removed, probably...
	TagRECTYPE.append(((std::size_t)AUREABytesInATagName) - TagRECTYPE.length() + 1, '\0');
	TagNUMREC.append(((std::size_t)AUREABytesInATagName) - TagNUMREC.length() + 1, '\0');
	TagGLOBRES.append(((std::size_t)AUREABytesInATagName) - TagGLOBRES.length() + 1, '\0');
	TagACQTIME.append(((std::size_t)AUREABytesInATagName) - TagACQTIME.length() + 1, '\0');
	TagHEADEREND.append(((std::size_t)AUREABytesInATagName) - TagHEADEREND.length() + 1, '\0');

	//		Demo for adding values (convert LONG to BYTE ARRAY):		//
	//char bytesOUTPUT[sizeof(NcntsTOT)];
	//std::copy(static_cast<const char*>(static_cast<const void*>(&NcntsTOT)),
	//	static_cast<const char*>(static_cast<const void*>(&NcntsTOT)) + sizeof NcntsTOT,
	//	bytesOUTPUT);
	//cout << NcntsTOT << "\t\t>" << bytesOUTPUT << std::endl;

#ifdef AureaProcessorPart2OverflowDebuggingMessages
	cout << TagRECTYPE << " \t" << TagRECTYPE.length() << std::endl;
	cout << TagNUMREC << " \t" << TagNUMREC.length() << std::endl;
	cout << TagGLOBRES << " \t" << TagGLOBRES.length() << std::endl;
	cout << TagACQTIME << " \t" << TagACQTIME.length() << std::endl;
	cout << TagHEADEREND << " \t" << TagHEADEREND.length() << std::endl;
#endif

	int32_t outputIdx = 1;
	uint32_t outputType;	// tyInt8 = 0x10000008; tyFloat8 = 0x20000008. //
	uint64_t outputValueUINT64 = 0;
	double outputValueDOUBLE = 0.0;

	//	char bytesOUTPUT[sizeof(NcntsTOT)];	// do we need it?
	// std::copy(static_cast<const char*>(static_cast<const void*>(&NcntsTOT)),
	//	static_cast<const char*>(static_cast<const void*>(&NcntsTOT)) + sizeof NcntsTOT,
	//	bytesOUTPUT);

	fileOutHandle.write((const char*)TagPREHEADER16bytes.c_str(), (TagPREHEADER16bytes.length()) * sizeof(char));

	// we want: rtPicoHarpT2     = 0x00010203,  # (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T2), HW: $03 (PicoHarp)
	// fileOutHandle.write((const char*)TagRECTYPE.c_str(), (TagRECTYPE.length())*sizeof(char));
	fileOutHandle.write((const char*)TagRECTYPE.c_str(), AUREABytesInATagName * sizeof(char));
	fileOutHandle.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
	// tyAnsiString  = 0x4001FFFF, tyBinaryBlob  = 0xFFFFFFFF
	fileOutHandle.write((const char*)& outputType, sizeof(uint32_t));
	outputValueUINT64 = 0x00010203; // PicoHarp, T2
	// rtHydraHarpT2 = 0x00010204, # (SubID = $00, RecFmt: $01) (V1), T - Mode: $02(T2), HW : $04(HydraHarp)
	fileOutHandle.write((const char*)& outputValueUINT64, sizeof(uint64_t));

	// fileOutHandle.write((const char*)TagNUMREC.c_str(), (TagNUMREC.length()) * sizeof(char));
	fileOutHandle.write((const char*)TagNUMREC.c_str(), AUREABytesInATagName * sizeof(char));
	fileOutHandle.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
	fileOutHandle.write((const char*)& outputType, sizeof(uint32_t));

	uint64_t ui64TempOutput = 0;
	ui64TempOutput = (Ncnts1 + Ncnts2) + nOVFLmarkersESTIMATE;
	fileOutHandle.write((const char*)& ui64TempOutput, sizeof(uint64_t));

	// fileOutHandle.write((const char*)TagGLOBRES.c_str(), (TagGLOBRES.length()) * sizeof(char));
	fileOutHandle.write((const char*)TagGLOBRES.c_str(), AUREABytesInATagName * sizeof(char));
	fileOutHandle.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x20000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
	fileOutHandle.write((const char*)& outputType, sizeof(uint32_t));
	outputValueDOUBLE = ((double)AUREAGlobalResolution);	// in seconds; DEFAULT!
	fileOutHandle.write((const char*)& outputValueDOUBLE, sizeof(double));

	// fileOutHandle.write((const char*)TagACQTIME.c_str(), (TagACQTIME.length()) * sizeof(char));
	fileOutHandle.write((const char*)TagACQTIME.c_str(), AUREABytesInATagName * sizeof(char));
	fileOutHandle.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // uint64_t ??? picoseconds??? //
	fileOutHandle.write((const char*)& outputType, sizeof(uint32_t));

	// VKORN_TUESDAY >>	
	// fileOutHandle.write((const char*)& inMeasTime, sizeof(uint64_t));

	// outputValueUINT64 = (uint64_t)inMeasTime;
	
	// outputValueUINT64 = (uint64_t)(trunc(inMeasTime));
	// STAGE3 (SPLITTER) >>>>>
	outputValueUINT64 = (uint64_t)(trunc(TimeToSplitSEC));
	// STAGE3 (SPLITTER) <<<<<
	fileOutHandle.write((const char*)& outputValueUINT64, sizeof(uint64_t));
	// << VKORN_TUESDAY

	// fileOutHandle.write((const char*)TagHEADEREND.c_str(), (TagHEADEREND.length()) * sizeof(char));
	fileOutHandle.write((const char*)TagHEADEREND.c_str(), AUREABytesInATagName * sizeof(char));
	fileOutHandle.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008. // here -- any.
	fileOutHandle.write((const char*)& outputType, sizeof(uint32_t));
	outputValueUINT64 = 0;
	fileOutHandle.write((const char*)& outputValueUINT64, sizeof(outputValueUINT64));
	// << Ver2 //

	cout << "HEADER WRITTEN" << std::endl;

	// VKORN_TUESDAY >>
	// VKORN DESPERATE FLAG >>
	fileOutHandle.flush();

	// fflush(fileOutHandle);
	// << VKORN DESPERATE FLAG

	// VKORN DESPERATE FLAG >>
	fileOutHandle.close();
	fileInHEADERHandle.close();

	// WWWWWWWWWWWWWWWWWWWWWW

	Sleep(100);

	// fs::copy_file("sandbox/file1.txt", "sandbox/file2.txt");
	// DOES NOT WORK. INSTEAD:


	int jj = 2;
	// copy files following >>
	// https://www.cplusplus.com/reference/ios/ios/rdbuf/
	// <<
	while (jj <= BUBnFiles)
	{
		BUBcopyFROM.open(BUBcurrOutputFileName);
		
		BUBtmpName = fileOutName;
		BUBtmpName.append(std::to_string(jj));
		BUBtmpName.append(".dat");

		BUBcopyTO.open(BUBtmpName);

		BUBcopyTO << BUBcopyFROM.rdbuf();

		jj++;
	}

	fileInCh1Handle = fopen(fileInNameCh1, "rb");
	if (fileInCh1Handle == NULL) {
		cout << "Error re-opening the input file for Ch.1" << std::endl;
		fileInHEADERHandle.close();
		// fclose(NEWOutHandle);
		return 1;
	}

	fileInCh2Handle = fopen(fileInNameCh2, "rb");
	if (fileInCh2Handle == NULL) {
		cout << "Error re-opening the input file for Ch.2" << std::endl;
		fileInHEADERHandle.close();
		// fclose(NEWOutHandle);
		fclose(fileInCh1Handle);
		return 1;
	}

	//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
	
	// LABEL_MODIF_KVV
	// std::size_t freadRESULT;
	// << removed (!)
	// added:
	
	// JJJJJJJJJJJJJJJJ
	// int	 fscanfSTATUS = 0;	// other option: EOF = -1. // most probably.
	std::size_t	 fscanfSTATUS = 0;	// other option: EOF = -1. // most probably.
	// <<<

	bool runFlag = true;
	
	// LABEL_MODIF_KVV
	// uint32_t	CurrTime = 0;	// incremental time for a loop;
	// TODEL
	
	// Combine channels, convert from float(ns) to unsigned(25bit)(ps), add overflow tags.

								
								// HydraHarp T2 mode record structure (in bits): //
	// OLD::: overflow_mark (1 bit)(MSB) || channel_No. (6 bits) || time_tag (25 bits) //
	// NEW::: overflow_mark OR channel_No. (4 bits)(MSB) || time_tag (28 bits) //
	// 0b0000 == 0 == ch1; 0b0001 == 1 == ch2; etc. //
	// 0b1111 == 15 == overflow event; //

	uint32_t	myCh1Mask = ((uint32_t)0) << 28; // number of channel (from 0) shifted by N_bits_TimeTag to the left.
	uint32_t	myCh2Mask = ((uint32_t)1) << 28; // number of channel (from 0) shifted by N_bits_TimeTag to the left.
	uint32_t	myOVFLMask = ((uint32_t)15) << 28; // MSB;	// = 0b1111 + 28 more bit values.

// LLLLLLLLLLL
#ifdef stage3_DEBUG_re_create_files_with_text_tag_time_values
	// create 2 aux. files, open, prepare for output.
	BUBcurrOutputFileName = fileOutName;
	BUBcurrOutputFileName.append("_aux_stage3_ch1.dat");
	std::ofstream fhOutAuxCh1(BUBcurrOutputFileName.c_str());
	if (!fhOutAuxCh1.is_open())
	{
		fileInHEADERHandle.close();
		fclose(fileInCh1Handle);
		fclose(fileInCh2Handle);
		cout << std::endl << "Error opening aux. output file (ch.1)." << std::endl;
		return 1;
	}
	
	BUBcurrOutputFileName = fileOutName;
	BUBcurrOutputFileName.append("_aux_stage3_ch2.dat");
	std::ofstream fhOutAuxCh2(BUBcurrOutputFileName.c_str());
	if (!fhOutAuxCh2.is_open())
	{
		fileInHEADERHandle.close();
		fclose(fileInCh1Handle);
		fclose(fileInCh2Handle);
		fhOutAuxCh1.close();
		cout << std::endl << "Error opening aux. output file (ch.2)." << std::endl;
		return 1;
	}

#endif


	// DEBUG PRINT: >>
	cout << "Defined masks (Ch1 // Ch2 // OverflowBit):" << std::endl;

	cout << std::bitset<32>(myCh1Mask) << std::endl;
	cout << std::bitset<32>(myCh2Mask) << std::endl;
	cout << std::bitset<32>(myOVFLMask) << std::endl;
	// <<

	std::size_t	helpSizeUINT32 = sizeof(uint32_t); // for I/O operations //
	std::size_t	helpSizeDOUBLE = sizeof(double);
	std::size_t	helpSizeUINT64 = sizeof(uint64_t);

	// LABEL EVENING TYPES >>>
	//uint64_t	hlpOVFLtrunc = ((uint64_t)trunc(((double)myOverflowVal) / inAureaPSin1Tag));
	int64_t	hlpOVFLtrunc = ((int64_t)trunc(((double)myOverflowVal) / inAureaPSin1Tag));

	// aux for calculations;
	// convert current excess time into {Tag, Time} pair.

	// <<<<<<  "CONSTANTS", not to iterate in a cycle //


	// LABEL EVENING TYPES >>>
	//uint64_t	ch1tag;		// tags, see <inAureaPSin1Tag>
	int64_t		ch1tag;		// tags, see <inAureaPSin1Tag>
	double		ch1time;	// ns
	//uint64_t	ch2tag;		// tags, see <inAureaPSin1Tag>
	int64_t		ch2tag;		// tags, see <inAureaPSin1Tag>
	double		ch2time;	// ns

	
	uint32_t	convertedCH1timePS; // after subtracting <lastOVFL> and converting to ps //
	uint32_t	convertedCH2timePS;
	// <<< // LABEL EVENING TYPES 


	// 20.12.2021: VKORN >>>
	double		auxTimePS;			//	ps
									// then convertedCH_x_ := (uint32_t)round(auxTime). //
	// <<<<

	//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO

	FILE* NEWOutHandle;		// is redefined in each cycle;
	// OUTPUT

	std::fstream OutOutOuttemp; // use option std::ios_base::binary if necessary
	// For rewriting the header.

	bool bCh1empty;
	bool bCh2empty;

	// Used to cope with overflow events; stores the last {time,tag} for an overflow event;
	// (add <myOverflowVal> (ps) and convert it to time and tag)
	
	// LABEL EVENING TYPES >>>
	//uint64_t	lastOVFLtag = 0;		// tags
	int64_t		lastOVFLtag = 0;		// tags
	double		lastOVFLtime = 0.0;		// ns

	uint64_t CH1left = Ncnts1;
	uint64_t CH2left = Ncnts2;

	// Used to cope with overflow events; stores the last {time,tag} for an overflow event;
	// (add <myOverflowVal> (ps) and convert it to time and tag)
	lastOVFLtag = 0;		// tags
	lastOVFLtime = 0.0;		// ns

	convertedCH1timePS = 0; // after subtracting <lastOVFL> and converting to ps //
	convertedCH2timePS = 0;
	
	// INITIALIZE TAGS AND TIMES: (NOT IN A CYCLE!)
	fread(&ch1tag, helpSizeUINT64, unityForFWRITE, fileInCh1Handle);
	fread(&ch1time, helpSizeDOUBLE, unityForFWRITE, fileInCh1Handle);
	fread(&ch2tag, helpSizeUINT64, unityForFWRITE, fileInCh2Handle);
	fread(&ch2time, helpSizeDOUBLE, unityForFWRITE, fileInCh2Handle);

	CH1left = CH1left - (uint64_t)1;
	CH2left = CH2left - (uint64_t)1;
	// <<<

	
	cout << std::endl << std::endl << "entering the output loop:" << std::endl;
	
	// LABEL EVENING TYPES >>>
	//uint64_t BUBskipTAG;
	int64_t BUBskipTAG;
	// Max tag number. When reached, flush the current file and then move forward.

	//RRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR
	// NEW: INIT CYCLE::

	for (BUBcurrFileN = 1; BUBcurrFileN <= BUBnFiles; BUBcurrFileN++)
	{ // MAIN NEW CYCLE

		BUBcurrOutputFileName = fileOutName;
		BUBcurrOutputFileName.append(std::to_string(BUBcurrFileN));
		BUBcurrOutputFileName.append(".dat");

		NEWOutHandle = fopen(BUBcurrOutputFileName.c_str(), "ab"); // append, binary
		if (NEWOutHandle == NULL) {
			cout << "Error re-opening the output file No." << BUBcurrFileN << " in binary mode" << std::endl;
			fileInHEADERHandle.close();
			fclose(fileInCh1Handle);
			fclose(fileInCh2Handle);
			return 1;
		}

		// LABEL EVENING TYPES >>>
		// BUBskipTAG = (std::min(ch1tag, ch2tag)) + (uint64_t)round(TimeToSplitSEC / (inAureaPSin1Tag * 1.0e-12));
		BUBskipTAG = (std::min(ch1tag, ch2tag)) + (int64_t)round(TimeToSplitSEC / (inAureaPSin1Tag * 1.0e-12));
		// do check it every cycle...

		bCh1empty = false;
		bCh2empty = false;

		BUBtotalItems = 0;

		runFlag = true;

		// #ifdef AureaProcessorPart2OverflowDebuggingMessages
		cout << "First record (i=" << BUBcurrFileN << "): " << std::endl;
		cout << "BUBskipTAG (max tag to process in this file) = " << BUBskipTAG << std::endl;
		cout << "ch1tag: " << ch1tag << " \tch1time: " << ch1time << std::endl;
		cout << "ch2tag: " << ch2tag << " \tch2time: " << ch2time << std::endl;
		// #endif

#ifdef AureaProcessorPart2OverflowDebuggingMessages
		long int i = 0;
#endif

//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO
		
		
#ifdef AureaProcessorPart2OverflowDebuggingMessages
		while ((runFlag) && (i < 10))
#else
		while (runFlag)
#endif
		{
			if ((!bCh2empty) && ((bCh1empty) || (ch1tag > ch2tag) || ((ch1tag == ch2tag) && (ch1time > ch2time))))
				// use CH2 if (a) it is not empty AND (b1) no values are left in CH1 or (b2) if CH1 curr. record time > CH2 curr. record time //
			{
				// write CH2, keep CH1 //
#ifdef AureaProcessorPart2OverflowDebuggingMessages
				cout << "ch2 write" << std::endl;
#endif
				//OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO

				// 20.12.2021: VKORN: >>>
				// convertedCH2timePS = (uint32_t)lround(((ch2tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch2time - lastOVFLtime));
				// convertedCH2timePS = (uint32_t)round(((ch2tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch2time - lastOVFLtime));
				// 1e3: ns -> ps //
				// AVERAGE TO UNITS OF PICOSECONDS //
				
				
				// auxTimePS = ((ch2tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch2time - lastOVFLtime);
				// :: moved below, to the "while [overflow]" loop.
				
				// <<<

				// Overflow check:
				// OLD: // if (convertedCH2timePS > myOverflowVal)
				// NOW SHOULD BE CAPABLE OF ADDING MULTIPLE OVERFLOW EVENTS //
				// WITHOUT RUINING UP THE TIME VALUE //
				

				// while (convertedCH2timePS > (double)myOverflowVal)


				// LABEL_MODIF_KVV
				// 20.12.2021: >>> 
				
				//while (auxTimePS > (double)myOverflowVal)
				//{
				//	// increase OVFL marker, recalculate value
				//	// hlpOVFLtrunc = ((uint64_t)trunc(  ((double)myOverflowVal) / inAureaPSin1Tag  ));
				//	// << defined before <while> loop.

//#ifdef AureaProcessorPart2OverflowDebuggingMessages
//					cout << "old EVENT time: " << convertedCH2timePS << std::endl;
//					cout << "old OVFL tag = " << lastOVFLtag << " \t" << "old OVFL time = " << lastOVFLtime << std::endl;
//#endif
					
				//	lastOVFLtag = lastOVFLtag + hlpOVFLtrunc;
				//	// // OLD: // lastOVFLtime = lastOVFLtime + (1e-3) * (((double)myOverflowVal) - (hlpOVFLtrunc * inAureaPSin1Tag));
				//	lastOVFLtime = lastOVFLtime + (1e-3)*(((double)myOverflowVal) - (((double)hlpOVFLtrunc) * inAureaPSin1Tag));
					
					
					// Nov.20: round to <ps> level:
					// lastOVFLtime = (1.0e-3) * round(lastOVFLtime * (1.0e3) + (((double)myOverflowVal) - (((double)hlpOVFLtrunc) * inAureaPSin1Tag)));
					// 1e-3: ps -> ns;
					// upd: rolled back; NOW: AVERAGED TO UNITS OF PICOSECONDS //

				//	if (lastOVFLtime > (inAureaPSin1Tag * (1.0e-3))) // can occur only once...
				//	{
				//		lastOVFLtag = lastOVFLtag + (uint64_t)1;
				//		
				//		// 20.12.2021: VKORN: >>>
				//		lastOVFLtime = lastOVFLtime - (inAureaPSin1Tag * (1e-3));
				//		// lastOVFLtime = (1.0e-3) * round(lastOVFLtime * (1.0e3) - inAureaPSin1Tag);
				//		// upd: revinded. NOW: AVERAGED TO UNITS OF PICOSECONDS //
				//		// <<<
				//	}

//#ifdef AureaProcessorPart2OverflowDebuggingMessages
//					cout << "new OVFL tag = " << lastOVFLtag << " \t" << "new OVFL time = " << lastOVFLtime << std::endl;
//#endif
					// Write an overflow event:
				//	convertedCH2timePS = ((uint32_t)0) | myOVFLMask;

				//	// VKORN_TUESDAY >>
					// OLD:
					// fileOutHandle.write((const char*)& convertedCH2timePS, helpSizeUINT32); // OVFL marker written;

				//	// NEW: text only ((
					// fileOutHandle << convertedCH2timePS;

					// NEW: binary:
					// VKORN DESPERATE FLAG >>
					// fileOutHandle.write(reinterpret_cast<char*>(&convertedCH2timePS), helpSizeUINT32); // OVFL marker written;
				//	fwrite(&convertedCH2timePS, helpSizeUINT32, unityForFWRITE, NEWOutHandle);
					// << VKORN DESPERATE FLAG
					// << VKORN_TUESDAY

				//	// increase the counter of overflow events;
				//	nOVFLmarkersREAL = nOVFLmarkersREAL + ((uint64_t)1);
				//	BUBtotalItems = BUBtotalItems + ((uint64_t)1);
				//	// decrease time by 1 "overflow value":
				//	// convertedCH2timePS = (uint32_t)lround(((ch2tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch2time - lastOVFLtime));
				//	convertedCH2timePS = (uint32_t)round(((ch2tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch2time - lastOVFLtime));

					// 1e-3: ps -> ns;
//#ifdef AureaProcessorPart2OverflowDebuggingMessages
//					cout << "new EVENT time: " << convertedCH2timePS << std::endl;
//#endif

				// }

				// <<< before 20.12.2021.
				// After: >>>
				auxTimePS = ((ch2tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch2time - lastOVFLtime);

				while (auxTimePS > (double)myOverflowVal)
				{
					
#ifdef stage3_DEBUG_re_create_files_with_text_tag_time_values
	#ifdef stage4_Write_Overflow_Flags_ToFile
	fhOutAuxCh2 << "OVFL_MARKER" << std::endl;
	#endif
#endif

					// Write an overflow event:
					convertedCH2timePS = ((uint32_t)0) | myOVFLMask;
					fwrite(&convertedCH2timePS, helpSizeUINT32, unityForFWRITE, NEWOutHandle);
					
					// increase the counter of overflow events;
					nOVFLmarkersREAL = nOVFLmarkersREAL + ((uint64_t)1);
					BUBtotalItems = BUBtotalItems + ((uint64_t)1);
					
					// decrease time by 1 "overflow value":
					// convertedCH2timePS = (uint32_t)lround(((ch2tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch2time - lastOVFLtime));
					// convertedCH2timePS = (uint32_t)round(((ch2tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch2time - lastOVFLtime));
					// 1e-3: ps -> ns;
					
					// lastOVFLtag = lastOVFLtag + (uint64_t)1;
					// auxTimePS = auxTimePS - inAureaPSin1Tag;
					
					lastOVFLtag = lastOVFLtag + hlpOVFLtrunc;
					lastOVFLtime = lastOVFLtime + (1e-3)*(((double)myOverflowVal) - (((double)hlpOVFLtrunc) * inAureaPSin1Tag));
						// Nov.20: round to <ps> level:
						// lastOVFLtime = (1.0e-3) * round(lastOVFLtime * (1.0e3) + (((double)myOverflowVal) - (((double)hlpOVFLtrunc) * inAureaPSin1Tag)));
						// 1e-3: ps -> ns;
						// upd: rolled back; NOW: AVERAGED TO UNITS OF PICOSECONDS //

					
					while (lastOVFLtime > (inAureaPSin1Tag * (1.0e-3))) // can occur only once...
					// I am putting a 'while' loop, just in case...
					{
						// LABEL EVENING TYPES >>>
						// lastOVFLtag = lastOVFLtag + (uint64_t)1;
						lastOVFLtag = lastOVFLtag + (int64_t)1;
						// 20.12.2021: VKORN: >>>
						lastOVFLtime = lastOVFLtime - (inAureaPSin1Tag * (1e-3));
						// lastOVFLtime = (1.0e-3) * round(lastOVFLtime * (1.0e3) - inAureaPSin1Tag);
						// upd: rolled back. NOW: AVERAGED TO UNITS OF PICOSECONDS //
						// <<<
					}
					
					
					// recalculate time (starting from the last overflow moment):
					auxTimePS = ((ch2tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch2time - lastOVFLtime);

#ifdef AureaProcessorPart2OverflowDebuggingMessages
					cout << "new EVENT time: " << convertedCH2timePS << std::endl;
#endif

				}	// while overshooting : write overflow events..
				
				
				// Apply the channel mask:
#ifdef AureaReverseChannelAndTimeData
				convertedCH2timePS = (convertedCH2timePS << 4) | myCh2Mask; // first Time, then Channel
#else
				// convertedCH2timePS = convertedCH2timePS | myCh2Mask; // first Channel (MSB), then Time
				convertedCH2timePS = ((uint32_t)(round(auxTimePS))) | myCh2Mask; // first Channel (MSB), then Time
#endif

				fwrite(&convertedCH2timePS, helpSizeUINT32, unityForFWRITE, NEWOutHandle);

#ifdef stage3_DEBUG_re_create_files_with_text_tag_time_values
fhOutAuxCh2 << "   " << ch2tag << "   " << ch2time << std::endl;
#endif

				// LABEL_MODIF_KVV
				// ToDo: replace "ch2 left" with fscanfSTATUS == EOF (-1).
				
				fscanfSTATUS = fread(&ch2tag, helpSizeUINT64, unityForFWRITE, fileInCh2Handle);
				// freadRESULT = fread(&ch2time, helpSizeDOUBLE, unityForFWRITE, fileInCh2Handle);
				// if (fscanfSTATUS != EOF)
				if (fscanfSTATUS == 1)
				{
					fscanfSTATUS = fread(&ch2time, helpSizeDOUBLE, unityForFWRITE, fileInCh2Handle);
				}

				// LABEL EVENING >>>

				CH2left = CH2left - (uint64_t)1;
				// << VKORN_TUESDAY

				// if (fileInCh2Handle.good())
				// if ((freadRESULT == helpSizeDOUBLE) && (CH2left != 0))
				
				if (ch2tag > BUBskipTAG)
					{
						runFlag = false;
						// go to flushing the file!
					}
				
				// JJJJJJJJJJJJJJJJ
				// if ((CH2left > 0) && (fscanfSTATUS != EOF))
				// if (fscanfSTATUS != EOF)
				if (fscanfSTATUS == 1)
				{
					// O.K.
#ifdef AureaProcessorPart2OverflowDebuggingMessages
					cout << "ch2tag: " << ch2tag << " \tch2time: " << ch2time << std::endl;
#endif
				}
				else
				{
					bCh2empty = true;
					if (bCh1empty == true)
					{
						runFlag = false;
					}
				}
			}
			else   // CH2 is empty, OR (CH1 not empty and CH1 time is less than in CH2) //
			{
				// write CH1, keep CH2 //
#ifdef AureaProcessorPart2OverflowDebuggingMessages
				cout << "ch1 write" << std::endl;
#endif

				// 20.12.2021: same changes are applied here:
				// >>>>>

				// <<< before 20.12.2021.
				// After: >>>
				auxTimePS = ((ch1tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch1time - lastOVFLtime);

				
				while (auxTimePS > (double)myOverflowVal)
				{
					
#ifdef stage3_DEBUG_re_create_files_with_text_tag_time_values
	#ifdef stage4_Write_Overflow_Flags_ToFile
	fhOutAuxCh1 << "OVFL_MARKER" << std::endl;
	#endif
#endif

					// Write an overflow event:
					convertedCH1timePS = ((uint32_t)0) | myOVFLMask;
					fwrite(&convertedCH1timePS, helpSizeUINT32, unityForFWRITE, NEWOutHandle);
					
					// increase the counter of overflow events;
					nOVFLmarkersREAL = nOVFLmarkersREAL + ((uint64_t)1);
					BUBtotalItems = BUBtotalItems + ((uint64_t)1);
					
					// decrease time by 1 "overflow value":
					// convertedCH2timePS = (uint32_t)lround(((ch2tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch2time - lastOVFLtime));
					// convertedCH2timePS = (uint32_t)round(((ch2tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch2time - lastOVFLtime));
					// 1e-3: ps -> ns;
					
					// lastOVFLtag = lastOVFLtag + (uint64_t)1;
					// auxTimePS = auxTimePS - inAureaPSin1Tag;
					
					lastOVFLtag = lastOVFLtag + hlpOVFLtrunc;
					lastOVFLtime = lastOVFLtime + (1e-3)*(((double)myOverflowVal) - (((double)hlpOVFLtrunc) * inAureaPSin1Tag));
						// Nov.20: round to <ps> level:
						// lastOVFLtime = (1.0e-3) * round(lastOVFLtime * (1.0e3) + (((double)myOverflowVal) - (((double)hlpOVFLtrunc) * inAureaPSin1Tag)));
						// 1e-3: ps -> ns;
						// upd: rolled back; NOW: AVERAGED TO UNITS OF PICOSECONDS //

					while (lastOVFLtime > (inAureaPSin1Tag * (1.0e-3))) // can occur only once...
					// I am putting a 'while' loop, just in case...
					{
						

						// LABEL EVENING TYPES >>>
						// lastOVFLtag = lastOVFLtag + (uint64_t)1;
						lastOVFLtag = lastOVFLtag + (int64_t)1;
						// 20.12.2021: VKORN: >>>
						lastOVFLtime = lastOVFLtime - (inAureaPSin1Tag * (1e-3));
						// lastOVFLtime = (1.0e-3) * round(lastOVFLtime * (1.0e3) - inAureaPSin1Tag);
						// upd: rolled back. NOW: AVERAGED TO UNITS OF PICOSECONDS //
						// <<<
					}

					
					
					// recalculate time (starting from the last overflow moment):
					auxTimePS = ((ch1tag - lastOVFLtag) * inAureaPSin1Tag) + (1.0e3) * (ch1time - lastOVFLtime);

#ifdef AureaProcessorPart2OverflowDebuggingMessages
					cout << "new EVENT time: " << convertedCH2timePS << std::endl;
#endif

				}	// while overshooting : write overflow events..

				// Just apply the channel mask:
#ifdef AureaReverseChannelAndTimeData
				convertedCH1timePS = (convertedCH1timePS << 4) | myCh1Mask; // first Time (MSB), then channel No.
#else
				// convertedCH1timePS = convertedCH1timePS | myCh1Mask; // first Channel (MSB), then Time
				 convertedCH1timePS = ((uint32_t)(round(auxTimePS))) | myCh1Mask; // first Channel (MSB), then Time
#endif

			// fileOutHandle << convertedCH1timePS;
			// VKORN_TUESDAY >>
			// OLD:
			// fileOutHandle.write((const char*)&convertedCH1timePS, helpSizeUINT32);

			// NEW: text only ((
			// fileOutHandle << convertedCH1timePS;

			// NEW: binary
			// VKORN DESPERATE FLAG >>
			// fileOutHandle.write(reinterpret_cast<char*>(&convertedCH1timePS), helpSizeUINT32);
			// << VKORN DESPERATE FLAG
				fwrite(&convertedCH1timePS, helpSizeUINT32, unityForFWRITE, NEWOutHandle);

#ifdef stage3_DEBUG_re_create_files_with_text_tag_time_values
fhOutAuxCh1 << "   " << ch1tag << "   " << ch1time << std::endl;
#endif

				// << VKORN_TUESDAY

				// Get new value from file for CH1:  (mark file end if needed:)
				// if (fileInCh1Handle >> ch1tag >> ch1time)

				// VKORN_TUESDAY >>
				// fileInCh1Handle.read((char*)& ch1tag, helpSizeUINT64);
				// fileInCh1Handle.read((char*)& ch1time, helpSizeDOUBLE);

				// fileInCh1Handle >> ch1tag;
				// fileInCh1Handle >> ch1time;

				// VKORN DESPERATE FLAG >>
				// fileInCh1Handle.read(reinterpret_cast<char*>(&ch1tag), helpSizeUINT64);
				// fileInCh1Handle.read(reinterpret_cast<char*>(&ch1time), helpSizeDOUBLE);

				fscanfSTATUS = fread(&ch1tag, helpSizeUINT64, unityForFWRITE, fileInCh1Handle);
				
				// LABEL_MODIF_KVV
				// freadRESULT = fread(&ch1time, helpSizeDOUBLE, unityForFWRITE, fileInCh1Handle);
				if (fscanfSTATUS == 1)
				{
					fscanfSTATUS = fread(&ch1time, helpSizeDOUBLE, unityForFWRITE, fileInCh1Handle);
				}
				// <<<
				// ToDo: replace "freadRESULT" with "fscanfSTATUS == 0 or EoF (-1)."

				CH1left = CH1left - (uint64_t)1;

				// << VKORN_TUESDAY
				// if (fileInCh1Handle.good())
				
				// LABEL EVENING >>>
				
				if (ch1tag > BUBskipTAG)
					{
						runFlag = false;
						// go to flushing the file!
					}
				
				// if ((freadRESULT != helpSizeDOUBLE) && (CH1left != 0))
				
				// JJJJJJJJJJJJJJJJ
				// if ((CH1left > 0) && (fscanfSTATUS != EOF))
				if (fscanfSTATUS == 1)
				// if (fscanfSTATUS != EOF)
					// << VKORN DESPERATE FLAG
				{
					// O.K.
#ifdef AureaProcessorPart2OverflowDebuggingMessages
					cout << "ch1tag: " << ch1tag << " \tch1time: " << ch1time << std::endl;
#endif
				}
				else
				{
					bCh1empty = true;
					if (bCh2empty == true)
					{
						runFlag = false;
					}
				}

			} // << CH1 <<

#ifdef AureaProcessorPart2OverflowDebuggingMessages
			i++;
#endif

		} // while (runFlag);



		*OUTnOVFLmarkersREAL = *OUTnOVFLmarkersREAL + nOVFLmarkersREAL;
		cout << "Overflows: EST = " << nOVFLmarkersESTIMATE << " \tREAL = " << nOVFLmarkersREAL << std::endl;


		// VKORN DESPERATE FLAG >>
		// fileOutHandle.flush();
		// fileOutHandle.close();
		fflush(NEWOutHandle);
		fclose(NEWOutHandle);
		// << VKORN DESPERATE FLAG

		// fileInHEADERHandle.close();
		// Moved significantly up!

		// RRRRRRRRRRRRRRRRRRR
		// REWRITE THE HEADER WITH THE GIVEN MEAS_TIME AND N_EVENTS >>>>
		// >>>>>>>>>>>>>>>>>>>

		Sleep(500);

		OutOutOuttemp.open(BUBcurrOutputFileName.c_str());
		// std::ios_base::out | std::ios_base::in
		OutOutOuttemp.seekp(0, std::ios_base::beg);

		if (OutOutOuttemp.fail())
		{
			cout << std::endl << "Error opening output file." << std::endl;
			return 1;
		}

		OutOutOuttemp.write((const char*)TagPREHEADER16bytes.c_str(), (TagPREHEADER16bytes.length()) * sizeof(char));

		// we want: rtPicoHarpT2     = 0x00010203,  # (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T2), HW: $03 (PicoHarp)
		OutOutOuttemp.write((const char*)TagRECTYPE.c_str(), AUREABytesInATagName * sizeof(char));
		OutOutOuttemp.write((const char*)& outputIdx, sizeof(int32_t));
		outputType = 0x10000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
		// tyAnsiString  = 0x4001FFFF, tyBinaryBlob  = 0xFFFFFFFF
		OutOutOuttemp.write((const char*)& outputType, sizeof(uint32_t));
		outputValueUINT64 = 0x00010203; // PicoHarp, T2
		// rtHydraHarpT2 = 0x00010204, # (SubID = $00, RecFmt: $01) (V1), T - Mode: $02(T2), HW : $04(HydraHarp)
		OutOutOuttemp.write((const char*)& outputValueUINT64, sizeof(uint64_t));

		OutOutOuttemp.write((const char*)TagNUMREC.c_str(), AUREABytesInATagName * sizeof(char));
		OutOutOuttemp.write((const char*)& outputIdx, sizeof(int32_t));
		outputType = 0x10000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
		OutOutOuttemp.write((const char*)& outputType, sizeof(uint32_t));

		// HERE >>
		// ui64TempOutput = (Ncnts1 + Ncnts2) + nOVFLmarkersESTIMATE;
		// ui64TempOutput = (Ncnts1 + Ncnts2) + nOVFLmarkersREAL; // 'cause we know that now.
		ui64TempOutput = BUBtotalItems;
		// <<<

		OutOutOuttemp.write((const char*)& ui64TempOutput, sizeof(uint64_t));

		OutOutOuttemp.write((const char*)TagGLOBRES.c_str(), AUREABytesInATagName * sizeof(char));
		OutOutOuttemp.write((const char*)& outputIdx, sizeof(int32_t));
		outputType = 0x20000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
		OutOutOuttemp.write((const char*)& outputType, sizeof(uint32_t));
		outputValueDOUBLE = ((double)AUREAGlobalResolution);	// in seconds; DEFAULT!
		OutOutOuttemp.write((const char*)& outputValueDOUBLE, sizeof(double));

		OutOutOuttemp.write((const char*)TagACQTIME.c_str(), AUREABytesInATagName * sizeof(char));
		OutOutOuttemp.write((const char*)& outputIdx, sizeof(int32_t));
		outputType = 0x10000008; // uint64_t ??? picoseconds??? //
		OutOutOuttemp.write((const char*)& outputType, sizeof(uint32_t));

		// VKORN_TUESDAY >>	
		// outputValueUINT64 = (uint64_t)inMeasTime;

		// outputValueUINT64 = (uint64_t)(trunc(inMeasTime));

		// STAGE3 (SPLITTER) >>>>>
		outputValueUINT64 = (uint64_t)(trunc(TimeToSplitSEC*1e3)); // in milliseconds!
		// STAGE3 (SPLITTER) <<<<<

		OutOutOuttemp.write((const char*)& outputValueUINT64, sizeof(uint64_t));
		// << VKORN_TUESDAY

		OutOutOuttemp.write((const char*)TagHEADEREND.c_str(), AUREABytesInATagName * sizeof(char));
		OutOutOuttemp.write((const char*)& outputIdx, sizeof(int32_t));
		outputType = 0x10000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008. // here -- any.
		OutOutOuttemp.write((const char*)& outputType, sizeof(uint32_t));
		outputValueUINT64 = 0;
		OutOutOuttemp.write((const char*)& outputValueUINT64, sizeof(outputValueUINT64));
		// << Ver2 //

		OutOutOuttemp.flush();
		Sleep(500);
		OutOutOuttemp.close();

	} // MAIN NEW CYCLE    (For <file> in <files> ...)
	
	// HERE GOES THE END OF THE FOR LOOP //
	
	// <<<<<<<<<<<<<<<<<<<<<<<<<
	// <<<< REWRITE THE HEADER

	
	cout << "closing the files" << std::endl;

	fclose(fileInCh1Handle);
	fclose(fileInCh2Handle);

// LLLLLLLLLLL
#ifdef stage3_DEBUG_re_create_files_with_text_tag_time_values
	// closing files with directly back-converted time and tag values.
	fhOutAuxCh1.flush();
	fhOutAuxCh2.flush();
	
	fhOutAuxCh1.close();
	fhOutAuxCh2.close();
#endif

	return 0;
}









//	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	//
//	*				Reverse converter (for checking the consistency...)			*	//
//	*																			*	//
//	*									[ Stage 4. ]							*	//
//	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	//


int ConvertPTUtoAUREA(const char* fileInNamePTU, const char* fileOutCh1, const char* fileOutCh2

	// 20.12.: header processing will be added later.
	// , const char* fileOutNameCh2
	// , const char* fileOutNameHEADER

)

{
	// I/O help "constants":
	std::size_t	helpSizeUINT32 = sizeof(uint32_t);
	std::size_t	helpSizeDOUBLE = sizeof(double);
	std::size_t	helpSizeUINT64 = sizeof(uint64_t);
	std::size_t unityForFWRITE = 1;

	// channel masks:
#ifdef AureaReverseChannelAndTimeData
	uint32_t	myCh1Mask = ((uint32_t)0); // number of channel (from 0) NOT shifted by N_bits_TimeTag to the left.
	uint32_t	myCh2Mask = ((uint32_t)1); // number of channel (from 0) NOT shifted by N_bits_TimeTag to the left.
	uint32_t	myOVFLMask = ((uint32_t)15);  // LSB;	// = 28 times 0b0, then 0b1111.
#else
	uint32_t	myCh1Mask = ((uint32_t)0) << 28; // number of channel (from 0) shifted by N_bits_TimeTag to the left.
	uint32_t	myCh2Mask = ((uint32_t)1) << 28; // number of channel (from 0) shifted by N_bits_TimeTag to the left.
	uint32_t	myOVFLMask = ((uint32_t)15) << 28; // MSB;	// = 0b1111 + 28 more bit values.
#endif

	// prepare files:
	std::ofstream fileOutCh1AUREA(fileOutCh1);
	if (!fileOutCh1AUREA.is_open())
		{
			cout << "Error opening file for output (Ch.1)." << std::endl;
			return 1;
		}
	
	std::ofstream fileOutCh2AUREA(fileOutCh2);
	if (!fileOutCh2AUREA.is_open())
		{
			cout << "Error opening file for output (Ch.2)." << std::endl;
			fileOutCh1AUREA.close();
			return 1;
		}

	FILE* fileInHandle;
	fileInHandle = fopen(fileInNamePTU, "rb");
	if (fileInHandle == NULL) {
		fileOutCh1AUREA.close();
		fileOutCh2AUREA.close();
		cout << "Error opening input file." << std::endl;
		return 1;
	}

	fseek(fileInHandle, 256, SEEK_SET);
	// ToDo: replace with finding the header end... ;

	bool runFlag = true;
	
	// JJJJJJJJJJJJJJJJ
	// int	 fscanfSTATUS = 0;	// other option: EOF = -1. // most probably.
	std::size_t	 fscanfSTATUS = 0;	// other option: EOF = -1. // most probably.

	std::stringstream ossTMP;
	std::string line = "initial_value";
	ossTMP.str(line);

	uint64_t currTAG = 0;		// reading from file, converting to tags and times
	double currTIMEps = 0.0;	// ps
	
	uint32_t PTUrecord = 0;	// will be used for extracting the data from PTU-file.

	const uint64_t T2WRAPAROUND = 210698240;	// ToDo: change to 2^28 here and in pre-processor (Aurea).
	
	int PTUchannel = 0;		
	uint32_t PTUtimePS = 0;	// number of picoseconds, as read from the record.
	
	double PTUcurrTimeExcessPS = 0.0;	// increased when overflow marker in PTU file,
										// decreased when (% + time) > myPSin1tag.
										// Can be negative.

	
	double myPSin1tag;
	myPSin1tag = (1.0 / (double)625000) * 1e+12; // ps
	// ToDo:
	// manually programmed to 625 kHz
	// replace with input from main program (later)


	fscanfSTATUS = fread(&PTUrecord, helpSizeUINT32, unityForFWRITE, fileInHandle);
	// JJJJJJJJJJJJJJJJ
	// if (fscanfSTATUS == EOF)
	if (fscanfSTATUS != 1)
		{
			runFlag = false;
		}

	while (runFlag)
	{
		
		// apply channel and overflow marker masks; process accordingly
		if ((PTUrecord >> 28) == (uint32_t)15) //this means we have a special record
		{
			// overflow event
			
			// markers not implemented //
			
			// PTUtimePS = 0;	>> not needed now...
			// PTUchannel = -1;	>> also not needed...
			PTUcurrTimeExcessPS = PTUcurrTimeExcessPS + (double)T2WRAPAROUND;
			// unwrap the time tag overflow	

#ifdef stage4_Write_Overflow_Flags_ToFile
				ossTMP.str("OVFL_MARKER\n");
				fileOutCh1AUREA << ossTMP.str();
				fileOutCh2AUREA << ossTMP.str();
#endif

		}
		else
		{
			// regular event - read, convert time, update time base marker, write to AureaFile
			
			// PTUtimePS = *oflcorrection + (record & (~(((uint32_t)15) << 28)));
			PTUtimePS = PTUrecord & (~(((uint32_t)15) << 28));
			PTUchannel = (int)(PTUrecord >> 28);	

			// convert input data to TAGS and TIMES:
			// each time we increment a tag (+=1), we decrement <PTUcurrTimeExcessPS> (-= myPSin1tag)
			currTIMEps = ((double)PTUtimePS) + PTUcurrTimeExcessPS;
			while (currTIMEps > myPSin1tag)
			{
				currTIMEps = currTIMEps - myPSin1tag;
				PTUcurrTimeExcessPS = PTUcurrTimeExcessPS - myPSin1tag;
				currTAG = currTAG + (uint64_t)1;
			}

			// output to file:
			// sprintf(line.c_str(), "\t%d\t%f\n", currTAG, currTIME);
			ossTMP.str("");
			currTIMEps = currTIMEps / 1000.;
			ossTMP << currTAG << "\t" << currTIMEps << std::endl;
			
			if (PTUchannel == 0)
			{
				// Ch.1
				fileOutCh1AUREA << ossTMP.str();
			}
			else
			{
				// Ch.2
				fileOutCh2AUREA << ossTMP.str();
			}

		}	// << regular event


		// fetch next record:

		fscanfSTATUS = fread(&PTUrecord, helpSizeUINT32, unityForFWRITE, fileInHandle);
		
		// cout << fscanfSTATUS << " ";

		//if (!fileInHandle.good())
		// if (fileInHandle.eof())
		// if (fscanfSTATUS == EOF)

		// JJJJJJJJJJJJJJJJ
		// if (!fscanfSTATUS)
		if (fscanfSTATUS != 1)
		{
			runFlag = false;
		}
				
	} // while (runFlag);

	fclose(fileInHandle);
	fileOutCh1AUREA.flush();
	fileOutCh2AUREA.flush();
	fileOutCh1AUREA.close();
	fileOutCh2AUREA.close();

	return 0;
}

// .EOF