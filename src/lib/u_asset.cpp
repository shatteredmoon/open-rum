#include <u_asset.h>

#include <u_broadcast_asset.h>
#include <u_creature_asset.h>
#include <u_custom_asset.h>
#include <u_graphic_asset.h>
#include <u_inventory_asset.h>
#include <u_log.h>
#include <u_map_asset.h>
#include <u_pawn_asset.h>
#include <u_portal_asset.h>
#include <u_property_asset.h>
#include <u_sound_asset.h>
#include <u_tile_asset.h>
#include <u_widget_asset.h>


rumAsset::~rumAsset()
{
  RUM_COUT_IFDEF( MEMORY_DEBUG,
                  "Freeing asset " << m_strName << " (" << rumStringUtils::ToHexString( m_eAssetID ) << ")\n" );
}


// static
void rumAsset::BindAssetID( rumAssetID i_eAssetID, const std::string& i_strName, const std::string& i_strTypeSuffix )
{
  rumAssert( i_eAssetID != INVALID_ASSET_ID );
  if( INVALID_ASSET_ID == i_eAssetID )
  {
    return;
  }

  const std::string strName{ i_strName + i_strTypeSuffix + ASSET_ID_SUFFIX };

  // Expose the asset ID to scripts
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
  Sqrat::ConstTable( pcVM ).Const( strName.c_str(), i_eAssetID );
}


void rumAsset::ExportCSVFile( std::ofstream& o_rcOutfile ) const
{
  // Asset ID
  o_rcOutfile << rumStringUtils::ToHexString( GetAssetID() );

  // Name
  o_rcOutfile << ',' << GetName();

  // Base class override
  o_rcOutfile << ',' << GetBaseClassOverride();
}


void rumAsset::ExportCSVPropertyFile( std::ofstream& o_rcOutfile ) const
{
  // Sort the hash
  using SortedHash = std::map< rumAssetID, Sqrat::Object >;
  const SortedHash hashProperties( m_hashProperties.begin(), m_hashProperties.end() );

  // Export csv values
  for( const auto& iter : hashProperties )
  {
    const rumAssetID ePropertyID{ iter.first };
    Sqrat::Object sqValue{ iter.second };

    const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( ePropertyID ) };
    rumAssert( pcProperty );
    if( pcProperty )
    {
      // Asset ID
      o_rcOutfile << rumStringUtils::ToHexString( GetAssetID() );

      // Property ID
      o_rcOutfile << ',' << rumStringUtils::ToHexString( ePropertyID );

      // Property value
      if( pcProperty->IsAssetRef() || ( pcProperty->GetValueType() == PropertyValueType::StringToken ) )
      {
        o_rcOutfile << ',' << rumStringUtils::ToHexString( sqValue.Cast<uint32_t>() );
      }
      else
      {
        o_rcOutfile << ',' << rumScript::ToSerializationString( sqValue );
      }

      // Done
      o_rcOutfile << '\n';
    }
  }
}


// virtual
void rumAsset::ExportDBTable( std::string& io_strQuery ) const
{
  io_strQuery += rumStringUtils::ToHexString( GetAssetID() );
  io_strQuery += ",'" + GetName() + '\'';

  const std::string& strBaseClass{ GetBaseClassOverride() };
  if( strBaseClass.empty() )
  {
    io_strQuery += ",null";
  }
  else
  {
    io_strQuery += ",'" + GetBaseClassOverride() + '\'';
  }
}


void rumAsset::ExportDBPropertyTable( std::string& io_strQuery, const std::string& i_strStorageName,
                                      ServiceType i_eServiceType ) const
{
  const auto& rcProperties{ GetProperties() };
  for( const auto iter : rcProperties )
  {
    const rumAssetID ePropertyID{ iter.first };

    const auto* pcProperty{ rumPropertyAsset::Fetch( ePropertyID ) };
    if( nullptr == pcProperty )
    {
      continue;
    }

    // Don't export the property to service types that don't match
    if( ( pcProperty->GetServiceType() == i_eServiceType ) || ( pcProperty->GetServiceType() == Shared_ServiceType ) )
    {
      Sqrat::Object sqValue{ iter.second };

      io_strQuery += "INSERT INTO ";
      io_strQuery += i_strStorageName;
      io_strQuery += "_properties (";
      io_strQuery += i_strStorageName;
      io_strQuery += "_id_fk,property_id_fk,value) VALUES (";
      io_strQuery += rumStringUtils::ToHexString( GetAssetID() );
      io_strQuery += ",";
      io_strQuery += rumStringUtils::ToHexString( ePropertyID );
      io_strQuery += ",";
      if( sqValue.GetType() == OT_STRING )
      {
        io_strQuery += "'";
        io_strQuery += rumScript::ToSerializationString( sqValue );
        io_strQuery += "'";
      }
      else
      {
        io_strQuery += rumScript::ToSerializationString( sqValue );
      }
      io_strQuery += ");";
    }
  }
}


