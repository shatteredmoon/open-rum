#include <u_property_asset.h>

#include <u_broadcast_asset.h>
#include <u_creature_asset.h>
#include <u_custom_asset.h>
#include <u_graphic_asset.h>
#include <u_inventory_asset.h>
#include <u_map_asset.h>
#include <u_portal_asset.h>
#include <u_sound_asset.h>
#include <u_strings.h>
#include <u_tile_asset.h>
#include <u_widget_asset.h>

#define ASSET_NATIVE_CLASS "rumPropertyAsset"
#define ASSET_STORAGE_NAME "property"
#define ASSET_TYPE_SUFFIX  "_Property"

// Static initializations
std::unordered_map< rumAssetID, rumPropertyAsset* > rumPropertyAsset::s_hashAssets;
rumAssetID rumPropertyAsset::s_eNextFreeID{ FULL_ASSET_ID( Property_AssetType, 0 ) };

// Engine defined properties
struct rumEngineProperty
{
  rumAssetID m_eAssetID{ INVALID_ASSET_ID };
  std::string m_strName;
  std::string m_strClass;
  PropertyValueType m_ePropertyValueType{ PropertyValueType::Null };
  ServiceType m_eServiceType{ ServiceType::Shared_ServiceType };
  bool m_bPersistent;
};

std::vector<rumEngineProperty> g_vEngineProperties
{
  { Map_ID_PropertyID,    "Map_ID",    "rumPropertyAsset", PropertyValueType::MapRef,  ServiceType::Shared_ServiceType, true },
  { Map_PosX_PropertyID,  "Map_PosX",  "rumPropertyAsset", PropertyValueType::Integer, ServiceType::Shared_ServiceType, true },
  { Map_PosY_PropertyID,  "Map_PosY",  "rumPropertyAsset", PropertyValueType::Integer, ServiceType::Shared_ServiceType, true },
  { Map_Wraps_PropertyID, "Map_Wraps", "rumPropertyAsset", PropertyValueType::Bool,    ServiceType::Shared_ServiceType, true }
};


rumPropertyAsset::rumPropertyAsset( rumAssetID i_eAssetID, const std::string& i_strName,
                                    const std::string& i_strClass ) : rumAsset()
{
  m_eAssetID = i_eAssetID;
  m_strName = i_strName;
  m_strBaseClassOverride = i_strClass;
}


// final
void rumPropertyAsset::ExportCSVFile( std::ofstream& o_rcOutfile ) const
{
  // Engine properties are not written to CSV
  rumAssert( !IsEngineProperty() );

  super::ExportCSVFile( o_rcOutfile );

  o_rcOutfile << ',' << rumStringUtils::ToHexString( rumUtility::ToUnderlyingType( GetValueType() ) );
  o_rcOutfile << ',' << rumScript::ToSerializationString( GetDefaultValue() );
  o_rcOutfile << ',' << GetEnumName();
  o_rcOutfile << ',' << ( GetUsesConstraints() ? '1' : '0' );
  o_rcOutfile << ',' << rumScript::ToSerializationString( GetMinValue() );
  o_rcOutfile << ',' << rumScript::ToSerializationString( GetMaxValue() );
  o_rcOutfile << ',' << rumStringUtils::ToHexString( GetUserFlags() );
  o_rcOutfile << ',' << rumStringUtils::ToString( GetServiceType() );
  o_rcOutfile << ',' << rumStringUtils::ToString( GetClientReplicationType() );
  o_rcOutfile << ',' << ( IsPersistent() ? '1' : '0' );
  o_rcOutfile << ',' << rumStringUtils::ToString( GetPriority() );

  // Done
  o_rcOutfile << '\n';
}


