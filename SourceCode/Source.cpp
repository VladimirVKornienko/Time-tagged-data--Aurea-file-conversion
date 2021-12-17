#include <stdio.h>
#include <stdlib.h>

#include <cstdint>
// #include <limits.h>
// #include <stdbool.h>

// #define _CRT_SECURE_NO_WARNINGS

#include <iostream>
using namespace std;
// for COUT

#include <bitset>
// for binary representation
// e.g.: std::cout << "a = " << std::bitset<8>(a)  << std::endl;


#include "KVV_Header.h"
// for other functions, defined in a project, but in other files.

#include <sstream>
#include <string>

#include <fstream>

#include <cstdint>

#include <iomanip>		// for outputting the float to required precision with 'cout' etc.
// cout << std::fixed << std::setprecision(2);
// revert:
// file1.unsetf(ios_base::fixed);
// file1 << std::setprecision(16) << thePoint[2];

// #include <windows.h>
// for Sleep(milliseconds); //

#include <vector>
// for std::vector<std::string> args(argv, argv + argc);


// ARGUMENT ORDER: <Source_file_name>, <Output_file_name>, <Path_to_temp_folder>. //

// For running from IDE, use "else" statement for manually enetring the filenames.

// int main(void)
int main(int argc, char ** argv) 
{
	std::vector<std::string> args(argv, argv + argc);
	// VER1 :
	// for (size_t i = 1; i < args.size(); ++i)
	// {
	//		std::string const& aFileName = args[i];
			// Do something with aFileName
	//}

	// const char* fname01in = "";
	// const char* fname02out1 = "";
	// const char* fname03out2 = "";
	// const char* fname03out3header = "";
	// const char* fname04result = "";

	std::string fname01in = "";
	std::string fname02out1 = "";
	std::string fname03out2 = "";
	std::string fname03out3header = "";
	std::string fname04result = "";

	// std::string const& aFileName;

	if (args.size() == 3+1) // 1 argument is there by default...
	{
		cout << "Using file names from command line arguments:" << std::endl;
		
		fname01in = args[1];
		fname04result = args[2];
		fname02out1 = args[3];
		fname02out1.append("\\demo_out_ch1.txt");
		fname03out2 = args[3];
		fname03out2.append("\\demo_out_ch2.txt");
		fname03out3header = args[3];
		fname03out3header.append("\\demo_out_header.txt");
	}
	else
	{
		cout << "Using default file names:" << std::endl;
		
		// Default filenames: //
		fname01in = "Z:\\Documents\\ForPiton\\VCC_input_data\\demo_in.txt";
		fname02out1 = "Z:\\Documents\\ForPiton\\VCC_input_data\\demo_out_ch1.txt";
		fname03out2 = "Z:\\Documents\\ForPiton\\VCC_input_data\\demo_out_ch2.txt";
		fname03out3header = "Z:\\Documents\\ForPiton\\VCC_input_data\\demo_out_header.txt";
		fname04result = "Z:\\Documents\\ForPiton\\VCC_input_data\\demo_RESULT.dat";
		// <<<< //

		// ENTER THE FILENAMES HERE when running from IDE: //
		// (uncomment to use)(at least, uncomment <fname01in> and <fname04result>) //
		
		// fname01in = "Z:\\Documents\\ForPiton\\VCC_input_data\\redone_v43\\h18TargONredoneSecond2dB29min30TimeAndTag.txt";
		// fname04result = "Z:\\Documents\\ForPiton\\VCC_input_data\\redone_v43\\h18_02dB_on_CALC.dat";
		
		fname01in = "Z:\\Documents\\ForPiton\\VCC_input_data\\demo_in_REAL_(LARGE).txt";
		// fname01in = "Z:\\Documents\\ForPiton\\VCC_input_data\\demo_in_REAL_(SMALL).txt";
		
		fname04result = "Z:\\Documents\\ForPiton\\VCC_input_data\\redone_v43\\TEST_LARGE_vech";

		// // fname02out1 = "Z:\\Documents\\ForPiton\\VCC_input_data\\demo_out_ch1.txt";
		// // fname03out2 = "Z:\\Documents\\ForPiton\\VCC_input_data\\demo_out_ch2.txt";
		// // fname03out3header = "Z:\\Documents\\ForPiton\\VCC_input_data\\demo_out_header.txt";
		// <<< //
	}

	cout << "in: " << fname01in << std::endl;
	cout << "ch1: " << fname02out1 << std::endl;
	cout << "ch2: " << fname03out2 << std::endl;
	cout << "header: " << fname03out3header << std::endl;
	cout << "result file (output): " << fname04result << std::endl << std::endl;

	uint64_t a = 1;
	int b = 0;

	double dPSin1Tag = 0.;

	
	// see below: bit representation (1); bit masks (2); //
	// file I/O ver1 basic (3); file I/O: input and output file streams (4); //
	// parsing strings, string streams, and char arrays (5); //
	// reading from binary file (6) - tested.; //
	// writing to 'cout' (maybe, other streams also?) with given precision (7). //
	// defining a 4-bit structure (8). //
	// NUMBER to ITS BYTE (not BIT - it is done with std::bitset<64>(<obj>)) REPRESENTATION (9). //

	cout << std::endl << "* * *     Aurea pre-formatter testing     * * *" << std::endl;
	
	
	// DEBUGGING: >>> //
	/*
	const char* fname04result = "Z:\\Documents\\VKorn_NewFiles\\05 QuantIllum\\05 python HISTO processing\\04 newTry Nov\\data_samples_VKorn_AndBinary\\c_Interferometer\\demo_RESULT.dat";
	
	const char* fname01in = "Z:\\Documents\\VKorn_NewFiles\\05 QuantIllum\\05 python HISTO processing\\04 newTry Nov\\data_samples_VKorn_AndBinary\\c_Interferometer\\demo_in.txt";
	const char* fname02out1 = "Z:\\Documents\\VKorn_NewFiles\\05 QuantIllum\\05 python HISTO processing\\04 newTry Nov\\data_samples_VKorn_AndBinary\\c_Interferometer\\demo_out1.txt";
	const char* fname03out2 = "Z:\\Documents\\VKorn_NewFiles\\05 QuantIllum\\05 python HISTO processing\\04 newTry Nov\\data_samples_VKorn_AndBinary\\c_Interferometer\\demo_out2.txt";
	const char* fname03out3header = "Z:\\Documents\\VKorn_NewFiles\\05 QuantIllum\\05 python HISTO processing\\04 newTry Nov\\data_samples_VKorn_AndBinary\\c_Interferometer\\demo_out3header.txt";
	*/
	// <<< //
	

	
	// const char* fname01in = "Z:\\Documents\\ForPiton\\VCC_input_data\\b_redone5dBON29m30.txt";
	// const char* fname01in = "Z:\\Documents\\ForPiton\\VCC_input_data\\demo_in_REAL_(LARGE).txt";
	// const char* fname02out1 = "Z:\\Documents\\ForPiton\\VCC_input_data\\tmp_out_ch1.txt";
	// const char* fname03out2 = "Z:\\Documents\\ForPiton\\VCC_input_data\\tmp_out_ch2.txt";
	// const char* fname03out3header = "Z:\\Documents\\ForPiton\\VCC_input_data\\tmp_out_header.txt";
	// const char* fname04result = "Z:\\Documents\\ForPiton\\VCC_input_data\\TUE_b_05dB_on_p1_rdn.dat";
	


	/*
	// TESTING THE OUTPUT:
	// TESTING THE READ FROM BINARY FILE ROUTINE //
	double c = 0;

	std::ifstream fileTMP(fname02out1, ios::out | ios::binary);
	if (!fileTMP.is_open()) { cout << "Error opening the file!"; return 1; }
	
	for (b = 0; b < 5; b++)
	{
		fileTMP.read((char*)& a, sizeof(uint64_t));
		fileTMP.read((char*)& c, sizeof(double));
		cout << std::endl << a << "\t" << c << std::endl;
	}

	

	fileTMP.close();

	return 0;
	// << //
	*/
	

	// VKORN_TUESDAY >>
	// uint64_t MeasTime = 0;	// milliseconds ??!?
	double MeasTime = 0.0;	// milliseconds, but floating-point value
	// << VKORN_TUESDAY

	uint64_t NcntsCh1 = 0;
	uint64_t NcntsCh2 = 0;
	uint64_t MAINskippedCh1 = 0;
	uint64_t MAINskippedCh2 = 0;


	uint64_t MAINnOVFLmarkersESTIMATE = 0;
	uint64_t MAINnOVFLmarkersREAL = 0;

	cout << std::fixed << std::setprecision(2);

	cout << std::endl << "Stage 1. Splitting the file in three (header, ch1, ch2):" << std::endl << std::endl;

	b = PreProcessAureaDataStage1(fname01in.c_str(), fname02out1.c_str(), fname03out2.c_str(),
		fname03out3header.c_str(),
		&dPSin1Tag, &MeasTime, &NcntsCh1, &NcntsCh2, &MAINskippedCh1, &MAINskippedCh2);

	cout << "b == " << b << std::endl;
	cout << "dPSin1Tag == " << dPSin1Tag << " \t" << "MeasTime (ms) = " << MeasTime << std::endl;
	
	cout << std::endl << "Counts in Ch.1: " << NcntsCh1 << " ; \tCounts in Ch.2: " << NcntsCh2 << std::endl;
	cout << "Total counts (N_ch1 + N_ch2): " << (NcntsCh1 + NcntsCh2) << std::endl;
	cout << std::endl << "Total counts skipped (zero or too large):" << std::endl;
	cout << "Ch.1: " << MAINskippedCh1 << " \t Ch.2: " << MAINskippedCh2 << std::endl;

	cout << std::endl << std::endl << "[REDUNDANT!] waiting 1 second for the files to sync..." << std::endl << std::endl;
	Sleep(1000);

	cout << std::endl << "Starting a new part - combining 3 files into one (HH T2 format)(Stage 2):" << std::endl << std::endl;
	cout << "myOverflowVal == " << myOverflowVal << std::endl;

	// VKORN DESPERATE FLAG >>
	//b = PreProcessAureaDataStage2(fname04result.c_str(), fname02out1.c_str(),
	//	fname03out2.c_str(), fname03out3header.c_str(), dPSin1Tag,
	//	MeasTime, (NcntsCh1+NcntsCh2+MAINskippedCh1+MAINskippedCh2), &MAINnOVFLmarkersESTIMATE, &MAINnOVFLmarkersREAL);

	/*
	b = PreProcessAureaDataStage2(fname04result.c_str(), fname02out1.c_str(),
		fname03out2.c_str(), fname03out3header.c_str(), dPSin1Tag,
		MeasTime, NcntsCh1, NcntsCh2, &MAINnOVFLmarkersESTIMATE, &MAINnOVFLmarkersREAL);
	*/

	
	double SECONDStoSPLIT = 60.0; // seconds

	b = PreProcessAureaDataStage3splitter(SECONDStoSPLIT, fname04result.c_str(), fname02out1.c_str(),
		fname03out2.c_str(), fname03out3header.c_str(), dPSin1Tag,
		MeasTime, NcntsCh1, NcntsCh2, &MAINnOVFLmarkersESTIMATE, &MAINnOVFLmarkersREAL);
	

	// << VKORN DESPERATE FLAG

	cout << "b == " << b << std::endl;

	return 0;
}

