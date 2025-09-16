// Defined in the main project CMakeLists.txt
#ifdef USE_GLFW

#ifndef _C_GRAPHICS_GLFW_H_
#define _C_GRAPHICS_GLFW_H_

#include <c_graphics.h>

#include <u_log.h>
#include <u_utility.h>

#include <glad/glad.h>

struct GLFWwindow;
struct GLFWmonitor;


class rumClientGraphic : public rumClientGraphicBase
{
public:

  void Blit( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc, const rumPoint& i_rcDest,
             uint32_t i_uiWidth, uint32_t i_uiHeight ) override;

  void BlitAdd( const rumGraphic& i_rcGraphic, const rumPoint& i_rcPos, const rumColor& i_rcColor ) override;

  void BlitAlpha( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc, const rumPoint& i_rcDest,
                  uint32_t i_uiWidth, uint32_t i_uiHeight, bool i_bUseSrcColor ) const override;
  void BlitAlphaPreserve( const rumGraphic& i_rcGraphic, const rumPoint& i_rcDest ) const override;

  void BlitColorMask( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc, const rumPoint& i_rcDest,
                      uint32_t i_uiWidth, uint32_t i_uiHeight ) override;

  void BlitMultiply( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc, const rumPoint& i_rcDest,
                     uint32_t i_uiWidth, uint32_t i_uiHeight, const rumColor& i_rcColor ) override;

  void BlitRandomEffect( const rumGraphic& i_rcGraphic, const rumPoint& i_rcPos,
                         uint32_t i_uiWidth, uint32_t i_uiHeight ) const override;

  void BlitStretch( rumGraphic& i_rcGraphic,
                    const rumPoint& i_rcSrc,
                    const rumPoint& i_rcDest,
                    uint32_t i_uiDestWidth,
                    uint32_t i_uiDestHeight ) override;

  void BlitTransparent( const rumGraphic& i_rcGraphic, const rumPoint& i_rcPos, uint32_t i_iAlpha ) override;

  void Clear( const rumColor& i_rcColor ) override;

  void ClearRect( const rumRectangle& i_rcRect, const rumColor& i_rcColor ) override;

  void CopyAlpha( const rumGraphic& i_rcGraphic ) override;

  void Draw( const rumPoint& i_rcPos ) const override;
  void Draw( const rumPoint& i_rcDest, const rumPoint& i_rcSrc,
             uint32_t i_uiWidth, uint32_t i_uiHeight ) const override;
  void DrawCircle( const rumPoint& i_rcPos, int32_t i_iRadius, const rumColor& i_rcColor ) override;
  void DrawLine( const rumPoint& i_rcPos1, const rumPoint& i_rcPos2, const rumColor& i_rcColor ) override;
  void DrawRect( const rumRectangle& i_rcRect, const rumColor& i_rcColor ) override;
  void DrawRectUnfilled( const rumRectangle& i_rcRect, const rumColor& i_rcColor ) override;
  void DrawTriangle( const rumPoint& i_rcPos1, const rumPoint& i_rcPos2, const rumPoint& i_rcPos3,
                     const rumColor& i_rcColor ) override;

  void Flip() override;
  void FlipMirror() override;

  void Free() override;

  void GenerateAlpha() override;

  uint32_t GetHeight() const override;
  uint32_t GetWidth() const override;

  rumColor GetMaskColor() override;

  bool HasAlpha() const override;

  // Init data from asset
  bool InitData() override;

  // Init data with an empty width and height
  bool InitData( uint32_t i_uiWidth, uint32_t i_uiHeight ) override;

  // Init data from raw pixel information
  bool InitData( const rumColor* i_pcImage, uint32_t i_uiWidth, uint32_t i_uiHeight ) override;

  void Mirror() override;

  void Shift( const rumVector& i_rcVector ) override;

  static rumPoint GetBestResolutionForAspectRatio( int32_t i_iMaxWidth, int32_t i_iMaxHeight, float i_fAspectRatio );
  static rumPoint GetMaxScreenResolution();

  static Sqrat::Array GetResolutions();

#ifdef WIN32
  static HWND GetWindowHandle();
#endif // WIN32

  static int32_t Init( const rumConfig& i_rcConfig );

  static void OutputText( const std::string& i_str, const rumPoint& i_rcPos, const rumColor& i_rcColor );

  static void RenderGame();

  static void ScriptBind();

  static void SetFullscreenMode( uint32_t i_uiWidth, uint32_t i_uiHeight );
  static void SetWindowedMode( uint32_t i_uiWidth, uint32_t i_uiHeight );
  static void SetWindowSize( uint32_t i_uiWidth, uint32_t i_uiHeight );

  static void SetWindowTitle( const std::string& i_strTitle );

  static void Shutdown();

  static bool WindowClosed();

private:

  void FreeInternal();

  static void CopyFramebuffer( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc, const rumPoint& i_rcDest,
                               uint32_t i_uiWidth, uint32_t i_uiHeight );

  static void DisableFramebuffer();

  static void EnableFramebuffer( GLdouble i_fWidth, GLdouble i_fHeight );

  static void ErrorCallback( int32_t i_iError, const char* i_strDescription );
  static void FramebufferSizeCallback( GLFWwindow* i_pcWindow, int32_t i_iWidth, int32_t i_iHeight );
  static void MonitorCallback( GLFWmonitor* i_pcMonitor, int32_t i_iEvent );

  static GLboolean QueryExtension( char* i_strExtName );

  static void RenderQuad( const rumGraphic& i_rcSrcGraphic,
                          const rumPoint& i_rcSrc, uint32_t i_uiSrcWidth, uint32_t i_uiSrcHeight,
                          const rumGraphic& i_rcDestGraphic, rumPoint i_rcDest,
                          bool i_bUseFramebuffer );

  static void TextureQuad( GLuint i_hTexture,
                           float i_fSrcX1, float i_fSrcY1, float i_fSrcX2, float i_fSrcY2,
                           float i_fDestX1, float i_fDestY1, float i_fDestX2, float i_fDestY2 );

  static void WindowCloseCallback( GLFWwindow* i_pcWindow );
  static void WindowContentScaleCallback( GLFWwindow* i_pcWindow, float i_fXScale, float i_fYScale );
  static void WindowFocusCallback( GLFWwindow* i_pcWindow, int32_t i_iFocused );
  static void WindowMaximizedCallback( GLFWwindow* i_pcWindow, int32_t i_iMaximized );
  static void WindowPositionCallback( GLFWwindow* i_pcWindow, int32_t i_iXPos, int32_t i_iYPos );
  static void WindowRefreshCallback( GLFWwindow* i_pcWindow );
  static void WindowSizeCallback( GLFWwindow* i_pcWindow, int32_t i_iWidth, int32_t i_iHeight );

  static GLFWwindow* s_pcWindow;

  static GLuint s_hFramebuffer[2];
  static GLuint s_hFramebufferTexture[2];

  static GLubyte* s_pcBuffer[4];

  static rumGraphic* s_pcTempGraphic[4];

  static bool s_bUsingFramebuffer;

  // Set to true any time a resize is programmatically requested. GLFW resize callbacks don't provide a mechanism for
  // determining if the resize is because the user is dragging the screen edge, so this is used to infer that.
  static bool s_bProgrammaticResizeRequested;

  using super = rumClientGraphicBase;
};

#endif // _C_GRAPHICS_GLFW_H_

#endif // USE_GLFW