// static
void rumPropertyAsset::ExportCSVFiles( std::ofstream& o_rcOutfile, std::ofstream& o_rcPropertyOutfile )
{
  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumPropertyAsset* >;
  const SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  // Export csv values for all user-defined properties
  for( const auto& iter : hashAssets )
  {
    const rumPropertyAsset* pcAsset{ iter.second };
    rumAssert( pcAsset );
    if( pcAsset && !pcAsset->IsEngineProperty() )
    {
      pcAsset->ExportCSVFile( o_rcOutfile );
    }
  }
}


// final
void rumPropertyAsset::ExportDBTable( std::string& io_strQuery ) const
{
  // Engine properties are not written to DB
  rumAssert( !IsEngineProperty() );

  io_strQuery = "INSERT INTO " ASSET_STORAGE_NAME " (type_id,name,baseclass,var_type,enum_name,default_value,"
                "constraints,min_value,max_value,user_flags,service_type,replication_type,persistent,priority) "
                "VALUES (";

  super::ExportDBTable( io_strQuery );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToHexString( rumUtility::ToUnderlyingType( GetValueType() ) );

  io_strQuery += ",'";
  io_strQuery += GetEnumName();
  io_strQuery += '\'';

  io_strQuery += ',';
  const std::string strDefault{ rumScript::ToSerializationString( GetDefaultValue() ) };
  io_strQuery += ( !strDefault.empty() ) ? strDefault : "0";

  io_strQuery += ',';
  io_strQuery += ( GetUsesConstraints() ? '1' : '0' );

  io_strQuery += ',';
  const std::string strMin{ rumScript::ToSerializationString( GetMinValue() ) };
  io_strQuery += ( !strMin.empty() ) ? strMin : "0";

  io_strQuery += ',';
  const std::string strMax{ rumScript::ToSerializationString( GetMaxValue() ) };
  io_strQuery += ( !strMax.empty() ) ? strMax : "0";

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToHexString( GetUserFlags() );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToString( GetServiceType() );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToString( GetClientReplicationType() );

  io_strQuery += ',';
  io_strQuery += ( IsPersistent() ? '1' : '0' );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToString( GetPriority() );

  io_strQuery += ");";
}


// static
void rumPropertyAsset::ExportDBTables( ServiceType i_eServiceType )
{
  // Sort the hash
  using SortedHash = std::map< rumAssetID, rumPropertyAsset* >;
  const SortedHash hashAssets( s_hashAssets.begin(), s_hashAssets.end() );

  // Export each asset to db, but only if it's not an engine property
  for( const auto& iter : hashAssets )
  {
    const rumPropertyAsset* pcAsset{ iter.second };
    if( pcAsset && !pcAsset->IsEngineProperty() )
    {
      if( ( pcAsset->GetServiceType() == i_eServiceType ) || ( pcAsset->GetServiceType() == Shared_ServiceType ) )
      {
        std::string strQuery;
        pcAsset->ExportDBTable( strQuery );
        rumDatabase::Query( rumDatabase::Assets_DatabaseID, strQuery );
      }
    }
  }
}


// static
rumPropertyAsset* rumPropertyAsset::Fetch( rumAssetID i_eAssetID )
{
  const AssetHash::iterator iter{ s_hashAssets.find( i_eAssetID ) };
  return ( iter != s_hashAssets.end() ) ? iter->second : nullptr;
}


// static
const std::string rumPropertyAsset::GetAssetClassName()
{
  return ASSET_NATIVE_CLASS;
}


// static
const std::string rumPropertyAsset::GetAssetTypeSuffix()
{
  return ASSET_TYPE_SUFFIX;
}


