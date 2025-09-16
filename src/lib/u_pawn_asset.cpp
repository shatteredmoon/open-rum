#include <u_pawn_asset.h>

#include <u_creature_asset.h>
#include <u_portal_asset.h>
#include <u_widget_asset.h>


void rumPawnAsset::ExportCSVFile( std::ofstream& o_rcOutfile ) const
{
  super::ExportCSVFile( o_rcOutfile );

  o_rcOutfile << ',' << rumStringUtils::ToHexString( GetGraphicID() );
  o_rcOutfile << ',' << rumStringUtils::ToHexString( GetMoveType() );
  o_rcOutfile << ',' << ( GetBlocksLOS() ? '1' : '0' );
  o_rcOutfile << ',' << rumStringUtils::ToHexString( GetCollisionFlags() );
  o_rcOutfile << ',' << GetLightRange();
  o_rcOutfile << ',' << GetDrawOrder();
  o_rcOutfile << ',' << GetServiceType();
}


// override
void rumPawnAsset::ExportDBTable( std::string& io_strQuery ) const
{
  super::ExportDBTable( io_strQuery );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToHexString( GetGraphicID() );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToHexString( GetMoveType() );

  io_strQuery += ',';
  io_strQuery += ( GetBlocksLOS() ? '1' : '0' );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToHexString( GetCollisionFlags() );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToString( GetLightRange() );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToFloatString( GetDrawOrder() );

  io_strQuery += ',';
  io_strQuery += rumStringUtils::ToString( GetServiceType() );
}


// static
rumPawnAsset* rumPawnAsset::Fetch( rumAssetID i_eAssetID )
{
  if( INVALID_ASSET_ID != i_eAssetID )
  {
    const AssetType eAssetType{ (AssetType)RAW_ASSET_TYPE( i_eAssetID ) };
    rumAssert( eAssetType < rumPawn::NumPawnTypes );

    switch( eAssetType )
    {
      case rumPawn::Creature_PawnType:
        return rumCreatureAsset::Fetch( i_eAssetID );

      case rumPawn::Portal_PawnType:
        return rumPortalAsset::Fetch( i_eAssetID );

      case rumPawn::Widget_PawnType:
        return rumWidgetAsset::Fetch( i_eAssetID );
    }
  }

  return nullptr;
}


// static
int32_t rumPawnAsset::Init( const std::string& i_strPath )
{
  int32_t eResult{ rumCreatureAsset::Init( i_strPath ) };
  eResult |= rumPortalAsset::Init( i_strPath );
  eResult |= rumWidgetAsset::Init( i_strPath );

  return eResult;
}


void rumPawnAsset::OnCreated( const std::vector<std::string>& i_rvFields )
{
  if( !i_rvFields.empty() )
  {
    enum { COL_ID, COL_NAME, COL_BASECLASS, COL_GRAPHIC_ID, COL_MOVE_FLAGS, COL_BLOCKS_LOS, COL_COLLISION_FLAGS,
           COL_LIGHT_RANGE, COL_RENDER_PRIORITY, COL_SERVICE_TYPE };

    m_eGraphicID = (rumAssetID)rumStringUtils::ToUInt( i_rvFields.at( COL_GRAPHIC_ID ) );
    m_uiCollisionFlags = rumStringUtils::ToUInt( i_rvFields.at( COL_COLLISION_FLAGS ) );
    m_uiMoveType = rumStringUtils::ToUInt( i_rvFields.at( COL_MOVE_FLAGS ) );
    m_bBlocksLOS = rumStringUtils::ToBool( i_rvFields.at( COL_BLOCKS_LOS ) );
    m_uiLightRange = rumStringUtils::ToUInt( i_rvFields.at( COL_LIGHT_RANGE ) );
    m_fDrawOrder = strtof( i_rvFields.at( COL_RENDER_PRIORITY ).c_str(), nullptr );
    m_eServiceType = (ServiceType)rumStringUtils::ToInt( i_rvFields.at( COL_SERVICE_TYPE ) );
  }
}


// static
void rumPawnAsset::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumPawnAsset, rumAsset, Sqrat::NoConstructor<rumPawnAsset> >
    cPawnAsset( pcVM, SCRIPT_PAWN_ASSET_NATIVE_CLASS );
  cPawnAsset.Func( "GetGraphicID", &GetGraphicID );
  Sqrat::RootTable( pcVM ).Bind( SCRIPT_PAWN_ASSET_NATIVE_CLASS, cPawnAsset );

  rumCreatureAsset::ScriptBind();
  rumPortalAsset::ScriptBind();
  rumWidgetAsset::ScriptBind();
}


// static
void rumPawnAsset::Shutdown()
{
  rumCreatureAsset::Shutdown();
  rumPortalAsset::Shutdown();
  rumWidgetAsset::Shutdown();
}
