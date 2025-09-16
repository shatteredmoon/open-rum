#ifndef _E_RUM_H_
#define _E_RUM_H_

#include <u_db.h>

void BindScripts();

int32_t InitDatabase();

void Shutdown();

struct rumConfig
{
  std::string m_strEditorIni;
  std::string m_strEditorLog;
  std::string m_strProjectPath;
};

double GetElapsedTime();

const std::string& GetProjectPath();
void SetProjectPath( const std::string& i_strPath );

rumDatabase::DatabaseID GetDatabaseIDFromFilename( const std::string& i_strFile );

#endif // _E_RUM_H_