// static
Sqrat::Object rumPropertyAsset::GetDefaultValueForType( PropertyValueType i_eType )
{
  Sqrat::Object sqValue;

  if( PropertyValueType::FirstAssetRef <= i_eType && PropertyValueType::LastAssetRef >= i_eType )
  {
    rumScript::SetValue( sqValue, INVALID_ASSET_ID );
    return sqValue;
  }
  else if( PropertyValueType::StringToken == i_eType )
  {
    rumScript::SetValue( sqValue, rumStringTable::INVALID_TOKEN_ID );
    return sqValue;
  }

  switch( i_eType )
  {
    case PropertyValueType::Null:
    case PropertyValueType::Bitfield:
    case PropertyValueType::Integer:
      rumScript::SetValue( sqValue, 0 );
      break;

    case PropertyValueType::Float:
      rumScript::SetValue( sqValue, 0.0f );
      break;

    case PropertyValueType::Bool:
      rumScript::SetValue( sqValue, false );
      break;

    case PropertyValueType::String:
      rumScript::SetValue( sqValue, "" );
      break;

    default:
      rumAssertMsg( false, "Unexpected default value type" );
      break;
  }

  return sqValue;
}


// static
Sqrat::Object rumPropertyAsset::GetMinPropertyValue( rumAssetID i_eAssetID )
{
  const rumPropertyAsset* pcAsset{ Fetch( i_eAssetID ) };
  if( pcAsset )
  {
    return pcAsset->GetMinValue();
  }

  return Sqrat::Object();
}


// static
Sqrat::Object rumPropertyAsset::GetMaxPropertyValue( rumAssetID i_eAssetID )
{
  const rumPropertyAsset* pcAsset{ Fetch( i_eAssetID ) };
  if( pcAsset )
  {
    return pcAsset->GetMaxValue();
  }

  return Sqrat::Object();
}


// static
const std::string rumPropertyAsset::GetNativeClassName()
{
  // Properties are never instanced!
  return "";
}


// static
const std::string rumPropertyAsset::GetPropertyTableCreateQuery()
{
  // Properties do not support property tables!
  return "";
}


// static
const std::string rumPropertyAsset::GetPropertyTableSelectQuery()
{
  // Properties do not support property tables!
  return "";
}


// static
std::string_view rumPropertyAsset::GetStorageName()
{
  return ASSET_STORAGE_NAME;
}


// static
const std::string rumPropertyAsset::GetTableCreateQuery()
{
  return "CREATE TABLE[" ASSET_STORAGE_NAME "]("
    "[type_id] INTEGER NOT NULL UNIQUE,"
    "[name] TEXT NOT NULL UNIQUE,"
    "[baseclass] TEXT,"
    "[var_type] INTEGER NOT NULL,"
    "[default_value] TEXT,"
    "[enum_name] TEXT,"
    "[constraints] INTEGER DEFAULT 0,"
    "[min_value] INTEGER,"
    "[max_value] INTEGER,"
    "[user_flags] INTEGER DEFAULT 0,"
    "[service_type] INTEGER NOT NULL DEFAULT 0,"
    "[replication_type] INTEGER DEFAULT 0,"
    "[persistent] INTEGER DEFAULT 0,"
    "[priority] INTEGER DEFAULT 0)";
}


// static
const std::string rumPropertyAsset::GetTableSelectQuery()
{
  return "SELECT type_id,name,baseclass,var_type,default_value,enum_name,constraints,min_value,max_value,"
         "user_flags,service_type,replication_type,persistent,priority FROM " ASSET_STORAGE_NAME;
}


// override
const std::string_view rumPropertyAsset::GetTypeName() const
{
  return ASSET_STORAGE_NAME;
}


// override
const std::string_view rumPropertyAsset::GetTypeSuffix() const
{
  return ASSET_TYPE_SUFFIX;
}


// static
int32_t rumPropertyAsset::Init( const std::string& i_strPath )
{
  rumAssert( s_hashAssets.empty() );

  // Create the built-in properties
  for( const auto& iter : g_vEngineProperties )
  {
    const rumEngineProperty& rcProperty{ iter };

    rumPropertyAsset* pcAsset{ new rumPropertyAsset( rcProperty.m_eAssetID, rcProperty.m_strName, rcProperty.m_strClass ) };
    if( pcAsset != nullptr )
    {
      pcAsset->SetValueType( rcProperty.m_ePropertyValueType );
      pcAsset->SetServiceType( rcProperty.m_eServiceType );
      pcAsset->SetPersistence( rcProperty.m_bPersistent );

      s_hashAssets.insert( std::make_pair( rcProperty.m_eAssetID, pcAsset ) );
    }
  }

  // Build the asset hash from any available resource
  LoadAssets<rumPropertyAsset>( i_strPath );

  return RESULT_SUCCESS;
}


