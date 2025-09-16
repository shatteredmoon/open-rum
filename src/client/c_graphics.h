#ifndef _C_GRAPHIC_H_
#define _C_GRAPHIC_H_

#include <u_graphic.h>
#include <u_structs.h>

struct rumConfig;

#ifdef MessageBox
#undef MessageBox
#endif // MessageBox

#define DEFAULT_COLOR_DEPTH       16
#define DEFAULT_FULLSCREEN        false
#define DEFAULT_FULLSCREEN_WIDTH  640
#define DEFAULT_FULLSCREEN_HEIGHT 480
#define DEFAULT_SCREEN_WIDTH      640
#define DEFAULT_SCREEN_HEIGHT     480


class rumClientGraphicBase : public rumGraphic
{
public:

  static void DrawCircle_VM( const rumPoint& i_rcPos, int32_t i_iRadius, const rumColor& i_rcColor );
  static void DrawLine_VM( const rumPoint& i_rcPos1, const rumPoint& i_rcPos2, const rumColor& i_rcColor );
  static void DrawRect_VM( const rumRectangle& i_rcRect, const rumColor& i_rcColor );
  static void DrawRectUnfilled_VM( const rumRectangle& i_rcRect, const rumColor& i_rcColor );
  static void DrawTriangle_VM( const rumPoint& i_rcPos1, const rumPoint& i_rcPos2, const rumPoint& i_rcPos3,
                               const rumColor& i_rcColor );

  virtual rumColor GetMaskColor() = 0;

  static void ClearScreen()
  {
    ClearScreenColor( rumColor::s_cBlack );
  }

  static void ClearScreenColor( const rumColor& i_rcColor )
  {
    GetBackBuffer().Clear( i_rcColor );
  }

  static void ClearScreen( const rumRectangle& i_rcRect )
  {
    ClearScreen( i_rcRect, rumColor::s_cBlack );
  }

  static void ClearScreen( const rumRectangle& i_rcRect, const rumColor& i_rcColor )
  {
    GetBackBuffer().ClearRect( i_rcRect, i_rcColor );
  }

  static SQInteger ClearScreen_VM( HSQUIRRELVM i_pcVM );

  static rumGraphic& GetBackBuffer();
  static rumGraphic& GetPrimaryBuffer();

  static uint32_t GetFullScreenHeight()
  {
    return s_uiFullScreenHeight;
  }

  static uint32_t GetFullScreenWidth()
  {
    return s_uiFullScreenWidth;
  }

  static uint32_t GetScreenBufferHeight()
  {
    return DEFAULT_SCREEN_HEIGHT;
  }

  static uint32_t GetScreenBufferWidth()
  {
    return DEFAULT_SCREEN_WIDTH;
  }

  static uint32_t GetWindowedScreenHeight()
  {
    return s_uiWindowedScreenHeight;
  }

  static uint32_t GetWindowedScreenWidth()
  {
    return s_uiWindowedScreenWidth;
  }

  static int32_t Init( const rumConfig& i_rcConfig );

  static bool IsBufferBMP( const char* i_pcBuffer, uint32_t i_uiSize );
  static bool IsBufferJPG( const char* i_pcBuffer, uint32_t i_uiSize );
  static bool IsBufferPCX( const char* i_pcBuffer, uint32_t i_uiSize );
  static bool IsBufferPNG( const char* i_pcBuffer, uint32_t i_uiSize );
  static bool IsBufferTGA( const char* i_pcBuffer, uint32_t i_uiSize );

  static bool IsFullscreen();

  static void MessageBox( const std::string& i_rcStr );

  // Provides simple ANSI text output
  static void OutputText( const std::string& i_rcStr, const rumPoint& i_rcDest, const rumColor& i_rcColor );

  static void RenderGame();

  static void ScriptBind();

  static void Shutdown();

protected:

  static void OnWindowSizeChanged();

  static int32_t s_iWindowPosX;
  static int32_t s_iWindowPosY;

  static uint32_t s_uiFullScreenHeight;
  static uint32_t s_uiFullScreenWidth;

  static uint32_t s_uiWindowedScreenHeight;
  static uint32_t s_uiWindowedScreenWidth;

  static bool s_bFullscreen;

private:

  static bool s_bInitialized;

  using super = rumGraphic;
};

#endif // _C_GRAPHIC_H_
