#include <u_graphic.h>

#include <u_db.h>
#include <u_log.h>
#include <u_script.h>
#include <u_utility.h>

// Static initializations
Sqrat::Object rumGraphic::s_sqClass;
rumGraphic::GraphicHash rumGraphic::s_hashGraphics;
rumGraphic::GraphicAssetHash rumGraphic::s_hashGraphicAssets;


rumGraphic::~rumGraphic()
{
  Unmanage();
  rumAssert( !m_pcData );
}


// override
void rumGraphic::AllocateGameID( rumUniqueID i_uiGameID )
{
  rumAssert( INVALID_GAME_ID == i_uiGameID );

  static uint64_t s_uiAssetType{ ( rumUniqueID( rumGraphicAsset::GetClassRegistryID() ) ) << 60 };
  static uint64_t s_uiGameID{ ( rumUniqueID( Client_ObjectCreationType ) ) << 56 };
  SetGameID( ++s_uiGameID | s_uiAssetType );
}


uint32_t rumGraphic::AnimationAdvance( uint32_t i_uiFrame )
{
  m_cGraphicAttributes.m_uiAnimationFrame = i_uiFrame % GetNumAnimFrames();
  return m_cGraphicAttributes.m_uiAnimationFrame;
}


void rumGraphic::CalcAnimation()
{
  rumAssert( GetNumAnimStates() != 0 );
  rumAssert( GetNumAnimFrames() != 0 );

  m_uiFrameWidth = GetNumAnimStates() != 0 ? GetWidth() / GetNumAnimStates() : 0;
  m_uiFrameHeight = GetNumAnimFrames() != 0 ? GetHeight() / GetNumAnimFrames() : 0;
}


// static
rumGraphic* rumGraphic::Create()
{
  Sqrat::Object sqClass = Sqrat::RootTable().GetSlot( SCRIPT_GRAPHIC_NATIVE_CLASS );
  Sqrat::Table sqParams( Sqrat::DefaultVM::Get() );
  Sqrat::Object sqInstance = rumGameObject::Create( sqClass, sqParams );
  rumGraphic* pcGraphic{ sqInstance.Cast< rumGraphic* >() };
  if( pcGraphic )
  {
    pcGraphic->ScriptInstanceSet( sqInstance );
  }
  return pcGraphic;
}


// static
rumGraphic* rumGraphic::Fetch( const std::string& i_strName )
{
  // This is a reverse lookup, so walk the entire hash looking for a graphic matching the provided name
  for( const auto& iter : s_hashGraphics )
  {
    rumGraphic* pcGraphic{ iter.second };
    const std::string& strName{ pcGraphic->GetName() };
    if( strName.compare( i_strName ) == 0 )
    {
      return pcGraphic;
    }
    else
    {
      const std::string& strFilename{ pcGraphic->GetFilename() };
      if( strFilename.compare( i_strName ) == 0 )
      {
        return pcGraphic;
      }
    }
  }

  return nullptr;
}


// static
rumGraphic* rumGraphic::Fetch( rumAssetID i_eAssetID )
{
  const auto& iter{ s_hashGraphicAssets.find( i_eAssetID ) };
  return ( iter != s_hashGraphicAssets.end() ) ? iter->second : nullptr;
}


// static
rumGraphic* rumGraphic::Fetch( rumUniqueID i_uiGameID )
{
  const auto& iter{ s_hashGraphics.find( i_uiGameID ) };
  return ( iter != s_hashGraphics.end() ) ? iter->second : nullptr;
}


// static
Sqrat::Object rumGraphic::FetchScriptInstance( rumAssetID i_eAssetID )
{
  rumGraphic* pcGraphic{ Fetch( i_eAssetID ) };
  return pcGraphic ? pcGraphic->GetScriptInstance() : Sqrat::Object();
}


bool rumGraphic::InitData( const rumGraphic& i_rcGraphic )
{
  if( InitData( i_rcGraphic.GetWidth(), i_rcGraphic.GetHeight() ) )
  {
    // Copy the graphic to the new surface
    const rumPoint cZero;
    Blit( i_rcGraphic, cZero, cZero, i_rcGraphic.GetWidth(), i_rcGraphic.GetHeight() );
    return true;
  }

  return false;
}


// static
void rumGraphic::Init( bool i_bForceCreateAll )
{
  rumAssert( rumScript::GetCurrentVMType() != rumScript::VM_SERVER );

#pragma message ( "TODO - lazily load graphics, sounds, etc." )

  // Create a graphic for every asset
  const rumGraphicAsset::AssetHash& hashAssets{ rumGraphicAsset::GetAssetHash() };
  for( const auto& iter : hashAssets )
  {
    const rumGraphicAsset* pcAsset{ iter.second };
    if( i_bForceCreateAll || pcAsset->IsClientRendered() )
    {
      Sqrat::Object sqInstance{ rumGameObject::Create( pcAsset->GetAssetID() ) };
      rumAssert( sqInstance.GetType() == OT_INSTANCE );
      if( sqInstance.GetType() == OT_INSTANCE )
      {
        ManageScriptObject( sqInstance );
      }
    }
  }
}


