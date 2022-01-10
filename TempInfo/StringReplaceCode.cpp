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


int main(void)
{
    cout << "Hi!" << std::endl;
    
    const char * initialString = "Test string QQQQ is here!";

    cout << "initial string (const char *): " << initialString << std::endl;

    
    std::string myStrTmp;
    std::string myResStr;

    // myStrTmp.str(initialString);
    myStrTmp = initialString;

    cout << "string: " << myStrTmp << std::endl;

    std::size_t myFindResult;
    
    myFindResult = myStrTmp.find("QQQQ",0);

    cout << myFindResult << std::endl;

    if (myFindResult == string::npos)
    {
        cout << "It's <npos>!" << std::endl;
    }
    else
    {
        cout << "It's no nothing <npos>, all's good." << std::endl;

        myStrTmp.replace(myFindResult, 4, "YE");

        cout << "New value of <myStrTmp>: " << myStrTmp << std::endl;
    }


    myFindResult = myStrTmp.find("WWWWW",0);

    cout << myFindResult << std::endl;

    if (myFindResult == string::npos)
    {
        cout << "It's <npos>!" << std::endl;
    }
    else
    {
        cout << "It's no nothing <npos>, all's good." << std::endl;
    }

    return 0;
}