// static
rumAsset* rumAsset::Fetch( rumAssetID i_eAssetID )
{
  rumAsset* pcAsset{ nullptr };

  const AssetType eAssetType{ RAW_ASSET_TYPE( i_eAssetID ) };
  switch( eAssetType )
  {
    case Creature_AssetType:
    case Portal_AssetType:
    case Widget_AssetType:    pcAsset = rumPawnAsset::Fetch( i_eAssetID );      break;
    case Broadcast_AssetType: pcAsset = rumBroadcastAsset::Fetch( i_eAssetID ); break;
    case Tile_AssetType:      pcAsset = rumTileAsset::Fetch( i_eAssetID );      break;
    case Map_AssetType:       pcAsset = rumMapAsset::Fetch( i_eAssetID );       break;
    case Graphic_AssetType:   pcAsset = rumGraphicAsset::Fetch( i_eAssetID );   break;
    case Sound_AssetType:     pcAsset = rumSoundAsset::Fetch( i_eAssetID );     break;
    case Property_AssetType:  pcAsset = rumPropertyAsset::Fetch( i_eAssetID );  break;
    case Inventory_AssetType: pcAsset = rumInventoryAsset::Fetch( i_eAssetID ); break;
    case Custom_AssetType:    pcAsset = rumCustomAsset::Fetch( i_eAssetID );    break;
  }

  return pcAsset;
}


// static
rumAsset* rumAsset::FetchByName( const std::string& i_strName )
{
  rumAssetID eAssetID{ GetAssetIDFromConstTable( i_strName ) };
  if( INVALID_ASSET_ID != eAssetID )
  {
    return Fetch( eAssetID );
  }
  else
  {
    // Try each type one at a time. Yes, this is as expensive as it sounds.

    eAssetID = GetAssetIDFromConstTable( i_strName + rumPropertyAsset::GetAssetTypeSuffix() + ASSET_ID_SUFFIX );
    if( INVALID_ASSET_ID != eAssetID )
    {
      return Fetch( eAssetID );
    }

    eAssetID = GetAssetIDFromConstTable( i_strName + rumCreatureAsset::GetAssetTypeSuffix() + ASSET_ID_SUFFIX );
    if( INVALID_ASSET_ID != eAssetID )
    {
      return Fetch( eAssetID );
    }

    eAssetID = GetAssetIDFromConstTable( i_strName + rumPortalAsset::GetAssetTypeSuffix() + ASSET_ID_SUFFIX );
    if( INVALID_ASSET_ID != eAssetID )
    {
      return Fetch( eAssetID );
    }

    eAssetID = GetAssetIDFromConstTable( i_strName + rumWidgetAsset::GetAssetTypeSuffix() + ASSET_ID_SUFFIX );
    if( INVALID_ASSET_ID != eAssetID )
    {
      return Fetch( eAssetID );
    }

    eAssetID = GetAssetIDFromConstTable( i_strName + rumBroadcastAsset::GetAssetTypeSuffix() + ASSET_ID_SUFFIX );
    if( INVALID_ASSET_ID != eAssetID )
    {
      return Fetch( eAssetID );
    }

    eAssetID = GetAssetIDFromConstTable( i_strName + rumTileAsset::GetAssetTypeSuffix() + ASSET_ID_SUFFIX );
    if( INVALID_ASSET_ID != eAssetID )
    {
      return Fetch( eAssetID );
    }

    eAssetID = GetAssetIDFromConstTable( i_strName + rumMapAsset::GetAssetTypeSuffix() + ASSET_ID_SUFFIX );
    if( INVALID_ASSET_ID != eAssetID )
    {
      return Fetch( eAssetID );
    }

    eAssetID = GetAssetIDFromConstTable( i_strName + rumGraphicAsset::GetAssetTypeSuffix() + ASSET_ID_SUFFIX );
    if( INVALID_ASSET_ID != eAssetID )
    {
      return Fetch( eAssetID );
    }

    eAssetID = GetAssetIDFromConstTable( i_strName + rumSoundAsset::GetAssetTypeSuffix() + ASSET_ID_SUFFIX );
    if( INVALID_ASSET_ID != eAssetID )
    {
      return Fetch( eAssetID );
    }

    eAssetID = GetAssetIDFromConstTable( i_strName + rumInventoryAsset::GetAssetTypeSuffix() + ASSET_ID_SUFFIX );
    if( INVALID_ASSET_ID != eAssetID )
    {
      return Fetch( eAssetID );
    }

    eAssetID = GetAssetIDFromConstTable( i_strName + rumCustomAsset::GetAssetTypeSuffix() + ASSET_ID_SUFFIX );
    if( INVALID_ASSET_ID != eAssetID )
    {
      return Fetch( eAssetID );
    }
  }

  return nullptr;
}


