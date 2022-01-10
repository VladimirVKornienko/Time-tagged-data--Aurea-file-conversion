/*
*		Aurea measurement files' pre-processing tool.
*
*   From "header,data" create a "new_header,HH_format_data" file and store.
*
*/

//	*	*	*	*	*	*	*	*	*	*	*	*	*	*	//
//	*		   Log file operation not implemented.		*	//
//	*	*	*	*	*	*	*	*	*	*	*	*	*	*	//

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
	fileOutHandle.write(   (const char*)TagRECTYPE.c_str(), (std::streamsize)(AUREABytesInATagName * sizeof(char))   );
	fileOutHandle.write((const char*)&outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
	// tyAnsiString  = 0x4001FFFF, tyBinaryBlob  = 0xFFFFFFFF
	fileOutHandle.write((const char*)&outputType, sizeof(uint32_t));
	outputValueUINT64 = 0x00010203; // PicoHarp, T2
	// rtHydraHarpT2 = 0x00010204, # (SubID = $00, RecFmt: $01) (V1), T - Mode: $02(T2), HW : $04(HydraHarp)
	fileOutHandle.write((const char*)& outputValueUINT64, sizeof(uint64_t));
	
	// fileOutHandle.write((const char*)TagNUMREC.c_str(), (TagNUMREC.length()) * sizeof(char));
	fileOutHandle.write(   (const char*)TagNUMREC.c_str(), (std::streamsize)(AUREABytesInATagName * sizeof(char))   );
	fileOutHandle.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
	fileOutHandle.write((const char*)& outputType, sizeof(uint32_t));

	uint64_t ui64TempOutput = 0;
	ui64TempOutput = (Ncnts1 + Ncnts2) + nOVFLmarkersESTIMATE;
	fileOutHandle.write((const char*)&ui64TempOutput, sizeof(uint64_t));

	// fileOutHandle.write((const char*)TagGLOBRES.c_str(), (TagGLOBRES.length()) * sizeof(char));
	fileOutHandle.write(   (const char*)TagGLOBRES.c_str(), (std::streamsize)(AUREABytesInATagName * sizeof(char)))   ;
	fileOutHandle.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x20000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
	fileOutHandle.write((const char*)& outputType, sizeof(uint32_t));
	outputValueDOUBLE = ((double)AUREAGlobalResolution);	// in seconds; DEFAULT!
	fileOutHandle.write((const char*)& outputValueDOUBLE, sizeof(double));

	// fileOutHandle.write((const char*)TagACQTIME.c_str(), (TagACQTIME.length()) * sizeof(char));
	fileOutHandle.write(   (const char*)TagACQTIME.c_str(), (std::streamsize)(AUREABytesInATagName * sizeof(char))   );
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
	fileOutHandle.write(   (const char*)TagHEADEREND.c_str(), (std::streamsize)(AUREABytesInATagName * sizeof(char))   );
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
	OutOutOuttemp.write(   (const char*)TagRECTYPE.c_str(), (std::streamsize)(AUREABytesInATagName * sizeof(char))   );
	OutOutOuttemp.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
	// tyAnsiString  = 0x4001FFFF, tyBinaryBlob  = 0xFFFFFFFF
	OutOutOuttemp.write((const char*)& outputType, sizeof(uint32_t));
	outputValueUINT64 = 0x00010203; // PicoHarp, T2
	// rtHydraHarpT2 = 0x00010204, # (SubID = $00, RecFmt: $01) (V1), T - Mode: $02(T2), HW : $04(HydraHarp)
	OutOutOuttemp.write((const char*)& outputValueUINT64, sizeof(uint64_t));

	OutOutOuttemp.write(   (const char*)TagNUMREC.c_str(), (std::streamsize)(AUREABytesInATagName * sizeof(char))   );
	OutOutOuttemp.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
	OutOutOuttemp.write((const char*)& outputType, sizeof(uint32_t));

	// HERE >>
	// ui64TempOutput = (Ncnts1 + Ncnts2) + nOVFLmarkersESTIMATE;
	ui64TempOutput = (Ncnts1 + Ncnts2) + nOVFLmarkersREAL; // 'cause we know that now.
	// <<<
	
	OutOutOuttemp.write((const char*)& ui64TempOutput, sizeof(uint64_t));

	OutOutOuttemp.write(   (const char*)TagGLOBRES.c_str(), (std::streamsize)(AUREABytesInATagName * sizeof(char))   );
	OutOutOuttemp.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x20000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
	OutOutOuttemp.write((const char*)& outputType, sizeof(uint32_t));
	outputValueDOUBLE = ((double)AUREAGlobalResolution);	// in seconds; DEFAULT!
	OutOutOuttemp.write((const char*)& outputValueDOUBLE, sizeof(double));

	OutOutOuttemp.write(   (const char*)TagACQTIME.c_str(), (std::streamsize)(AUREABytesInATagName * sizeof(char))   );
	OutOutOuttemp.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // uint64_t ??? picoseconds??? //
	OutOutOuttemp.write((const char*)& outputType, sizeof(uint32_t));

	// VKORN_TUESDAY >>	
	// outputValueUINT64 = (uint64_t)inMeasTime;
	outputValueUINT64 = (uint64_t)(trunc(inMeasTime));
	OutOutOuttemp.write((const char*)& outputValueUINT64, sizeof(uint64_t));
	// << VKORN_TUESDAY

	OutOutOuttemp.write(   (const char*)TagHEADEREND.c_str(), (std::streamsize)(AUREABytesInATagName * sizeof(char))   );
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

