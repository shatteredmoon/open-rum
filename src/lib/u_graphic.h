#ifndef _U_GRAPHIC_H_
#define _U_GRAPHIC_H_

#include <platform.h>
#include <u_asset.h>
#include <u_graphic_asset.h>
#include <u_graphic_attributes.h>
#include <u_object.h>

#include <unordered_map>

#define SCRIPT_GRAPHIC_NATIVE_CLASS "rumGraphic"

//#define SCRIPT_GRAPHIC_CLASS    "rumGraphic"
//#define SCRIPT_GRAPHIC_FLIPPED  "rumFlipped"
//#define SCRIPT_GRAPHIC_MIRRORED "rumMirrored"


class rumGraphic : public rumGameObject
{
public:

  ~rumGraphic() override;

  static rumGraphic* Fetch( rumUniqueID i_uiGameID );
  static rumGraphic* Fetch( rumAssetID i_eAssetID );
  static rumGraphic* Fetch( const std::string& i_strName );
  static Sqrat::Object FetchScriptInstance( rumAssetID i_eAssetID );

  uint32_t AnimationAdvance()
  {
    return AnimationAdvance( m_cGraphicAttributes.m_uiAnimationFrame + 1 );
  }

  uint32_t AnimationAdvance( uint32_t i_uiFrame );

  virtual void Blit( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc, const rumPoint& i_rcDest,
                     uint32_t i_uiWidth, uint32_t i_uiHeight ) = 0;

  virtual void BlitAdd( const rumGraphic& i_rcGraphic, const rumPoint& i_rcPos,
                        const rumColor& i_rcColor = rumColor::s_cWhite ) = 0;

  virtual void BlitAlpha( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc, const rumPoint& i_rcDest,
                          uint32_t i_uiWidth, uint32_t i_uiHeight, bool i_bUseSrcColor ) const = 0;

  // Only blits pixels that are more opaque than the destination
  virtual void BlitAlphaPreserve( const rumGraphic& i_rcGraphic,
                                  const rumPoint& i_rcDest = rumPoint( 0, 0 ) ) const = 0;

  // Copies any pixel matching the current color mask from src graphic
  virtual void BlitColorMask( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc, const rumPoint& i_rcDest,
                              uint32_t i_uiWidth, uint32_t i_uiHeight ) = 0;

  virtual void BlitMultiply( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc, const rumPoint& i_rcDest,
                             uint32_t i_uiWidth, uint32_t i_uiHeight,
                             const rumColor& i_rcColor = rumColor::s_cWhite ) = 0;

  // This is mainly for testing various effects quickly
  virtual void BlitRandomEffect( const rumGraphic& i_rcGraphic, const rumPoint& i_rcPos,
                                 uint32_t i_uiWidth, uint32_t i_uiHeight ) const = 0;

  virtual void BlitStretch( rumGraphic& i_rcGraphic,
                            const rumPoint& i_rcSrc,
                            const rumPoint& i_rcDest,
                            uint32_t i_uiDestWidth,
                            uint32_t i_uiDestHeight ) = 0;

  virtual void BlitTransparent( const rumGraphic& i_rcGraphic, const rumPoint& i_rcPos, uint32_t i_iAlpha ) = 0;

  void CalcAnimation();

  virtual void Clear()
  {
    Clear( rumColor::s_cBlack );
  }

  virtual void Clear( const rumColor& i_rcGraphic ) = 0;

  void ClearRect( const rumRectangle& i_rcRect )
  {
    ClearRect( i_rcRect, rumColor::s_cBlack );
  }

  virtual void ClearRect( const rumRectangle& i_rcRect, const rumColor& i_rcColor ) = 0;

  // Copies the alpha from the entire src graphic to the calling graphic
  virtual void CopyAlpha( const rumGraphic& i_rcGraphic ) = 0;

  // Draws entire graphic to a point on the backbuffer
  virtual void Draw( const rumPoint& i_rcPos ) const = 0;

  // Draws part of a graphic to a point on the backbuffer
  virtual void Draw( const rumPoint& i_rcDest, const rumPoint& i_rcSrc,
                     uint32_t i_uiWidth, uint32_t i_uiHeight ) const = 0;

  // Convenience functions - these call the above Draw definitions

  // Draws graphic using draw prop animation information
  inline void DrawAnimation( const rumPoint& i_rcPos, uint32_t i_uiSetIndex, uint32_t i_uiFrameIndex ) const
  {
    Draw( i_rcPos, rumPoint( i_uiSetIndex * GetFrameWidth(), i_uiFrameIndex * GetFrameHeight() ),
          GetFrameWidth(), GetFrameHeight() );
  }

  inline void DrawAnimation( const rumPoint& i_rcPos ) const
  {
    DrawAnimation( i_rcPos, m_cGraphicAttributes.m_uiAnimationSet, m_cGraphicAttributes.m_uiAnimationFrame );
  }