// static
rumAssetID rumAsset::GetAssetIDFromConstTable( const std::string& i_strPath )
{
  auto sqObject{ rumScript::GetConstTableEntry( i_strPath ) };
  if( sqObject.GetType() == OT_INTEGER )
  {
    return sqObject.Cast<rumAssetID>();
  }

  return INVALID_ASSET_ID;
}


// static
int32_t rumAsset::Init( const std::string& i_strPath )
{
  int32_t eResult = RESULT_SUCCESS;

  // Properties must be handled first!
  eResult |= rumPropertyAsset::Init( i_strPath );

  // Now, the rest...
  eResult |= rumBroadcastAsset::Init( i_strPath );
  eResult |= rumCustomAsset::Init( i_strPath );
  eResult |= rumInventoryAsset::Init( i_strPath );
  eResult |= rumMapAsset::Init( i_strPath );
  eResult |= rumPawnAsset::Init( i_strPath );
  eResult |= rumTileAsset::Init( i_strPath );

  if( rumScript::GetCurrentVMType() != rumScript::VM_SERVER )
  {
    eResult |= rumGraphicAsset::Init( i_strPath );
    eResult |= rumSoundAsset::Init( i_strPath );
  }

  return eResult;
}


// static
void rumAsset::RegisterClasses()
{
  // This should only be called after the startup scripts have been executed
  rumAssert( rumScript::StartupScriptExecuted() );

  RegisterClasses<rumBroadcastAsset>();
  RegisterClasses<rumCreatureAsset>();
  RegisterClasses<rumCustomAsset>();
  RegisterClasses<rumInventoryAsset>();
  RegisterClasses<rumMapAsset>();
  RegisterClasses<rumPortalAsset>();
  RegisterClasses<rumPropertyAsset>();
  RegisterClasses<rumTileAsset>();
  RegisterClasses<rumWidgetAsset>();

  if( rumScript::GetCurrentVMType() != rumScript::VM_SERVER )
  {
    RegisterClasses<rumGraphicAsset>();
    RegisterClasses<rumSoundAsset>();
  }
}


// static
void rumAsset::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM )
    .Func( "rumGetAsset", Fetch )
    .Func( "rumGetAssetByName", FetchByName )
    .Func( "rumGetAssetID", GetAssetIDFromConstTable );

  Sqrat::DerivedClass<rumAsset, rumPropertyContainer, Sqrat::NoConstructor<rumAsset>> cAsset( pcVM, "rumAsset" );
  cAsset
    .Func( "GetAssetID", &GetAssetID )
    .Func( "GetAssetType", &GetAssetType )
    .Func( "GetName", &GetName )
    .Func( "ToString", &ToString )
    .Overload<Sqrat::Object( rumAsset::* )( rumAssetID, Sqrat::Object )>( "GetProperty", &GetProperty )
    .Overload<Sqrat::Object( rumAsset::* )( rumAssetID )>( "GetProperty", &GetProperty );
  Sqrat::RootTable( pcVM ).Bind( "rumAsset", cAsset );

  rumFileAsset::ScriptBind();

  // Bind specific asset types
  rumPropertyAsset::ScriptBind();
  rumBroadcastAsset::ScriptBind();
  rumCustomAsset::ScriptBind();
  rumInventoryAsset::ScriptBind();
  rumMapAsset::ScriptBind();
  rumPawnAsset::ScriptBind();
  rumTileAsset::ScriptBind();

  if( rumScript::GetCurrentVMType() != rumScript::VM_SERVER )
  {
    rumGraphicAsset::ScriptBind();
    rumSoundAsset::ScriptBind();
  }
}


// static
void rumAsset::Shutdown()
{
  rumBroadcastAsset::Shutdown();
  rumCustomAsset::Shutdown();
  rumInventoryAsset::Shutdown();
  rumMapAsset::Shutdown();
  rumPawnAsset::Shutdown();
  rumTileAsset::Shutdown();
  rumPropertyAsset::Shutdown();

  if( rumScript::GetCurrentVMType() != rumScript::VM_SERVER )
  {
    rumGraphicAsset::Shutdown();
    rumSoundAsset::Shutdown();
  }
}