// Some ready-made code sections go here: //


// .1. Bit representation of integer numbers: //

/*
uint64_t a = 1;
//printf("%d\n", a);
// printf("%" PRIu64 "\n", a);
// printf("%lld\n", a);
cout << a << std::endl;
cout << "or" << std::endl;
cout << std::bitset<64>(a) << std::endl;

a = a >> 1;
// printf("%lld\n", a);
cout << a << std::endl;

a = 0 - 1;
// printf("%lld\n", a);
cout << a << std::endl;

a = 0xFFFFFFFFFFFFFFFF;
// printf("%lld\n", a);
cout << a << std::endl;

a = UINT64_MAX;
// printf("%lld\n", a);
cout << a << std::endl;

a = a >> 63;
cout << a << std::endl;
cout << "Or, in binary representation,\n";
cout << std::bitset<64>(a) << std::endl;

a = 1;
a = a << 3;
cout << a << std::endl;
cout << "Or, in binary representation,\n";
cout << std::bitset<64>(a) << std::endl;
*/


// .2. Bit masks: //
/*
uint64_t a = 1;
a = ((uint64_t)1) << (64 - 1); // MSB, "1" only on the left-most position.
cout << a << std::endl;
cout << "Or, in binary representation,\n";
cout << std::bitset<64>(a) << std::endl;

a = (((uint64_t)1) << (3)) - ((uint64_t)1); // desired number (now: 3) of ones at the right side...;
cout << a << std::endl;
cout << "Or, in binary representation,\n";
cout << std::bitset<64>(a) << std::endl;

a = (((uint64_t)1) << (25)) - ((uint64_t)1); // desired number (now: 25) of ones at the right side...;
cout << a << std::endl;
cout << "Or, in binary representation,\n";
cout << std::bitset<64>(a) << std::endl;

const uint64_t spMASK = ((uint64_t)1) << (64 - 1); // MSB
const uint64_t tmMASK = (((uint64_t)1) << (25)) - ((uint64_t)1); // 25 bits at the LSB side
const uint64_t chMASK = ~(spMASK | tmMASK); // all the middle

a = chMASK;
cout << a << std::endl;
cout << "Or, in binary representation,\n";
cout << std::bitset<64>(a) << std::endl;
*/


