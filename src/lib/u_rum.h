/*

R/U/M Construction Kit Library

MIT License

Copyright 2015 Jonathon Blake Wood-Brooks

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#ifndef _U_RUM_H_
#define _U_RUM_H_

#include <platform.h>
#include <string>

typedef int32_t NativeHandle;

#define PROGRAM_MAJOR_VERSION 1
#define PROGRAM_MINOR_VERSION 0

#define RUM_INVALID_NATIVEHANDLE -1

#define RUM_BANNER \
    "---------------------------------------------------------\n\n" \
    " R/U/M Construction Kit" \
    " www.shatteredmoon.com\n\n" \
    " Copyright 2015 Jonathon Blake Wood-Brooks\n\n" \
    "---------------------------------------------------------\n\n"

#define DEFAULT_GAME_PATH "game"
#define DEFAULT_GAME_FILE DEFAULT_GAME_PATH ## ".rum"

#define CSV_FOLDER_NAME   "csv"
#define CSV_EXTENSION     "." ## CSV_FOLDER_NAME

// All functions below must be defined by any program linking against this lib

struct rumConfig;

const rumConfig& GetConfig();
double GetElapsedTime();
uint64_t GetFrameIndex();
const std::string& GetProjectPath();

#define LINE_BREAK "-----------------------------------------------------------------------"

enum { RESULT_SUCCESS, RESULT_FAILED };

static const char* g_pstrLastErrorString{ nullptr };

#endif // _U_RUM_H_
