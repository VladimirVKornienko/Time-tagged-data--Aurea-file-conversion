/*
*		Aurea measurement files' pre-processing tool.
*
*   From "header,data" create a "new_header,HH_format_data" file and store.
*
*/


//	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	//
//		Stage1 (+) splits the file in three + extracts PSin1Tag, MeasTime, Nch1, Nch2 from the file.		//
//		Stage2 (obsolete) gets some of those parameters as inputs, and makes a *.PTU file in reqired format.		//
//		Stage3 (+) is same to Stage2, but splits the initial file in chunks of			//
//																		<TimeToSplitSEC> time chunks each.	//
//	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	//





// VKORN_TUESDAY >>
// <<< Label for the recent changes in file I/O: from "write" to "<<".

/*
Moved to constants.cpp and header file.

#define AUREABytesInATagName	32	// for formatting the header file.
#define AUREAGlobalResolution	1.0e-12 // in seconds; universal, everywhere here...
*/

// Nov.20: for "Stage2", added rounding to the nearest ps value: value = round( value * 1000.0 ) / 1000.0;

// [<MeasurementTime>] == milliseconds ??!? //


/*
06.01.: moved to the header file:


#include <stdio.h>
#include <stdlib.h>
#include <cstdint>
#include <iostream>

#include <fstream>

#include <sstream>
#include <string>

#include <bitset>		// for binary representations >> debug only, most likely.

#include <iomanip>		// for outputting the floast to required precision with 'cout' etc.

#include <math.h>       // lround

#include <algorithm>		// std::max

*/

#include "KVV_Header.h"

using namespace std;


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
	double myUpperDiscrLevelFraction = 2.50;	// a fraction of the (1/freq), will be calculated later
												// all delays above that value will be treated as erroneous records.
	// previous value: 1.00. Due to rounding errors, some points were missing, maybe.


	double myLowerDiscrLevel = 1.0e-6;			// entered manually
	//   <<<   DEFINE THESE !!  <<<   //

	uint64_t ch1Nskipped = 0;
	uint64_t ch2Nskipped = 0;		// bad records not passing the discriminator
									// see also <myUpperDiscrLevel> and <myLowerDiscrLevel>

	uint64_t NtotCh1 = 0;
	uint64_t NtotCh2 = 0;
	
	double myUpperDiscrLevel = 0; // ns (!) // Will mainly be used for checking the wrong values of Times... //
	double myAureaFreq = 0; // to be more flexible... though unsigned long should be enough.

	std::size_t myFindResult; // for replacing "123e+07" with "123e+7".
	

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
					cout << "freq == " << myAureaFreq << " \t [ 1 Tag == " << ((1.0 / myAureaFreq) * 1e+9) << " ns]" << std::endl;
					cout << "discr level (Max. Time, ns) == " << myUpperDiscrLevel << std::endl;
					cout << "Overflow value int the result file (ns): " << std::endl;
					cout << ( ((double)myOverflowVal)/1.0e+3) << " ns == " <<
					 (  (((double)myOverflowVal)/1.0e+3) / ((1.0 / myAureaFreq) * 1e+9) ) <<
					 " Tags" << std::endl;
#ifdef use_log_file_output
logFile << "freq == " << myAureaFreq << " \t [ 1 Tag == " << ((1.0 / myAureaFreq) * 1e+9) << " ns]\n";
logFile << "discr level (Max. Time, ns) == " << myUpperDiscrLevel << "\n";
logFile << "Overflow value int the result file (ns):\n";
logFile << ( ((double)myOverflowVal)/1.0e+3) << " ns == " <<
 (  (((double)myOverflowVal)/1.0e+3) / ((1.0 / myAureaFreq) * 1e+9) ) <<
 " Tags\n";
#endif

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
		
		
		// 02.01.2022: replace "123e+07" with "123e+7", and "123e-07" with "123e-7"
		// std::size_t myFindResult;
		myFindResult = line.find("e+0",0);
		if (myFindResult != string::npos)
    	{
        	line = line.replace(myFindResult, 3, "e+");
			// Length is 3, which skips the last symbol, i.e. '0'. So "e+07" -> "e+7".
    	}
		
		myFindResult = line.find("e-0",0);
		if (myFindResult != string::npos)
    	{
        	line = line.replace(myFindResult, 3, "e-");
			// Length is 3, which skips the last symbol, i.e. '0'. So "e+07" -> "e+7".
    	}
    
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
#ifdef use_log_file_output
logFile << "ch2 selected, file 1 WILL BE closed (LATER!).\n";
#endif

		
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
#ifdef use_log_file_output
logFile << "unknown record encountered at stage 3, aborting...\n";
#endif
				runFlag = false;
			}
		}
	}
	else
	{	// else, select channel 1 as output;
		// fileOut2.close();
		cout << "ch1 selected, file 2 WILL BE closed (LATER!)." << std::endl;
#ifdef use_log_file_output
logFile << "ch1 selected, file 2 WILL BE closed (LATER!).\n";
#endif

		
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
#ifdef use_log_file_output
logFile << "unknown record encountered at stage 3, aborting...\n";
#endif
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