// .3. File I/O - ver.1 (not very useful; just for consistency)(better use ifstream and ofstream) //

/*
	// Prepare the file
	// const char * myFName = "Z:/Documents/VKorn_NewFiles/05 QuantIllum/05 python HISTO processing/04 newTry Nov/data_samples_VKorn_AndBinary/c_Interferometer/h05_TT_test_TagAndTime_TargON_5dB_2min0.txt";
	const char* myFName = "Z:\\Documents\\VKorn_NewFiles\\05 QuantIllum\\05 python HISTO processing\\04 newTry Nov\\data_samples_VKorn_AndBinary\\c_Interferometer\\h05_TT_test_TagAndTime_TargON_5dB_2min0.txt";
	char chHelp[100];

	FILE *filehandle = fopen(myFName, "rb");

	fscanf(filehandle, "%s", chHelp);
	printf("%s", chHelp);

	fclose(filehandle);

// ANOTHER IMPLEMENTATION: >>>

// FILE* fileOutHandle;

	// fileInHandle = fopen(fileInName, "r");
	// if (fileInHandle == NULL)
	// {
	//		printf("Error opening file (read)!");
	// 		return 1;
	// }
	// else
	// {
	// 	printf("File opened for reading.");
	// }

	// fileOutHandle = fopen(fileOutName, "w");
	// if (fileOutHandle == NULL)
	// {
	//		printf("Error opening file (write)!");
	//	// fclose(fileInHandle);
	//	return 1;
	//}
	//else
	//{
	//	printf("File opened for writing.");
	//}		// fileOUT

// <<< ;

	*/


