#ifndef _E_GRAPHICS_H_
#define _E_GRAPHICS_H_

#include <u_graphic.h>
#include <u_structs.h>

struct rumConfig;
class QString;


class EditorGraphicBase : public rumGraphic
{
public:

  void Blit( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc, const rumPoint& i_rcDest,
             uint32_t i_uiWidth, uint32_t i_uiHeight ) override
  {
    rumAssert( false );
  }

  void BlitAdd( const rumGraphic& i_rcGraphic, const rumPoint& i_rcPos, const rumColor& i_rcColor ) override
  {
    rumAssert( false );
  }

  void BlitAlpha( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc, const rumPoint& i_rcDest,
                  uint32_t i_uiWidth, uint32_t i_uiHeight, bool i_bUseSrcColor ) const override
  {
    rumAssert( false );
  }

  void BlitAlphaPreserve( const rumGraphic& i_rcGraphic, const rumPoint& i_rcDest ) const override
  {
    rumAssert( false );
  }

  void BlitColorMask( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc, const rumPoint& i_rcDest,
                      uint32_t i_uiWidth, uint32_t i_uiHeight ) override
  {
    rumAssert( false );
  }

  void BlitMultiply( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc, const rumPoint& i_rcDest,
                     uint32_t i_uiWidth, uint32_t i_uiHeight, const rumColor& i_rcColor ) override
  {
    rumAssert( false );
  }

  void BlitRandomEffect( const rumGraphic& i_rcGraphic, const rumPoint& i_rcPos,
                         uint32_t i_uiWidth, uint32_t i_uiHeight ) const override
  {
    rumAssert( false );
  }

  void BlitStretch( rumGraphic& i_rcGraphic,
                    const rumPoint& i_rcSrc,
                    const rumPoint& i_rcDest,
                    uint32_t i_uiDestWidth,
                    uint32_t i_uiDestHeight ) override
  {
    rumAssert( false );
  }

  void BlitTransparent( const rumGraphic& i_rcGraphic, const rumPoint& i_rcPos, uint32_t i_iAlpha ) override
  {
    rumAssert( false );
  }

  void Clear( const rumColor& i_rcColor ) override
  {
    rumAssert( false );
  }

  void ClearRect( const rumRectangle& i_rcRect, const rumColor& i_rcColor ) override
  {
    rumAssert( false );
  }

  void CopyAlpha( const rumGraphic& i_rcGraphic ) override
  {
    rumAssert( false );
  }

  void DrawCircle( const rumPoint& i_rcPos, int32_t i_iRadius, const rumColor& i_rcColor ) override
  {
    rumAssert( false );
  }

  void DrawLine( const rumPoint& i_rcPos1, const rumPoint& i_rcPos2, const rumColor& i_rcColor ) override
  {
    rumAssert( false );
  }

  void DrawRect( const rumRectangle& i_rcRect, const rumColor& i_rcColor ) override
  {
    rumAssert( false );
  }

  void DrawRectUnfilled( const rumRectangle& i_rcRect, const rumColor& i_rcColor ) override
  {
    rumAssert( false );
  }

  void DrawTriangle( const rumPoint& i_rcPos1, const rumPoint& i_rcPos2, const rumPoint& i_rcPos3,
                     const rumColor& i_rcColor ) override
  {
    rumAssert( false );
  }

  void Flip() override
  {
    rumAssert( false );
  }

  void FlipMirror() override
  {
    rumAssert( false );
  }

  //void FreeData() override;

  void GenerateAlpha() override
  {
    rumAssert( false );
  }

  bool HasAlpha() const override
  {
    rumAssert( false );
    return false;
  }

  // Init data with an empty width and height
  bool InitData( uint32_t i_uiWidth, uint32_t i_uiHeight ) override
  {
    return InitData();
  }

  // Init data from asset
  bool InitData() override
  {
    rumAssert( false );
    return false;
  }

  // Init data from raw pixel information
  bool InitData( const rumColor* i_pcImage, uint32_t i_uiWidth, uint32_t i_uiHeight ) override
  {
    return InitData();
  }

  void Mirror() override
  {
    rumAssert( false );
  }

  static void ScriptBind();

protected:

  void OnCreated() override;

private:

  using super = rumGraphic;
};

#endif // _E_GRAPHICS_H_
