#include "KVV_Header.h"

using namespace std;


//	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	//
//	*				Reverse converter (for checking the consistency...)			*	//
//	*																			*	//
//	*									[ Stage 4. ]							*	//
//	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	*	//


int ConvertPTUtoAUREA(const char* fileInNamePTU, const char* fileOutCh1, const char* fileOutCh2,
						double INdPSin1Tag)

	// 20.12.: header processing will be added later.
	// , const char* fileOutNameCh2
	// , const char* fileOutNameHEADER

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

// GGGGGGGGGGGGGGGGGGGG
	uint64_t currTAG = 0;		// reading from file, converting to tags and times
	double currTIMEps = 0.0;	// ps
	
	uint32_t PTUrecord = 0;	// will be used for extracting the data from PTU-file.

// GGGGGGGGGGGGGGGGGGGG

//	const uint64_t T2WRAPAROUND = 210698240;	// ToDo: change to 2^28 here and in pre-processor (Aurea).
	
// 18.11.2021: changed to match PicoHarp Parser...
// From "constants.cpp": const uint32_t myOverflowVal = 210698240 ;
// <<<


	int PTUchannel = 0;		
	uint32_t PTUtimePS = 0;	// number of picoseconds, as read from the record.
	
	double PTUcurrTimeExcessPS = 0.0;	// increased when overflow marker in PTU file,
										// decreased when (% + time) > INdPSin1Tag.
										// Can be negative.

	
	// GGGGGGGGGGGGGGGGGGGG
	// double myPSin1tag;
	// myPSin1tag = (1.0 / (double)625000) * 1e+12; // ps

	// now: <INdPSin1Tag> is passed as a parameter.

	// ToDo:
	// manually programmed to 625 kHz
	// replace with input from main program (later)
	// GGGGGGGGGGGGGGGGGGGG


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
			
			// PTUcurrTimeExcessPS = PTUcurrTimeExcessPS + (double)T2WRAPAROUND;
			PTUcurrTimeExcessPS = PTUcurrTimeExcessPS + (double)myOverflowVal;

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
			// each time we increment a tag (+=1), we decrement <PTUcurrTimeExcessPS> (-= INdPSin1Tag)
			currTIMEps = ((double)PTUtimePS) + PTUcurrTimeExcessPS;
			while (currTIMEps > INdPSin1Tag)
			{
				currTIMEps = currTIMEps - INdPSin1Tag;
				PTUcurrTimeExcessPS = PTUcurrTimeExcessPS - INdPSin1Tag;
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