// .4. File I/O: input and output file streams (see also "AureaPreProcessingToHH_T2.cpp")

/*
	std::ifstream fileInHandle(fileInName);
	if (!fileInHandle.is_open()) { return 1; }
*/


// .5. Parsing strings, string streams, and char arrays: //

/*
// TEMP: testing parsing strings: //
std::string sTest = "   123   654   22   33";

cout << sTest << std::endl;

uint32_t ui32TEST1 = 0;
double dTEST1 = 0.0;

uint32_t ui32TEST2 = 0;
double dTEST2 = 0.0;

cout << b << std::endl;
cout << ui32TEST1 << std::endl;
cout << dTEST1 << std::endl;

cout << std::endl << std::endl << "Now - with StringStream:" << std::endl;

// String Stream demo:

// std::istringstream issTMP(line);
// Better way (if >1 assignment is needed): >>
std::istringstream issTMP;
issTMP.str(line);
// <<< ;

iss >> ui32TEST1 >> dTEST1;
cout << ui32TEST1 << std::endl;
cout << dTEST1 << std::endl;

b = sscanf("   325   5.43", "%d %lf", &ui32TEST1, &dTEST1);

cout << b << std::endl;
*/


// .6. Reading from a binary file: (tested)
/*
// TESTING THE OUTPUT:
	// TESTING THE READ FROM BINARY FILE ROUTINE //
double c = 0;

std::ifstream fileTMP(fname02out1, ios::out | ios::binary);
if (!fileTMP.is_open()) { cout << "Error opening the file!"; return 1; }

fileTMP.read((char*)& a, sizeof(uint64_t));
fileTMP.read((char*)& c, sizeof(double));

cout << std::endl << a;
cout << std::endl << c << std::endl;

fileTMP.close();

return 0;
// << //
*/


// .7. Writing to 'cout' (maybe, other streams also?) with given precision (7).
/*
#include <iomanip>		// for outputting the floast to required precision with 'cout' etc.
// cout << std::fixed << std::setprecision(2);
// revert:
// file1.unsetf(ios_base::fixed);
// file1 << std::setprecision(16) << thePoint[2];
*/


// .8. Defining a 4-bit structure
//struct foo
//{
//	unsigned a : 4;  // <- 4 bits wide only
//};

//foo myStruct;

//myStruct.a = 0;

//cout << "bitset: " << (std::bitset<4>(myStruct.a)) << std::endl;
//cout << "just a structure: " << myStruct.a << std::endl;

//myStruct.a = myStruct.a - 32;

//cout << "After subtraction" << std::endl << "bitset: " << (std::bitset<4>(myStruct.a)) << std::endl;
//cout << "just a structure: " << myStruct.a << std::endl;



// NUMBER to ITS BYTE (not BIT - it is done with std::bitset<64>(<obj>)) REPRESENTATION (9). //
// char bytesOUTPUT[sizeof(NcntsTOT)];
// std::copy(static_cast<const char*>(static_cast<const void*>(&NcntsTOT)),
//	static_cast<const char*>(static_cast<const void*>(&NcntsTOT)) + sizeof NcntsTOT,
//	bytesOUTPUT);


// .EOF