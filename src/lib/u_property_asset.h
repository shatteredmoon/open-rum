#ifndef _U_PROPERTY_ASSET_H_
#define _U_PROPERTY_ASSET_H_

#include <u_asset.h>
#include <u_enum.h>

#include <unordered_map>

#define FIRST_ENGINE_PROPERTY 0x8FFF0000

// Engine defined properties - don't change the value once established as these are potentially saved to data!
constexpr rumAssetID Map_ID_PropertyID{ (rumAssetID)FIRST_ENGINE_PROPERTY };
constexpr rumAssetID Map_PosX_PropertyID{ (rumAssetID)FIRST_ENGINE_PROPERTY + 1 };
constexpr rumAssetID Map_PosY_PropertyID{ (rumAssetID)FIRST_ENGINE_PROPERTY + 2 };
constexpr rumAssetID Map_Wraps_PropertyID{ (rumAssetID)FIRST_ENGINE_PROPERTY + 3 };


// An instance of a property definition from the game database. These serve as a source or template for individual
// property settings. Unlike some other game objects, properties are never instanced, so there is no matching
// rumProperty class.
class rumPropertyAsset : public rumAsset
{
public:

  rumPropertyAsset() = default;

  AssetType GetAssetType() const override
  {
    return GetClassRegistryID();
  }

  Sqrat::Object GetDefaultValue() const
  {
    return m_sqDefaultValue;
  }

  void SetDefaultValue( Sqrat::Object i_sqValue )
  {
    m_sqDefaultValue = i_sqValue;
  }

  const std::string& GetEnumName() const
  {
    return m_strEnumName;
  }

  void SetEnumName( const std::string& i_strEnumName)
  {
    m_strEnumName = i_strEnumName;
  }

  Sqrat::Object GetMaxValue() const
  {
    return m_sqMaxValue;
  }

  void SetMaxValue( Sqrat::Object i_sqValue )
  {
    m_sqMaxValue = i_sqValue;
  }

  Sqrat::Object GetMinValue() const
  {
    return m_sqMinValue;
  }

  void SetMinValue( Sqrat::Object i_sqValue )
  {
    m_sqMinValue = i_sqValue;
  }

  int32_t GetPriority() const
  {
    return m_iPriority;
  }

  void SetPriority( int32_t i_iPriority )
  {
    m_iPriority = i_iPriority;
  }

  const std::string_view GetTypeName() const override;
  const std::string_view GetTypeSuffix() const override;

  uint32_t GetUserFlags() const
  {
    return m_eUserFlags;
  }

  void SetUserFlags( uint32_t i_eUserFlags )
  {
    m_eUserFlags = i_eUserFlags;
  }

  bool GetUsesConstraints() const
  {
    return m_bUseConstraints;
  }

  void SetUsesConstraints( bool i_bUseConstraints )
  {
    m_bUseConstraints = i_bUseConstraints;
  }

  ServiceType GetServiceType() const
  {
    return m_eServiceType;
  }

  void SetServiceType( ServiceType i_eServiceType )
  {
    m_eServiceType = i_eServiceType;
  }

  ClientReplicationType GetClientReplicationType() const
  {
    return m_eClientReplicationType;
  }

  void SetClientReplicationType( ClientReplicationType i_eReplicationType )
  {
    m_eClientReplicationType = i_eReplicationType;
  }

  PropertyValueType GetValueType() const
  {
    return m_eValueType;
  }

  int32_t GetValueType_VM() const
  {
    return rumUtility::ToUnderlyingType( GetValueType() );
  }

  void SetValueType( PropertyValueType i_eValueType )
  {
    m_eValueType = i_eValueType;
  }

  bool IsAssetRef() const
  {
    return ( m_eValueType == PropertyValueType::AssetRef ) ||
           ( m_eValueType >= PropertyValueType::FirstAssetRef && m_eValueType <= PropertyValueType::LastAssetRef );
  }

