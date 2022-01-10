//  *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   //
//  *       Uncomment function header, and                              *   //
//  *           comment all the other mains to compile this file.       *   //
//  *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   //

#include "KVV_Header.h"

#ifndef DEBUG_MODE_FIVE_ITERS_ONLY
// #define DEBUG_MODE_FIVE_ITERS_ONLY
// uncomment use the debug print
#endif

using namespace std;

// >>>>    UNCOMMENT HERE   >>>>>> //
// int main(int argc, char ** argv)
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< //
int CheckMaxNumMarkers(int argc, char ** argv)
{
    std::string fileInName;
    
    fileInName = "C:\\tmp\\LabProcessing\\Dec24\\ser11n\\pr_s11f01a_aux_stage3_ch1.dat";
    // << entered manually.
    // upd: or parsed from the command line, if an argument is passed.

    cout << "Checking max. number of consecutive OVFL markers and EVENT POINTS." << std::endl;


    std::vector<std::string> args(argv, argv + argc);
    if (args.size() == 1+1) // 1 argument is there by default...
	{
		cout << "Using file names from the command line argument:" << std::endl;
		fileInName = args[1];
        cout << fileInName << std::endl;
	}
	else
	{
		cout << "Using the pre-defined file name:" << std::endl;
		cout << fileInName << std::endl;
    }
    
    std::ifstream fileInHandle(fileInName);
	if (!fileInHandle.is_open()) { cout << "error opening file." << std::endl; return 1; }

    uint64_t currOVFL = 0;
    uint64_t currVALUES = 0;
    uint64_t maxOVFL = 0;
    uint64_t maxVALUES = 0;

    std::string line;
#ifdef DEBUG_MODE_FIVE_ITERS_ONLY
    int i = 0;
    while ( (i<5) && (std::getline(fileInHandle, line)) )
    {
        i++;
#else
    while (std::getline(fileInHandle, line))
    {
#endif

        if (line == "OVFL_MARKER")
        {   // "OVFL_MARKER" //
#ifdef DEBUG_MODE_FIVE_ITERS_ONLY
            cout << line.c_str() << std::endl;
            cout << "OVFL_MARKER" << std::endl;
#endif
            if (currOVFL == 0)
            {   // switching from VALUE to OVERFLOW //
                if (currVALUES > maxVALUES)
                {
                    maxVALUES = currVALUES;
                }
                currVALUES = 0; // reset counter for further use;
            }
            
            currOVFL++;   // in all the cases
        }
        else
        {   // "regular value" //
#ifdef DEBUG_MODE_FIVE_ITERS_ONLY
            cout << line.c_str() << std::endl;
            cout << "normal value" << std::endl;
#endif
            if (currVALUES == 0)
            {   // switching from OVERFLOW to VALUES //
                if (currOVFL > maxOVFL)
                {
                    maxOVFL = currOVFL;
                }
                currOVFL = 0; // reset counter for further use;
            }
            
            currVALUES++;   // in all the cases
        }
    } // while getline .. 

    cout << std::endl << std::endl;
    cout << "max consecutive VALUES streak: " << maxVALUES << std::endl;
    cout << "max consecutive OVFL_M streak: " << maxOVFL << std::endl << std::endl;

    return 0;
}