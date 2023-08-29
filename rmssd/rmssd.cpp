// Calculate RMSSD using a variety of floating point representations.
// RMSSD is the root mean square of successive RR interval differences.
// Writen by Tim Murphy <tim@murphy.org> 2023

#include "InvalidArgument.h"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <math.h> // for sqrt and sqrtf, and rounding
#include <string>
#include <tgmath.h> // for sqrtl
#include <vector>

namespace {

// make sure we do square roots with the correct types
template <typename T>
inline T squareroot(T val);

template<> inline float squareroot(float val)
{
    return sqrtf(val);
}

template<> inline double squareroot(double val)
{
    return sqrt(val);
}

template<> inline long double squareroot(long double val)
{
    return sqrtl(val);
}

// convert a string to a specific floating point type
// note: can throw invalid_argument and out_of_range
template <typename T>
inline T stringToFP(const std::string &numString);

template<> inline float stringToFP(const std::string &numString)
{
    return std::stof(numString);
}

template<> inline double stringToFP(const std::string &numString)
{
    return std::stod(numString);
}

template<> inline long double stringToFP(const std::string &numString)
{
    return std::stold(numString);
}

// load interval data from file
// @param dataFilePath path to data file where each line contains a single
//                     decimal value in string form.
// @param roundTo number of decimal places to round each value to (-1 for none)
template <typename T>
std::vector<T> loadIntervals(const std::string &dataFilePath, int roundTo = -1)
{
    std::vector<T> intervals;

    std::fstream infile;
    infile.open(dataFilePath, std::ios::in);

    if (!infile.is_open())
    {
        throw InvalidArgument(
            std::string(__FILE__) + ":" + std::to_string(__LINE__) +
            " : could not open data file " + dataFilePath);
    }

    // convert each line into a floating point number of the correct type
    std::string numString;
    while (std::getline(infile, numString))
    {
        T num = ::stringToFP<T>(numString);

        if (roundTo >= 0)
        {
            T multiplier = static_cast<T>(std::pow(10.0, roundTo));
            num = round(num * multiplier) / multiplier;
        }

        intervals.push_back(num);
    }

    infile.close();

    return intervals;
}

// calculate the RMSSD from values in a text file.
// @param dataFilePath path to data file where each line contains a single
//                     decimal value in string form.
// @param roundTo number of decimal places to round each value to (-1 for none)
template <typename T>
T calculateRMSSD(const std::string &dataFilePath, int roundTo = -1)
{
    std::vector<T> intervals(::loadIntervals<T>(dataFilePath, roundTo));

    // sanity checks
    if (intervals.size() < 2)
    {
        throw InvalidArgument(
            std::string(__FILE__) + ":" + std::to_string(__LINE__) +
            " : too few RR intervals to calculate RMSSD");
    }

    // formula:
    // 1. calculate the difference between successive items
    // 2. square each item
    // 3. calaculate the mean value
    // 4. calculate the square root

    std::cout << (sizeof(T) * 8) << "-bit float" << std::endl;

    // 1. calculate the difference between successive items
    // for each element, the diff is that element minus the previous element
    // there is no diff for the first element so diffs will be 1 element
    // smaller than intervals.
    std::vector<T> diffs;
    T prev;
    for (auto it = begin(intervals); it != end(intervals); ++it)
    {
        if (it != begin(intervals))
        {
            diffs.push_back(*it - prev);
        }

        prev = *it;
    }

    // 2. square each item
    for (auto &diff : diffs)
    {
        diff *= diff;
    }

    // 3. calculate the mean
    // we will do this manually to ensure the correct datatype is used at all times
    T sum = static_cast<T>(0.0);
    for (auto &diff : diffs)
    {
        sum += diff;
    }
    T mean = sum / static_cast<T>(diffs.size());

    // 4. calculate the square root
    T root = ::squareroot(mean);

    return root;
}

} // end anonymous namespace

int main()
{
    // show lots of decimal places
    std::cout << std::fixed;
    std::cout << std::setprecision(80);

    // FIXME make this a command line argument
    const std::string dataFilePath("test/RRIntervals_P7D1_Baseline.txt");

    try
    {
        // note: these need to be separate statements otherwise the compiler
        //       will parallelise the RMSSD calls and mess up our output.
        std::cout << "float" << std::endl;
        std::cout << ::calculateRMSSD<float>(dataFilePath) << std::endl;
        std::cout << std::endl;

        std::cout << "double" << std::endl;
        std::cout << ::calculateRMSSD<double>(dataFilePath) << std::endl;
        std::cout << std::endl;

        std::cout << "long double" << std::endl;
        std::cout << ::calculateRMSSD<long double>(dataFilePath) << std::endl;
        std::cout << std::endl;

        std::cout << "float (rounded to 3 decimal places)" << std::endl;
        std::cout << ::calculateRMSSD<float>(dataFilePath, 3) << std::endl;
        std::cout << std::endl;

        std::cout << "double (rounded to 3 decimal places)" << std::endl;
        std::cout << ::calculateRMSSD<double>(dataFilePath, 3) << std::endl;
        std::cout << std::endl;

        std::cout << "long double (rounded to 3 decimal places)" << std::endl;
        std::cout << ::calculateRMSSD<long double>(dataFilePath, 3) << std::endl;
        std::cout << std::endl;
    }
    catch (InvalidArgument &e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }

    std::cout << "Press <enter> to exit" << std::endl;
    std::cin.get();

    return EXIT_SUCCESS;
}