#include <u_animation.h>

#include <u_assert.h>
#include <u_script.h>


void rumAnimation::AdvanceFrame()
{
  if( m_uiNumFrames <= 1 )
  {
    // Nothing to animate
    return;
  }

  m_tLastUpdate.Restart();

  rumGraphic* pcGraphic{ rumGraphic::Fetch( m_eGraphicID ) };
  if( pcGraphic )
  {
    if( Random_AnimationType == m_eType )
    {
      // Choose a random frame
      m_uiFrame = pcGraphic->AnimationAdvance( rumNumberUtils::GetRandomUInt32() % GetNumAnimFrames() );
    }
    else if( Custom_AnimationType == m_eType )
    {
      HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
      const auto cPair{ rumScript::EvalOptionalFunc( GetScriptInstance(), "OnAnimate", 0, m_uiFrame ) };
      if( cPair.first )
      {
        m_uiFrame = pcGraphic->AnimationAdvance( cPair.second );
      }
    }
    else
    {
      // Advance a single frame
      m_uiFrame = pcGraphic->AnimationAdvance( m_uiFrame + 1 );
    }
  }
}


// static
void rumAnimation::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::ConstTable( pcVM )
    .Const( "rumStandardLoopingAnimation", StandardLooping_AnimationType )
    .Const( "rumStandardOnceAnimation", StandardOnce_AnimationType )
    .Const( "rumRandomAnimation", Random_AnimationType )
    .Const( "rumCustomAnimation", Custom_AnimationType );

  Sqrat::DerivedClass<rumAnimation, rumPawn, Sqrat::NoConstructor<rumAnimation>> cAnimation( pcVM, "rumAnimation" );
  cAnimation
    .Func( "IsAnimating", &IsAnimating )
    .Func( "ResetAnimation", &Reset )
    .Func( "StartAnimating", &Start )
    .Func( "StopAnimating", &Stop )
    .Func( "GetDrawOrder", &GetDrawOrder )
    .Func( "SetDrawOrder", &SetDrawOrder )
    .Func( "GetGraphic", &GetGraphicID )
    .Func( "SetGraphic", &SetGraphicID )
    .Func( "AdvanceAnimation", &AdvanceFrame )
    .Func( "GetAnimationType", &GetType )
    .Func( "SetAnimationType", &SetType )
    .Func( "GetAnimationFrame", &GetFrame )
    .Func( "SetAnimationFrame", &SetFrame )
    .Func( "GetAnimationInterval", &GetInterval )
    .Func( "SetAnimationInterval", &SetInterval )
    .Func( "GetNumAnimationFrames", &GetNumAnimFrames )
    .Func( "SetTransparencyLevel", &SetTransparencyLevel )
    .Func( "SetTransparencyLevelFromFloat", &SetTransparencyLevelFromFloat )
    .Func( "GetTransparencyLevel", &GetTransparencyLevel )
    .Func( "SetBlendColor", &SetBlendColor )
    .Func( "SetBufferColor", &SetBufferColor )
    .Func( "SetBlendType", &SetBlendType )
    .Func( "SetRestoreAlphaPostBlend", &SetRestoreAlphaPostBlend )
    .Func( "UseAnimationSet", &SetAnimSet )
    .Func( "GetAnimationSet", &GetAnimSet );
  Sqrat::RootTable( pcVM ).Bind( "rumAnimation", cAnimation );
}


void rumAnimation::SetGraphicID( rumAssetID i_eGraphicID )
{
  rumGraphic* pcGraphic{ rumGraphic::Fetch( i_eGraphicID ) };
  if( !pcGraphic )
  {
    // No graphic for this pawn - in some cases, this is expected
    m_eGraphicID = INVALID_ASSET_ID;
    m_uiNumFrames = 0;
    return;
  }

  rumGraphicAssetAttributes cGraphicAttributes;
  const auto* pcAsset{ pcGraphic->GetAsset() };
  if( pcAsset )
  {
    pcAsset->GetAttributes( cGraphicAttributes );
  }

  m_eGraphicID = i_eGraphicID;
  m_uiNumFrames = pcGraphic->GetNumAnimFrames();
  m_eType = cGraphicAttributes.GetAnimType();
  m_fInterval = cGraphicAttributes.GetAnimInterval();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };
  rumScript::ExecOptionalFunc( Sqrat::RootTable( pcVM ), "OnGraphicUpdated", GetScriptInstance(), i_eGraphicID );

  Reset();
  Start();
}


void rumAnimation::SetTransparencyLevel( uint32_t i_uiLevel )
{
  rumNumberUtils::Clamp<uint32_t>( i_uiLevel, RUM_ALPHA_TRANSPARENT, RUM_ALPHA_OPAQUE );
  m_uiTransparencyLevel = i_uiLevel;
}


void rumAnimation::Start()
{
  m_bAnimating = GetNumAnimFrames() > 1;
}


void rumAnimation::Stop()
{
  m_bAnimating = false;
}


// override
void rumAnimation::Tick()
{
  if( m_bAnimating && m_tLastUpdate.GetElapsedSeconds() > m_fInterval )
  {
    AdvanceFrame();
  }
}
