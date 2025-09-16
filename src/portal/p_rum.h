#ifndef _P_RUM_H_
#define _P_RUM_H_

#define PROGRAM_SHORT_DESC  "RUM Portal"
#define PROGRAM_LONG_DESC   "R/U/M Portal"

#define DEFAULT_CONFIG      "rum_portal"

#define DEFAULT_LIBRARY_DB  "library.db"

#define DEFAULT_WEB_PORT    80
#define DEFAULT_WEB_ADDR    "127.0.0.1"

#include <platform.h>

#include <string>

// A full download downloads all client and game files if they do not already exist or do not match MD5 checksums. Full
// downloads will also overwrite editable files (but makes a backup copy of any file overwritten).
// A patch downloads only game files that do not already exist or do not match MD5 checksums. Does not overwrite
// editable files.
// A repair is just like a full download, but will completely remove and re-download the entire scripts folder contents
// for the specified game.
enum DownloadType
{
  Download_None, Download_Full, Download_Patch, Download_Repair
};

struct rumConfig
{
  rumConfig()
    : m_eDownloadType( Download_None )
    , m_iWebPort( DEFAULT_WEB_PORT )
    , m_strWebAddress( DEFAULT_WEB_ADDR )
    , m_bAutoStart( false )
  {}

  DownloadType m_eDownloadType;

  int32_t m_iWebPort;

  std::string m_strUuid;

  std::string m_strPortalINI;
  std::string m_strPortalLog;

  std::string m_strWebAddress;
  std::string m_strDownloadPath;

  bool m_bAutoStart;
};

#endif // _P_RUM_H_