void rumPropertyAsset::OnCreated( const std::vector<std::string>& i_rvFields )
{
  s_hashAssets.insert( std::make_pair( GetAssetID(), this ) );

  if( !IsEngineProperty() && GetAssetID() >= s_eNextFreeID )
  {
    s_eNextFreeID = GetAssetID() + 1;
    rumAssert( s_eNextFreeID < FIRST_ENGINE_PROPERTY );
  }

  if( i_rvFields.empty() )
  {
    return;
  }

  enum { COL_ID, COL_NAME, COL_BASECLASS, COL_TYPE, COL_DEFAULT_VALUE, COL_ENUM_NAME, COL_CONSTRAINTS,
         COL_MIN_VALUE, COL_MAX_VALUE, COL_USER_FLAGS, COL_SERVICE_TYPE, COL_REPLICATION_TYPE, COL_PERSISTENT,
         COL_PRIORITY };

  m_eValueType = (PropertyValueType)rumStringUtils::ToInt( i_rvFields.at( COL_TYPE ) );
  m_strEnumName = i_rvFields.at( COL_ENUM_NAME );
  m_bUseConstraints = rumStringUtils::ToBool( i_rvFields.at( COL_CONSTRAINTS ) );
  if( m_bUseConstraints )
  {
    if( IsAssetRef() )
    {
      rumScript::SetValue( m_sqMinValue, rumStringUtils::ToInt( i_rvFields.at( COL_MIN_VALUE ) ) );
      rumScript::SetValue( m_sqMaxValue, rumStringUtils::ToInt( i_rvFields.at( COL_MAX_VALUE ) ) );
    }
    else
    {
      switch( m_eValueType )
      {
        case PropertyValueType::Bitfield:
        case PropertyValueType::Integer:
          rumScript::SetValue( m_sqMinValue, rumStringUtils::ToInt( i_rvFields.at( COL_MIN_VALUE ) ) );
          rumScript::SetValue( m_sqMaxValue, rumStringUtils::ToInt( i_rvFields.at( COL_MAX_VALUE ) ) );
          break;

        case PropertyValueType::Float:
          rumScript::SetValue( m_sqMinValue, strtof( i_rvFields.at( COL_MIN_VALUE ).c_str(), nullptr ) );
          rumScript::SetValue( m_sqMaxValue, strtof( i_rvFields.at( COL_MAX_VALUE ).c_str(), nullptr ) );
          break;

        default:
          rumAssertArgs( false, "Unexpected value type for property: ", m_eValueType );
          break;
      }
    }
  }
  else
  {
    switch( m_eValueType )
    {
      case PropertyValueType::Bitfield:
      case PropertyValueType::Integer:
        rumScript::SetValue( m_sqMinValue, 0 );
        rumScript::SetValue( m_sqMaxValue, 0 );
        break;

      case PropertyValueType::Float:
        rumScript::SetValue( m_sqMinValue, 0.f );
        rumScript::SetValue( m_sqMaxValue, 0.f );
        break;
    }
  }

  switch( m_eValueType )
  {
    case PropertyValueType::Bitfield:
    case PropertyValueType::Integer:
      rumScript::SetValue( m_sqDefaultValue, rumStringUtils::ToInt( i_rvFields.at( COL_DEFAULT_VALUE ) ) );
      break;

    case PropertyValueType::Float:
      rumScript::SetValue( m_sqDefaultValue, strtof( i_rvFields.at( COL_DEFAULT_VALUE ).c_str(), nullptr ) );
      break;

    case PropertyValueType::Bool:
      rumScript::SetValue( m_sqDefaultValue, rumStringUtils::ToBool( i_rvFields.at( COL_DEFAULT_VALUE ) ) );
      break;

    case PropertyValueType::String:
      rumScript::SetValue( m_sqDefaultValue, i_rvFields.at( COL_DEFAULT_VALUE ) );
      break;
  }

  m_eUserFlags = rumStringUtils::ToUInt( i_rvFields.at( COL_USER_FLAGS ) );
  m_eClientReplicationType = (ClientReplicationType)rumStringUtils::ToInt( i_rvFields.at( COL_REPLICATION_TYPE ) );
  m_bPersistent = rumStringUtils::ToBool( i_rvFields.at( COL_PERSISTENT ) );
  m_iPriority = rumStringUtils::ToInt( i_rvFields.at( COL_PRIORITY ) );
  m_eServiceType = (ServiceType)rumStringUtils::ToInt( i_rvFields.at( COL_SERVICE_TYPE ) );
}


