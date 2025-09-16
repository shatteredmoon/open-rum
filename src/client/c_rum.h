#ifndef _C_RUM_H_
#define _C_RUM_H_

#include <platform.h>
#include <string>

#define DEFAULT_SERVER_PORT 58888
#define DEFAULT_FPS         60
#define DEFAULT_LANGUAGE    "English"


//INI server options
struct rumConfig
{
  uint32_t m_uiFullScreenHeight{ 640 };
  uint32_t m_uiFullScreenWidth{ 480 };

  uint32_t m_uiWindowedScreenHeight{ 640 };
  uint32_t m_uiWindowedScreenWidth{ 480 };

  uint32_t m_uiColorDepth{ 16 };

  uint32_t m_uiFPS{ DEFAULT_FPS };

  uint32_t m_uiServerPort{ DEFAULT_SERVER_PORT };

  std::string m_strServerAddr;

  std::string m_strClientIni;
  std::string m_strClientLog;

  std::string m_strUUID;

  std::string m_strLanguage{ DEFAULT_LANGUAGE };

  bool m_bFullscreen{ false };
  mutable bool m_bShutdown{ false };
  //bool m_bDatabaseRefresh{ false };

#ifdef _DEBUG
  bool m_bScriptDebug{ false };
#endif // _DEBUG
};

#ifdef WIN32
  int MessageProcedure( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, int *pRetval );
#else
  #error add support for your custom message handler here
#endif

#endif // _C_RUM_H_
