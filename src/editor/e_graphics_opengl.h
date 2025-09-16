// Defined in the main project CMakeLists.txt

#ifndef _E_GRAPHICS_OPENGL_H_
#define _E_GRAPHICS_OPENGL_H_

#include <e_graphics.h>


class EditorGraphic : public EditorGraphicBase
{
public:

  ~EditorGraphic();

  // Draws entire graphic to a point on the backbuffer
  void Draw( const rumPoint& i_rcPos ) const override;

  // Draws part of a graphic to a point on the backbuffer
  void Draw( const rumPoint& i_rcDest, const rumPoint& i_rcSrc, uint32_t i_uiWidth, uint32_t i_uiHeight ) const override;

  void Free() override;

  uint32_t GetHeight() const override;
  uint32_t GetWidth() const override;

  // Init data with an empty width and height
  bool InitData( uint32_t i_uiWidth, uint32_t i_uiHeight ) override;

  // Init data from asset
  bool InitData() override;

  // Init data from raw pixel information
  bool InitData( const rumColor* i_pcImage, uint32_t i_uiWidth, uint32_t i_uiHeight ) override;

  void Shift( const rumVector& i_rcVector ) override;

  static void EnableBlending( bool i_bEnable );

  static void Init();
  static void ScriptBind();
  static void Shutdown();

private:

  void FreeInternal();

  using super = EditorGraphicBase;
};

#endif // #define _E_GRAPHICS_OPENGL_H_