// static
void rumPropertyAsset::ScriptBind()
{
  rumScript::GetOrCreateClassRegistry( GetClassRegistryID() );

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM )
    .Func( "rumGetPropertyAsset", Fetch )
    .Func( "rumGetMaxPropertyValue", GetMaxPropertyValue )
    .Func( "rumGetMinPropertyValue", GetMinPropertyValue );

  Sqrat::DerivedClass<rumPropertyAsset, rumAsset> cPropertyAsset( pcVM, ASSET_NATIVE_CLASS );
  cPropertyAsset
    .Func( "GetEnumName", &GetEnumName )
    .Func( "GetMaxValue", &GetMaxValue )
    .Func( "GetMinValue", &GetMinValue )
    .Func( "GetUserFlags", &GetUserFlags )
    .Func( "GetValueType", &GetValueType_VM )
    .Func( "IsAssetRef", &IsAssetRef )
    .Func( "UsesConstraints", &GetUsesConstraints );
  Sqrat::RootTable( pcVM ).Bind( ASSET_NATIVE_CLASS, cPropertyAsset );

  // Bind all available assets to script
  for( const auto& iter : s_hashAssets )
  {
    BindAssetID( iter.first, iter.second->GetName(), ASSET_TYPE_SUFFIX );
  }
}


// static
void rumPropertyAsset::Shutdown()
{
  auto* pcRegistry{ rumScript::GetOrCreateClassRegistry( GetClassRegistryID() ) };

  for( const auto& iter : s_hashAssets )
  {
    rumPropertyAsset* pcAsset{ iter.second };
    pcRegistry->UnregisterScriptClass( pcAsset->GetAssetID() );
    delete pcAsset;
  }

  s_hashAssets.clear();
}


// override
std::string rumPropertyAsset::ToString() const
{
  return "Asset: " + GetName();
}