std::string rumAsset::ToString() const
{
  std::string strDesc{ "Asset: " + GetName() };

  if( m_hashProperties.empty() )
  {
    return strDesc;
  }

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  strDesc += "\n\nAsset Properties:\n\n";

  for( const auto& iter : m_hashProperties )
  {
    const rumAssetID ePropertyID{ iter.first };
    const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( ePropertyID ) };
    rumAssert( pcProperty );
    if( pcProperty )
    {
      strDesc += "  " + pcProperty->GetName() + " = ";

      std::string strValue;
      if( pcProperty->IsAssetRef() )
      {
        strValue = rumStringUtils::ToHexString( iter.second.Cast<rumAssetID>() );
      }
      else
      {
        strValue = rumScript::ToString( iter.second );
      }

      const auto cPair{ rumScript::EvalOptionalFunc( Sqrat::RootTable( pcVM ), "OnPropertyDesc", std::string{},
                                                     pcProperty, iter.second ) };
      if( !cPair.second.empty() )
      {
        strDesc += cPair.second + " (" + strValue + ") ";
      }
      else
      {
        strDesc += strValue + " ";
      }

      strDesc += "<" + rumScript::GetObjectTypeName( iter.second ) + ">\n";
    }
  }

  return strDesc;
}


void rumFileAsset::ExportCSVFile( std::ofstream& o_rcOutfile ) const
{
  super::ExportCSVFile( o_rcOutfile );

  // Data filename
  o_rcOutfile << ',' << GetFilename();
}


void rumFileAsset::ExportDBTable( std::string& io_strQuery ) const
{
  super::ExportDBTable( io_strQuery );

  io_strQuery += ",'" + GetFilename() + '\'';
}


void rumAsset::ParseProperty( const std::string& i_strID, const std::string& i_strValue )
{
  Sqrat::Object sqValue;

  const rumAssetID ePropertyID{ (rumAssetID)rumStringUtils::ToUInt( i_strID ) };
  rumAssert( ePropertyID != INVALID_ASSET_ID );
  const rumPropertyAsset* pcProperty{ rumPropertyAsset::Fetch( ePropertyID ) };
  if( pcProperty )
  {
    if( pcProperty->IsAssetRef() )
    {
      rumScript::SetValue( sqValue, rumStringUtils::ToUInt( i_strValue ) );
    }
    else
    {
      switch( pcProperty->GetValueType() )
      {
        case PropertyValueType::Bitfield:
        case PropertyValueType::Integer:
        case PropertyValueType::StringToken:
          rumScript::SetValue( sqValue, rumStringUtils::ToUInt( i_strValue ) );
          break;

        case PropertyValueType::Bool:
          rumScript::SetValue( sqValue, rumStringUtils::ToBool( i_strValue ) );
          break;

        case PropertyValueType::Float:
          rumScript::SetValue( sqValue, strtof( i_strValue.c_str(), nullptr ) );
          break;

        case PropertyValueType::String:
          rumScript::SetValue( sqValue, i_strValue );
          break;

        default:
          std::string strError{ "Error: GetValueType could not handle type " };
          strError += rumStringUtils::ToString( rumUtility::ToUnderlyingType( pcProperty->GetValueType() ) );
          Logger::LogStandard( strError, Logger::LOG_ERROR );
          break;
      }
    }
  }
  else
  {
    std::string strWarning{ "Failed to fetch property id [" };
    strWarning += rumStringUtils::ToHexString( ePropertyID );
    strWarning += "] for asset ";
    strWarning += GetName();
    Logger::LogStandard( strWarning, Logger::LOG_WARNING );
  }

  SetProperty( ePropertyID, sqValue );
}


// static
void rumFileAsset::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumFileAsset, rumAsset, Sqrat::NoConstructor<rumAsset> > cFileAsset( pcVM, "rumFileAsset" );
  Sqrat::RootTable( pcVM ).Bind( "rumFileAsset", cFileAsset );
}


// virtual
void rumFileAsset::SetData( const rumByte* i_pcData, uint32_t i_uiNumBytes )
{
  SAFE_ARRAY_DELETE( m_pcData );

  m_uiAllocationSize = i_uiNumBytes;
  m_pcData = new rumByte[i_uiNumBytes];
  memcpy( m_pcData, i_pcData, sizeof( rumByte ) * i_uiNumBytes );
}
