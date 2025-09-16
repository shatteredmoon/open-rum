#pragma once

// Various Squirrel Imgui Debugger settings

#define DEBUGGER_DISPLAY_WIDTH  1920
#define DEBUGGER_DISPLAY_HEIGHT 1280

// The number of lines of data to preview before offering an expandable section that opens in a modal window
#define NUM_VARIABLE_PREVIEW_LINES 3

// The buffer size to use for filename handling
#define MAX_FILENAME_LENGTH 260

// Set to non-zero to enable some helpful debug logging
#define DEBUG_OUTPUT 0

#if DEBUG_OUTPUT
#include <cassert>
#include <iostream>
#endif // DEBUG_OUTPUT