  virtual void DrawCircle( const rumPoint& i_rcPos, int32_t i_iRadius, const rumColor& i_rcColor ) = 0;
  virtual void DrawLine( const rumPoint& i_rcPos1, const rumPoint& i_rcPos2, const rumColor& i_rcColor ) = 0;
  virtual void DrawRect( const rumRectangle& i_rcRect, const rumColor& i_rcColor ) = 0;
  virtual void DrawRectUnfilled( const rumRectangle& i_rcRect, const rumColor& i_rcColor ) = 0;
  virtual void DrawTriangle( const rumPoint& i_rcPos1, const rumPoint& i_rcPos2, const rumPoint& i_rcPos3,
                             const rumColor& i_rcColor ) = 0;

  virtual void Flip() = 0;
  virtual void FlipMirror() = 0;
  virtual void Mirror() = 0;

  virtual void GenerateAlpha() = 0;

  // TODO - this should be private
  void* GetData() const
  {
    return m_pcData;
  }

  virtual bool HasAlpha() const = 0;

  bool HasData() const
  {
    return m_pcData != nullptr;
  }

  float GetAnimAspectRatio() const;
  float GetAspectRatio() const;

  // Returns a copy
  rumGraphicAttributes GetAttributes() const
  {
    return m_cGraphicAttributes;
  }

  void SetAttributes( const rumGraphicAttributes& i_rcAttributes )
  {
    m_cGraphicAttributes = i_rcAttributes;
  }

  uint32_t GetFrameHeight() const
  {
    return m_uiFrameHeight;
  }

  uint32_t GetFrameWidth() const
  {
    return m_uiFrameWidth;
  }

  virtual uint32_t GetHeight() const = 0;
  virtual uint32_t GetWidth() const = 0;

  uint32_t GetNumAnimFrames() const
  {
    return ( m_cAssetAttributes.GetNumAnimFrames() );
  }

  uint32_t GetNumAnimStates() const
  {
    return ( m_cAssetAttributes.GetNumAnimStates() );
  }

  uint32_t GetScaledHeight() const
  {
    return static_cast<uint32_t>( m_cGraphicAttributes.m_fVerticalScale * GetHeight() );
  }

  uint32_t GetScaledWidth() const
  {
    return static_cast<uint32_t>( m_cGraphicAttributes.m_fHorizontalScale * GetWidth() );
  }

  uint32_t GetScaledFrameHeight() const
  {
    return static_cast<uint32_t>( m_cGraphicAttributes.m_fVerticalScale * GetFrameHeight() );
  }

  uint32_t GetScaledFrameWidth() const
  {
    return static_cast<uint32_t>( m_cGraphicAttributes.m_fHorizontalScale * GetFrameWidth() );
  }

  void SetMasked( bool m_bMasked )
  {
    m_cGraphicAttributes.m_bDrawMasked = m_bMasked;
  }

  uint32_t GetTransparentLevel() const
  {
    return m_cGraphicAttributes.m_uiTransparentLevel;
  }

  void SetTransparentLevel( uint32_t i_iLevel )
  {
    rumAssert( i_iLevel <= 255 );
    m_cGraphicAttributes.m_uiTransparentLevel = i_iLevel;
  }

  // Init data from another graphic
  virtual bool InitData( const rumGraphic& i_rcGraphic );

  // Init data with an empty width and height
  virtual bool InitData( uint32_t i_uiWidth, uint32_t i_uiHeight ) = 0;

  // Init data from asset
  virtual bool InitData() = 0;

  // Init data from raw pixel information
  virtual bool InitData( const rumColor* i_pcImage, uint32_t i_uiWidth, uint32_t i_uiHeight ) = 0;

  virtual void Shift( const rumVector& i_rcVector ) = 0;

  static rumGraphic* Create();

  static void Init( bool i_bForceCreateAll );

  static void OnAssetDataChanged( const rumGraphicAsset& i_rcAsset );

  static void ScriptBind();
  static void Shutdown();

protected:

  void AllocateGameID( rumUniqueID i_uiGameID = INVALID_GAME_ID ) override;

  void Manage() override;
  void Unmanage() override;

  void OnCreated() override;

  void SetData( void* i_pcData )
  {
    m_pcData = i_pcData;
  }

  // TODO - these should be private
  void* m_pcData{ nullptr };

  rumGraphicAttributes m_cGraphicAttributes;

protected:

  typedef std::unordered_map< rumUniqueID, rumGraphic* > GraphicHash;
  static GraphicHash s_hashGraphics;

  typedef std::unordered_map< rumAssetID, rumGraphic* > GraphicAssetHash;
  static GraphicAssetHash s_hashGraphicAssets;

  // Height of a single animation frame in pixels
  uint32_t m_uiFrameHeight{ 0 };

  // Width of a single animation frame in pixels
  uint32_t m_uiFrameWidth{ 0 };

  rumGraphicAssetAttributes m_cAssetAttributes;

private:

  static Sqrat::Object s_sqClass;

  using super = rumGameObject;
};

#endif // _U_GRAPHIC_H_
