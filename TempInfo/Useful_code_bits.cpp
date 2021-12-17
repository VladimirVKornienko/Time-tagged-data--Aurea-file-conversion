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