//
// Copyright (c) 2021 bingxio（丙杺，黄菁）. All rights reserved.
//

// GNU General Public License, more to see file: LICENSE
// https://www.gnu.org/licenses

//          THE DRIFT PROGRAMMING LANGUAGE
//
//          https://github.com/bingxio/drift
//
//          https://www.drift-lang.fun/
//

#ifndef DRIFT_UTIL_H
#define DRIFT_UTIL_H

#include <ctime>
#include <fstream>
#include <iostream>
#include <vector>

// return whether the string is all numbers
bool isNumberStr(const std::string);

// read the string buffer into string
bool fileString(const char *, std::string *);

// return the whether a value is the same
bool sameValue(std::vector<std::string> &, std::vector<std::string> &);

// generate a random strings with length
std::string strRand(int, bool);

#endif