// override
void rumGraphic::Manage()
{
  rumAssert( GetGameID() != INVALID_GAME_ID );
  if( GetGameID() != INVALID_GAME_ID )
  {
    s_hashGraphics.insert( { GetGameID(), this } );
  }

  if( GetAssetID() != INVALID_ASSET_ID )
  {
    s_hashGraphicAssets.insert( { GetAssetID(), this } );
  }
}


// override
void rumGraphic::Unmanage()
{
  s_hashGraphics.erase( GetGameID() );
  s_hashGraphicAssets.erase( GetAssetID() );
}


// static
void rumGraphic::OnAssetDataChanged( const rumGraphicAsset& i_rcAsset )
{
  for( const auto& iter : s_hashGraphicAssets )
  {
    rumGraphic* pcGraphic{ iter.second };
    rumAssert( pcGraphic );

    if( pcGraphic->GetAssetID() == i_rcAsset.GetAssetID() )
    {
      pcGraphic->InitData();
    }
  }
}


// override
void rumGraphic::OnCreated()
{
  if( m_pcAsset )
  {
    rumGraphicAssetAttributes cGraphicAttributes;
    m_pcAsset->GetAttributes( cGraphicAttributes );
    m_cAssetAttributes = cGraphicAttributes;
  }

  super::OnCreated();
}


// static
void rumGraphic::ScriptBind()
{
  rumAssert( rumScript::GetCurrentVMType() != rumScript::VM_SERVER );

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM ).Func( "rumGetGraphic", FetchScriptInstance );

  Sqrat::DerivedClass<rumGraphic, rumGameObject, Sqrat::NoConstructor<rumGraphic>>
    cGraphic( pcVM, SCRIPT_GRAPHIC_NATIVE_CLASS );
  cGraphic
    .Func( "GetAnimAspectRatio", &GetAnimAspectRatio )
    .Func( "GetAspectRatio", &GetAspectRatio )
    .Func( "GetAttributes", &GetAttributes )
    .Func( "SetAttributes", &SetAttributes )
    .Func( "GetHeight", &GetHeight )
    .Func( "GetWidth", &GetWidth )
    .Func( "GetFrameHeight", &GetFrameHeight )
    .Func( "GetFrameWidth", &GetFrameWidth )
    .Func( "GetNumAnimFrames", &GetNumAnimFrames )
    .Func( "GetScaledHeight", &GetScaledHeight )
    .Func( "GetScaledWidth", &GetScaledWidth )
    .Func( "GetScaledFrameHeight", &GetScaledFrameHeight )
    .Func( "GetScaledFrameWidth", &GetScaledFrameWidth )
    .Func( "Shift", &Shift )
    .Overload<uint32_t( rumGraphic::* )( void )>( "AnimationAdvance", &AnimationAdvance )
    .Overload<uint32_t( rumGraphic::* )( uint32_t )>( "AnimationAdvance", &AnimationAdvance )
    .Overload<void( rumGraphic::* )( const rumPoint& ) const>( "Draw", &Draw )
    .Overload<void( rumGraphic::* )( const rumPoint&, const rumPoint&, uint32_t, uint32_t ) const>( "Draw", &Draw )
    .Overload<void( rumGraphic::* )( const rumPoint& ) const>( "DrawAnimation", &DrawAnimation )
    .Overload<void( rumGraphic::* )( const rumPoint&, uint32_t, uint32_t ) const>( "DrawAnimation", &DrawAnimation );
  Sqrat::RootTable( pcVM ).Bind( "rumGraphicBase", cGraphic );

  rumGraphicAttributes::ScriptBind();
}


float rumGraphic::GetAnimAspectRatio() const
{
  float fRatio{ 0.f };

  if( m_uiFrameHeight >= m_uiFrameWidth )
  {
    if( m_uiFrameWidth != 0 )
    {
      fRatio = m_uiFrameHeight / static_cast<float>( m_uiFrameWidth );
    }
  }
  else if( m_uiFrameHeight != 0 )
  {
    fRatio = m_uiFrameWidth / static_cast<float>( m_uiFrameHeight );
  }

  return fRatio;
}


float rumGraphic::GetAspectRatio() const
{
  float fRatio{ 0.f };

  const uint32_t uiHeight{ GetHeight() };
  const uint32_t uiWidth{ GetWidth() };
  if( uiHeight >= uiWidth )
  {
    if( uiWidth != 0 )
    {
      fRatio = uiHeight / static_cast<float>( uiWidth );
    }
  }
  else if( uiHeight != 0 )
  {
    fRatio = uiWidth / static_cast<float>( uiHeight );
  }

  return fRatio;
}


// static
void rumGraphic::Shutdown()
{
  rumAssert( rumScript::GetCurrentVMType() != rumScript::VM_SERVER );

  // Clear the graphics hash and free memory
  if( !s_hashGraphics.empty() )
  {
    // Make a copy of the hash map for iteration and deletion. We can't delete directly from the original hash because
    // the iteration will be destroyed by calls to each object's Free method.
    GraphicHash cHash{ s_hashGraphics };
    s_hashGraphics.clear();

    for( const auto& iter : cHash )
    {
      RUM_COUT_IFDEF( MEMORY_DEBUG, "Freeing Graphic: " << iter.second->GetName() << '\n' );

      rumGraphic* pcObject{ iter.second };
      if( pcObject )
      {
        pcObject->Free();
      }
    }

    cHash.clear();
  }

  rumAssert( s_hashGraphicAssets.empty() );
}
