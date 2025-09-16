#ifndef _U_PATCHER_H_
#define _U_PATCHER_H_

#include <list>
#include <map>
#include <string>
#include <vector>


class rumPatcher
{
public:

  enum class PatchType : int32_t
  {
    None        = -1,
    Standard    =  0,
    Conditional =  1,
    Remove      =  2,
    Ignore      =  3,
    NumTypes    =  4
  };

  struct rumPatchFileInfo
  {
    std::string m_strFile;
    std::string m_strFileCRC;
    int64_t m_uiFileSize{ 0UL };

    bool operator<( const rumPatchFileInfo& i_rcPatchEntry ) const
    {
      return m_strFile < i_rcPatchEntry.m_strFile;
    }

    bool operator==( const std::string& i_strFile ) const
    {
      return m_strFile.compare( i_strFile ) == 0;
    }
  };

  using PatchMap = std::map<PatchType, std::list<rumPatchFileInfo>>;

  static PatchMap& AccessPatchMap()
  {
    return s_cPatchEntries;
  }

  static void ExportCSVFiles( const std::string& i_strPath );
  static void ExportDBTables( std::vector<PatchType>&& i_cPatchTypesVector );

  static const std::string_view GetPatchDatabaseName();
  static const std::string_view GetPatchFileName();
  static const std::string_view GetPatchTableName();

  static const std::string_view GetTypeName( PatchType i_ePatchType );

  static void Init( const std::string& i_strPath );
  static void Shutdown();

private:

  static void ExportCSVFile( PatchType i_ePatchType, const std::string& i_strPath );
  static void ExportDBTable( PatchType i_ePatchType );

  static void ImportCSVFile( PatchType i_ePatchType, const std::string& i_strPath );
  static void ImportDBTable( PatchType i_ePatchType );

  static void LoadPatchData( const std::string& i_strPath );

  static PatchMap s_cPatchEntries;

  static std::vector<std::string> s_cPatchCategoriesVector;
};

#endif // _U_PATCHER_H_
