#pragma once

#include <u_enum.h>
#include <u_graphic_attributes.h>
#include <u_graphic.h>
#include <u_pawn.h>
#include <u_utility.h>
#include <u_timer.h>

// This is a mix-in class to give select pawn child classes animation
class rumAnimation : public rumPawn
{
public:

  // For manual control of animation frames
  void AdvanceFrame();

  void Reset()
  {
    SetFrame( 0 );
  }

  void Start();

  void Stop();

  bool IsAnimating() const
  {
    return m_bAnimating;
  }

  rumAnimationType GetType() const
  {
    return m_eType;
  }

  void SetType( rumAnimationType i_eType )
  {
    m_eType = i_eType;
  }

  uint32_t GetFrame() const
  {
    return m_uiFrame;
  }

  void SetFrame( uint32_t i_uiFrame )
  {
    m_uiFrame = i_uiFrame;
    m_tLastUpdate.Restart();
  }

  float GetInterval() const
  {
    return m_fInterval;
  }

  void SetInterval( float i_fInterval )
  {
    if( i_fInterval < 0.f )
    {
      i_fInterval = 0.f;
    }

    m_fInterval = i_fInterval;
  }

  uint32_t GetAnimSet() const
  {
    return m_eSet;
  }

  void SetAnimSet( uint32_t i_eSet )
  {
    Reset();
    m_eSet = i_eSet;
  }

  float GetDrawOrder() const override
  {
    return m_fDrawOrder;
  }

  void SetDrawOrder( float i_fDrawOrder )
  {
    m_fDrawOrder = i_fDrawOrder;
  }

  rumAssetID GetGraphicID() const
  {
    return m_eGraphicID;
  }

  void SetGraphicID( rumAssetID i_eGraphicID );

  uint32_t GetNumAnimFrames() const
  {
    return m_uiNumFrames;
  }

  uint32_t GetTransparencyLevel() const
  {
    return m_uiTransparencyLevel;
  }

  void SetTransparencyLevel( uint32_t i_uiTransparencyLevel );

  void SetTransparencyLevelFromFloat( float i_fTransparencyLevel )
  {
    SetTransparencyLevel( static_cast<uint32_t>(i_fTransparencyLevel * RUM_ALPHA_OPAQUE ) );
  }

  rumGraphicAttributes::BlendType GetBlendType() const
  {
    return m_eBlendType;
  }

  void SetBlendType( int32_t eBlendType )
  {
    m_eBlendType = ( rumGraphicAttributes::BlendType )eBlendType;
  }

  rumColor GetBlendColor() const
  {
    return m_cBlendColor;
  }

  void SetBlendColor( const rumColor& rcColor )
  {
    m_cBlendColor = rcColor;
  }

  rumColor GetBufferColor() const
  {
    return m_cBufferColor;
  }

  void SetBufferColor( const rumColor& rcColor )
  {
    m_cBufferColor = rcColor;
  }

  bool GetRestoreAlphaPostBlend() const
  {
    return m_bRestoreAlphaPostBlend;
  }

  void SetRestoreAlphaPostBlend( bool bRestore )
  {
    m_bRestoreAlphaPostBlend = bRestore;
  }

  void Tick() override;

  static void ScriptBind();

private:

  // The pawn's current graphic
  rumAssetID m_eGraphicID{ INVALID_ASSET_ID };

  // The current animation frame offset
  uint32_t m_uiFrame{ 0 };

  // The number of vertical animation frames in a graphic
  uint32_t m_uiNumFrames{ 0 };

  // The current animation set offset
  uint32_t m_eSet{ 0 };

  // Transparency level as a percentage, opaque 0 to 100 invisible
  uint32_t m_uiTransparencyLevel{ RUM_ALPHA_OPAQUE };

  // Blend the animation with a solid color specified by m_cColor
  rumGraphicAttributes::BlendType m_eBlendType{ rumGraphicAttributes::BlendType_None };

  // The color that temporary buffers are cleared to for blending operations. This value is only utilized when
  // m_eBlendType is set to anything other than BlendType_None.
  rumColor m_cBufferColor{ rumColor::s_cBlackOpaque };

  // The color setting used by blend actions. This value is only utilized when m_eBlendType is set to anything other
  // than BlendType_None.
  rumColor m_cBlendColor{ rumColor::s_cWhiteOpaque };

  // The last time a frame was advanced
  rumTimer m_tLastUpdate;

  // Defines how frames advance
  rumAnimationType m_eType{ StandardLooping_AnimationType };

  // The amount of time between animation frames
  float m_fInterval{ 0.5f };

  // The lower the number, the closer the object is to the viewer
  float m_fDrawOrder{ 0.0f };

  // Restores the original images alpha values after a blend operation. This value is only utilized when m_eBlendType
  // is set to anything other than BlendType_None.
  bool m_bRestoreAlphaPostBlend{ false };

  // When true, the animation will automatically advance to another frame on the set interval.
  bool m_bAnimating{ true };

  typedef rumPawn super;
};
