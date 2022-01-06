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
	fileOutHandle.write(   (const char*)TagRECTYPE.c_str(), (std::streamsize)(AUREABytesInATagName * sizeof(char))   );
	fileOutHandle.write((const char*)& outputIdx, sizeof(int32_t));
	outputType = 0x10000008; // tyInt8 = 0x10000008; tyFloat8 = 0x20000008.
	// tyAnsiString  = 0x4001FFFF, tyBinaryBlob  = 0xFFFFFFFF
	fileOutHandle.write((const char*)& outputType, sizeof(uint32_t));
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
	fileOutHandle.write((const char*)& ui64TempOutput, sizeof(uint64_t));

	// fileOutHandle.write((const char*)TagGLOBRES.c_str(), (TagGLOBRES.length()) * sizeof(char));
	fileOutHandle.write(   (const char*)TagGLOBRES.c_str(), (std::streamsize)(AUREABytesInATagName * sizeof(char))   );
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
	
	// outputValueUINT64 = (uint64_t)(trunc(inMeasTime));
	// STAGE3 (SPLITTER) >>>>>
	outputValueUINT64 = (uint64_t)(trunc(TimeToSplitSEC));
	// STAGE3 (SPLITTER) <<<<<
	fileOutHandle.write((const char*)& outputValueUINT64, sizeof(uint64_t));
	// << VKORN_TUESDAY

	// fileOutHandle.write((const char*)TagHEADEREND.c_str(), (TagHEADEREND.length()) * sizeof(char));
	fileOutHandle.write(   (const char*)TagHEADEREND.c_str(), (std::streamsize)(AUREABytesInATagName * sizeof(char))   );
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
				BUBtotalItems = BUBtotalItems + ((uint64_t)1);
				// increase the number of records written to file...

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

				BUBtotalItems = BUBtotalItems + ((uint64_t)1);
				// increase the number of records written to file...

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
		// ui64TempOutput = (Ncnts1 + Ncnts2) + nOVFLmarkersREAL; // 'cause we know that now.
		ui64TempOutput = BUBtotalItems;
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

		// outputValueUINT64 = (uint64_t)(trunc(inMeasTime));

		// STAGE3 (SPLITTER) >>>>>
		outputValueUINT64 = (uint64_t)(trunc(TimeToSplitSEC*1e3)); // in milliseconds!
		// STAGE3 (SPLITTER) <<<<<

		OutOutOuttemp.write((const char*)& outputValueUINT64, sizeof(uint64_t));
		// << VKORN_TUESDAY

		OutOutOuttemp.write(   (const char*)TagHEADEREND.c_str(), (std::streamsize)(AUREABytesInATagName * sizeof(char))   );
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