// static
bool rumPropertyAsset::Validate( rumAssetID i_eAssetID, Sqrat::Object& io_sqValue )
{
  const rumPropertyAsset* pcPropertyAsset{ Fetch( i_eAssetID ) };
  if( !pcPropertyAsset )
  {
    std::string strInfo{ "Error: Attempt to validate unknown property ID " };
    strInfo += rumStringUtils::ToString( i_eAssetID );
    Logger::LogStandard( strInfo );

    return false;
  }

  bool bValid{ true };

  const PropertyValueType eValueType{ pcPropertyAsset->GetValueType() };

  // Does the incoming value match the expected value type?
  if( ( eValueType == PropertyValueType::Integer ) || ( eValueType == PropertyValueType::Bitfield ) )
  {
    if( io_sqValue.GetType() != OT_INTEGER )
    {
      // Try to convert
      switch( io_sqValue.GetType() )
      {
        case OT_BOOL:
        case OT_FLOAT:
          rumScript::SetValue( io_sqValue, io_sqValue.Cast<int64_t>() );
          break;

        case OT_STRING:
        {
          const std::string str{ io_sqValue.Cast<std::string>() };
          rumScript::SetValue( io_sqValue, rumStringUtils::ToInt64( str ) );
          break;
        }

        default:
          rumAssert( false );
          break;
      }
    }
  }
  else if( eValueType == PropertyValueType::Float )
  {
    if( io_sqValue.GetType() != OT_FLOAT )
    {
      // Try to convert
      switch( io_sqValue.GetType() )
      {
        case OT_INTEGER:
          rumScript::SetValue( io_sqValue, io_sqValue.Cast<float>() );
          break;

        case OT_STRING:
        {
          const std::string str{ io_sqValue.Cast<std::string>() };
          rumScript::SetValue( io_sqValue, rumStringUtils::ToFloat( str ) );
          break;
        }

        default:
          rumAssert( false );
          break;
      }
    }
  }
  else if( eValueType == PropertyValueType::Bool )
  {
    if( io_sqValue.GetType() != OT_BOOL )
    {
      // Try to convert
      switch( io_sqValue.GetType() )
      {
        case OT_FLOAT:
        case OT_INTEGER:
          rumScript::SetValue( io_sqValue, io_sqValue.Cast<bool>() );
          break;

        case OT_STRING:
        {
          const std::string str{ io_sqValue.Cast<std::string>() };
          rumScript::SetValue( io_sqValue, rumStringUtils::ToBool( str ) );
          break;
        }

        default:
          rumAssert( false );
          break;
      }
    }
  }
  else if( eValueType == PropertyValueType::String )
  {
    if( io_sqValue.GetType() != OT_STRING )
    {
      // Try to convert
      switch( io_sqValue.GetType() )
      {
        case OT_BOOL:
        {
          bool bValue{ io_sqValue.Cast<bool>() };
          rumScript::SetValue( io_sqValue, bValue ? "True" : "False" );
          break;
        }

        case OT_INTEGER:
        {
          int64_t iValue{ io_sqValue.Cast<int64_t>() };
          rumScript::SetValue( io_sqValue, rumStringUtils::ToString64( iValue ) );
          break;
        }

        case OT_FLOAT:
        {
          float fValue{ io_sqValue.Cast<float>() };
          rumScript::SetValue( io_sqValue, rumStringUtils::ToFloatString( fValue ) );
          break;
        }

        default:
          rumAssert( false );
          break;
      }
    }
  }
  else if( eValueType == PropertyValueType::StringToken )
  {
    if( io_sqValue.GetType() != OT_INTEGER )
    {
      // Try to convert
      switch( io_sqValue.GetType() )
      {
        case OT_NULL:
        case OT_BOOL:
          rumScript::SetValue( io_sqValue, rumStringTable::INVALID_TOKEN_ID );
          break;

        case OT_FLOAT:
        case OT_INTEGER:
          rumScript::SetValue( io_sqValue, io_sqValue.Cast<rumTokenID>() );
          break;

        case OT_STRING:
        {
          rumTokenID eTokenID{ rumStringTable::INVALID_TOKEN_ID };
          const std::string strToken{ io_sqValue.Cast<std::string>() };

          const rumTokenID eConvertedTokenID{ rumStringUtils::ToUInt( strToken ) };
          if( eConvertedTokenID != 0 )
          {
            eTokenID = eConvertedTokenID;
          }
          else
          {
            const auto& rcStringTableIDVector{ rumStringTable::GetStringTableIDs() };
            if( !rcStringTableIDVector.empty() )
            {
              // Try to find the string
              for( const auto uiStringTableID : rcStringTableIDVector )
              {
                const auto& rcStringTable{ rumStringTable::GetStringTable( uiStringTableID ) };
                const auto& rcTokenHash{ rcStringTable.GetTokenHash() };
                for( const auto& iter : rcTokenHash )
                {
                  if( strToken.compare( iter.second ) == 0 )
                  {
                    // String found!
                    eTokenID = iter.first;
                    break;
                  }
                }

                if( rumStringTable::INVALID_TOKEN_ID != eTokenID )
                {
                  break;
                }
              }
            }
          }

          rumScript::SetValue( io_sqValue, eTokenID );
          break;
        }

        default:
          rumAssert( false );
          break;
      }
    }
  }
  else if( pcPropertyAsset->IsAssetRef() )
  {
    if( io_sqValue.GetType() != OT_INTEGER )
    {
      // Try to convert
      switch( io_sqValue.GetType() )
      {
        case OT_STRING:
        {
          rumAsset* pcAsset{ nullptr };

          const std::string str{ io_sqValue.Cast<std::string>() };
          rumAssetID eAssetID{ static_cast<rumAssetID>( rumStringUtils::ToInt( str ) ) };
          if( eAssetID != INVALID_ASSET_ID )
          {
            pcAsset = rumAsset::Fetch( eAssetID );
          }

          if( nullptr == pcAsset )
          {
            // Try the much more expensive fetch
            pcAsset = rumAsset::FetchByName( str );
          }

          if( pcAsset != nullptr )
          {
            rumScript::SetValue( io_sqValue, pcAsset->GetAssetID() );
          }
          else
          {
            rumScript::SetValue( io_sqValue, eAssetID );
          }
          break;
        }

        default:
          rumAssert( false );
          break;
      }
    }
  }
  else
  {
    bValid = false;
  }

  if( bValid )
  {
    if( ( pcPropertyAsset->GetValueType() == PropertyValueType::Bitfield ) ||
        ( pcPropertyAsset->GetValueType() == PropertyValueType::Integer ) ||
        ( pcPropertyAsset->GetValueType() == PropertyValueType::Float ) )
    {
      // Apply constraints if they have been provided
      Sqrat::Object sqMinValue;
      Sqrat::Object sqMaxValue;

      const bool bUsesContstraints{ pcPropertyAsset->GetUsesConstraints() };
      if( bUsesContstraints )
      {
        Sqrat::Object sqMinValue{ pcPropertyAsset->GetMinValue() };
        if( sqMinValue.GetType() != OT_NULL && ( sqMinValue.GetType() == io_sqValue.GetType() ) )
        {
          if( sqMinValue.GetType() == OT_INTEGER )
          {
            if( io_sqValue.Cast<int32_t>() < sqMinValue.Cast<int32_t>() )
            {
              io_sqValue = sqMinValue;
            }
          }
          else if( sqMinValue.GetType() == OT_FLOAT )
          {
            if( io_sqValue.Cast<float>() < sqMinValue.Cast<float>() )
            {
              io_sqValue = sqMinValue;
            }
          }
        }

        Sqrat::Object sqMaxValue{ pcPropertyAsset->GetMaxValue() };
        if( sqMaxValue.GetType() != OT_NULL && ( io_sqValue.GetType() == sqMaxValue.GetType() ) )
        {
          if( sqMaxValue.GetType() == OT_INTEGER )
          {
            if( io_sqValue.Cast<int32_t>() > sqMaxValue.Cast<int32_t>() )
            {
              io_sqValue = sqMaxValue;
            }
          }
          else if( sqMaxValue.GetType() == OT_FLOAT )
          {
            if( io_sqValue.Cast<float>() > sqMaxValue.Cast<float>() )
            {
              io_sqValue = sqMaxValue;
            }
          }
        }
      }
    }
  }
  else
  {
    std::string strInfo{ "Error: Unsupported property type " };
    strInfo += rumStringUtils::ToString( io_sqValue.GetType() );
    strInfo += " for property ";
    strInfo += pcPropertyAsset->GetName();
    strInfo += " (";
    strInfo += rumStringUtils::ToHexString( i_eAssetID );
    strInfo += ")";
    Logger::LogStandard( strInfo );
  }

  return bValid;
}
