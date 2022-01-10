using namespace std;

#include "KVV_Header.h"


// use the command line arguments to pass the names of the files to process.
// ARGUMENT ORDER: <Source_file_name>, <Output_file_name>, <Path_to_temp_folder>. //

// For running from IDE, use "else" statement for manually enetring the filenames.

int main(int argc, char ** argv) 
//int testfun(int argc, char ** argv)	// temp., for compiling other files. //
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


	b = PreProcessAureaDataStage1(fname01in.c_str(), fname02out1.c_str(), 
		fname03out2.c_str(),
		fname03out3header.c_str(),
		&dPSin1Tag, &MeasTime, &NcntsCh1, &NcntsCh2, &MAINskippedCh1, &MAINskippedCh2);

	cout << "Process exited with code: b == " << b << std::endl;
	cout << "dPSin1Tag == " << dPSin1Tag << " \t" << "MeasTime (ms) = " << MeasTime << std::endl;
	
	cout << std::endl << "Counts in Ch.1: " << NcntsCh1 << " ; \tCounts in Ch.2: " << NcntsCh2 << std::endl;
	cout << "Total counts (N_ch1 + N_ch2): " << (NcntsCh1 + NcntsCh2) << std::endl;
	cout << std::endl << "Total counts skipped (zero or too large):" << std::endl;
	cout << "Ch.1: " << MAINskippedCh1 << " \t Ch.2: " << MAINskippedCh2 << std::endl;

	cout << std::endl << std::endl << "[REDUNDANT!] waiting 1 second for the files to sync..." << std::endl << std::endl;
	Sleep(200);

	cout << std::endl << "Starting a new part - combining 3 files into one (HH T2 format)(Stage 2):" << std::endl << std::endl;
	cout << "myOverflowVal == " << myOverflowVal << std::endl;

	// VKORN DESPERATE FLAG >>
	//b = PreProcessAureaDataStage2(fname04result.c_str(), fname02out1.c_str(),
	//	fname03out2.c_str(), fname03out3header.c_str(), dPSin1Tag,
	//	MeasTime, (NcntsCh1+NcntsCh2+MAINskippedCh1+MAINskippedCh2), &MAINnOVFLmarkersESTIMATE, &MAINnOVFLmarkersREAL);

	
	// b = PreProcessAureaDataStage2(fname04result.c_str(), fname02out1.c_str(),
	//	fname03out2.c_str(), fname03out3header.c_str(), dPSin1Tag,
	//	MeasTime, NcntsCh1, NcntsCh2, &MAINnOVFLmarkersESTIMATE, &MAINnOVFLmarkersREAL);
	

	
	// double SECONDStoSPLIT = 60.0; // seconds
	double SECONDStoSPLIT = MeasTime * 1e-3; // seconds
	// this works nicely for taking all the measurement data points.


	b = PreProcessAureaDataStage3splitter(SECONDStoSPLIT, fname04result.c_str(), fname02out1.c_str(),
		fname03out2.c_str(), fname03out3header.c_str(), dPSin1Tag,
		MeasTime, NcntsCh1, NcntsCh2, &MAINnOVFLmarkersESTIMATE, &MAINnOVFLmarkersREAL);
	

	// << VKORN DESPERATE FLAG

	cout << "Process exited with code: b == " << b << std::endl;

	
	
#ifdef use_back_conversion_of_stage_4

	// process only the first file of a bucket - for simplicity matters:

	// ToDo: re-introduce path variables, and strip the path to main file. //
	// Then it will be possible to use auto-generated path here... //

	cout << "performing back-conversion (stage 4):" << std::endl;	
	cout << "File name: " << (fname04result + std::string("1.dat")).c_str() << std::endl;

	std::string stage4fileOutCh1;
	std::string stage4fileOutCh2;
	stage4fileOutCh1 = "C:\\tmp\\LabProcessing\\Dec24\\back_conv_ch1.dat";
	stage4fileOutCh2 = "C:\\tmp\\LabProcessing\\Dec24\\back_conv_ch2.dat";

	b = ConvertPTUtoAUREA(	(fname04result + std::string("1.dat")).c_str(),
							stage4fileOutCh1.c_str(),
							stage4fileOutCh2.c_str() );

	cout << "Process exited with code: b == " << b << std::endl;
	
#else
	cout << "Back-conversion not performed -- uncomment it in 'source.cpp'." << std::endl;
#endif
	
	return 0;
}