  bool IsEngineProperty() const
  {
    return GetAssetID() >= FIRST_ENGINE_PROPERTY;
  }

  bool IsGlobal() const
  {
    return( Global_ClientReplicationType == m_eClientReplicationType );
  }

  bool IsRegional() const
  {
    return( Regional_ClientReplicationType == m_eClientReplicationType );
  }

  bool IsPrivate() const
  {
    return( Private_ClientReplicationType == m_eClientReplicationType );
  }

  bool IsPersistent() const
  {
    return m_bPersistent;
  }

  void SetPersistence( bool i_bPersistent )
  {
    m_bPersistent = i_bPersistent;
  }

  std::string ToString() const override;

  static void ExportCSVFiles( std::ofstream& o_rcOutfile, std::ofstream& o_rcPropertyOutfile );
  static void ExportDBTables( ServiceType i_eServiceType );

  static rumPropertyAsset* Fetch( rumAssetID i_eAssetID );

  static const std::string GetAssetClassName();

  typedef std::unordered_map< rumAssetID, rumPropertyAsset* > AssetHash;
  static const AssetHash& GetAssetHash()
  {
    return s_hashAssets;
  }

  static const std::string GetAssetTypeSuffix();

  static AssetType GetClassRegistryID()
  {
    return Property_AssetType;
  }

  static Sqrat::Object GetDefaultValueForType( PropertyValueType i_eType );

  static Sqrat::Object GetMaxPropertyValue( rumAssetID i_eAssetID );
  static Sqrat::Object GetMinPropertyValue( rumAssetID i_eAssetID );

  static const std::string GetNativeClassName();

  static rumAssetID GetNextFreeID()
  {
    return s_eNextFreeID;
  }

  static const std::string GetPropertyTableCreateQuery();
  static const std::string GetPropertyTableSelectQuery();

  static std::string_view GetStorageName();

  static const std::string GetTableCreateQuery();
  static const std::string GetTableSelectQuery();

  static int32_t Init( const std::string& i_strPath );

  static void Remove( rumAssetID i_eAssetID )
  {
    s_hashAssets.erase( i_eAssetID );
  }

  static void ScriptBind();

  static void Shutdown();

  static bool Validate( rumAssetID i_eAssetID, Sqrat::Object& io_sqValue );

  void OnCreated( const std::vector<std::string>& i_rvFields ) override;

private:

  rumPropertyAsset( rumAssetID i_eAssetID, const std::string& strName, const std::string& strClass );

  void ExportCSVFile( std::ofstream& o_rcOutfile ) const final;
  void ExportDBTable( std::string& io_strQuery ) const final;

  static rumAssetID s_eNextFreeID;
  static AssetHash s_hashAssets;

  // An enumeration that specifies the expected data type of this property
  PropertyValueType m_eValueType{ PropertyValueType::Integer };

  // A generic bitfield
  uint32_t m_eUserFlags{ 0 };

  // Used for controlling load and network priority dependencies. Database fetches and network serialization are sorted
  // by priority.
  int32_t m_iPriority{ 0 };

  // How property changes are replicated to clients
  ClientReplicationType m_eClientReplicationType{ None_ClientReplicationType };

  ServiceType m_eServiceType{ Shared_ServiceType };

  Sqrat::Object m_sqDefaultValue;

  // The min and max values to limit values by when constraints are active
  Sqrat::Object m_sqMaxValue;
  Sqrat::Object m_sqMinValue;

  // When an enum name is provided, editor and debug output values can be cross-indexed with enum data to provide more
  // meaningful output. The enum table must be defined in scripts.
  std::string m_strEnumName;

  // When true, values set in m_sqMinValue and m_sqMaxValue will be applied to the property value
  bool m_bUseConstraints{ false };

  // Are server-side property changes saved to db?
  bool m_bPersistent{ false };

  using super = rumAsset;
};

#endif // _U_PROPERTY_ASSET_H_
