#ifndef _U_DATATABLE_H_
#define _U_DATATABLE_H_

#include <u_db.h>
#include <u_enum.h>
#include <u_language.h>

#include <unordered_map>


struct rumDataTableColumn
{
  std::string m_strName;
  PropertyValueType m_eValueType{ PropertyValueType::Integer };
  std::vector<Sqrat::Object> m_cValueVector;
};


class rumDataTable
{
public:
  using DataTableID = uint32_t;

  rumDataTableColumn& AccessColumnData( uint32_t i_uiColumn );
  const rumDataTableColumn& GetColumnData( uint32_t i_uiColumn ) const;

  uint32_t AddDataColumn( std::string i_strName, PropertyValueType i_eValueType );
  bool AddDataEntry();

  void ExportCSVFile( const std::string& i_strPath ) const;

  const DataTableID GetID() const
  {
    return m_uiDataTableID;
  }

  const std::string& GetName() const
  {
    return m_strName;
  }

  void SetName( const std::string& i_strName )
  {
    m_strName = i_strName;
  }

  uint32_t GetNumColumns() const
  {
    return static_cast<uint32_t>( m_cDataTableColumnVector.size() );
  }

  uint32_t GetNumEntries() const
  {
    return m_cDataTableColumnVector.empty()
            ? 0 : static_cast<uint32_t>( m_cDataTableColumnVector[0].m_cValueVector.size() );
  }

  ServiceType GetServiceType() const
  {
    return m_eServiceType;
  }

  void SetServiceType( ServiceType i_eServiceType )
  {
    m_eServiceType = i_eServiceType;
  }

  int32_t Load();

  void RemoveDataColumn( uint32_t i_uiCol );
  void RemoveDataEntry( uint32_t i_uiRow );
  bool RemoveDataTable( DataTableID i_uiDataTableID );

  static bool AddDataTable( DataTableID i_uiDataTableID, const std::string& i_strName, ServiceType i_eServiceType, bool i_bNewTable );

  static void ExportCSVFiles( const std::string& i_strPath );
  static void ExportDB( const std::string& i_strPath, std::vector<ServiceType>&& i_rcServiceTypeVector );

  static DataTableID GetNextDataTableID()
  {
    return s_uiNextDataTableID;
  }

  static Sqrat::Array GetColumnData( DataTableID i_uiDataTableID, uint32_t i_uiCol );
  static Sqrat::Array GetRowData( DataTableID i_uiDataTableID, uint32_t i_uiRow );

  static rumDataTable& GetDataTable( DataTableID i_uiDataTableID );
  static std::vector<DataTableID> GetDataTableIDs();

  static int32_t Init( const std::string& i_strPath );

  // Intended for the editor
  static int32_t LoadDataTable( DataTableID i_iTableID );

  static void ScriptBind();

  static void Shutdown();

  static constexpr DataTableID DEFAULT_DATATABLE_ID{ 0 };
  static constexpr DataTableID INVALID_DATATABLE_ID{ 0xFFFFFFFF };

private:

  void ExportCSVFile( std::ofstream& o_rcOutfile ) const;
  void ExportDBTable() const;

  int32_t LoadFromFile( const std::string& i_strFilePath );
  int32_t LoadFromDatabase();

  static int32_t LoadDataTableHash();
  static void ParseFields( const std::vector<std::string>& i_rvFields );

  std::string m_strName;

  DataTableID m_uiDataTableID{ INVALID_DATATABLE_ID };

  std::vector<rumDataTableColumn> m_cDataTableColumnVector;

  ServiceType m_eServiceType{ Shared_ServiceType };

  bool m_bLoaded{ false };

  //static void ScriptBind( rumAssetID i_eAssetID, const std::string& i_strName );

  using rumDataTableHash = std::unordered_map<DataTableID, rumDataTable>;
  static rumDataTableHash s_hashDataTables;

  static std::string s_strPath;

  // The next availabe data table ID
  static DataTableID s_uiNextDataTableID;
};

#endif // _U_DATATABLE_H_
