/*

R/U/M Construction Kit Archive Reader

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

#include <u_zlib.h>

#include <iostream>


struct configStruct
{
  configStruct() : unpack( false ), file( "" )
  {}

  bool unpack;
  std::string file;
};


int32 parseArgs( int32, char*[], configStruct & );
void showUsage();


int32 main( int32 argc, char* argv[] )
{
  configStruct cfg;

  // Handle command line early for usage requests
  if( argc > 1 )
  {
    // Parse command line arguments, store arguments in config struct
    if( parseArgs( argc, argv, cfg ) != RESULT_SUCCESS )
    {
      return RESULT_FAILED;
    }
  }
  else
  {
    showUsage();
    return RESULT_FAILED;
  }

  // Pack or unpack the provided file
  if( cfg.unpack == true )
  {
    int32 result = zlib::extractArchive( cfg.file );
    if( result == RESULT_SUCCESS )
    {
      std::cout << "Archive successfully unpacked" << std::endl;
    }
    else
    {
      std::cout << "Error encountered while unpacking archive" << std::endl;
    }
  }
  else
  {
    std::cout << "TODO" << std::endl;
  }

  return 0;
}


int32 parseArgs( int32 argc, char* argv[], configStruct &cfg )
{
  int32 result = RESULT_SUCCESS;
  bool bShowUsage = false;

  // Cycle through all parameters
  for( int32 i = 1; !bShowUsage && i < argc; ++i )
  {
    if( stricmp( "-u", argv[i] ) == 0 )
    {
      cfg.unpack = true;
    }
    else if( strcmp( "-?", argv[i] ) == 0 ||
             stricmp( "-h", argv[i] ) == 0 ||
             stricmp( "-help", argv[i] ) == 0 )
    {
      bShowUsage = true;
    }
    else if( cfg.file.empty() )
    {
      cfg.file = argv[i];
    }
    else
    {
      bShowUsage = true;
    }
  }

  if( cfg.file.empty() == true )
  {
    bShowUsage = true;
  }

  if( bShowUsage )
  {
    showUsage();
    result = RESULT_FAILED;
  }

  return result;
}


void showUsage()
{
  std::cout << std::endl;
  std::cout << "RUM Archiver" << std::endl;
  std::cout << std::endl;

#ifdef WIN32
  std::cout << "rum_archive [-?|-h|-help] | [-u] file" << std::endl;
  std::cout << std::endl;
  std::cout << "-?, -h, -help" << std::endl;
  std::cout << "   Displays program execution requirements and command-line parameter help" << std::endl;
  std::cout << std::endl;
  std::cout << "-u" << std::endl;
  std::cout << "   Unpack the provided archive file" << std::endl;
  std::cout << std::endl;
  std::cout << "file" << std::endl;
  std::cout << "   File to pack or unpack" << std::endl;
  std::cout << std::endl;
#else
#error UNHANDLED PLATFORM
#endif
}


namespace Logger
{
  int32 LogStandard( const std::string& strEntry, int32 param )
  {
    std::cout << strEntry << std::endl;
    return RESULT_SUCCESS;
  }

} // namespace Logger
