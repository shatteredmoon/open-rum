// Defined in the main project CMakeLists.txt
#ifdef USE_GLFW

#include <c_graphics_glfw.h>

#include <u_graphic_attributes.h>
#include <u_log.h>
#include <u_utility.h>

#include <GLFW/glfw3.h>

#ifdef WIN32
#include <Windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <GLFW/glfw3native.h>

// Note that this can't be included prior to the other GLFW includes
#include <c_input_glfw.h>

#ifdef _DEBUG
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#endif

// Namespace required because of typedef BOOL conflict
namespace FreeImage
{
#include <freeimage.h>
}


// Static initializations
GLFWwindow* rumClientGraphic::s_pcWindow{ nullptr };

GLuint rumClientGraphic::s_hFramebuffer[2] = {};
GLuint rumClientGraphic::s_hFramebufferTexture[2] = {};

GLubyte* rumClientGraphic::s_pcBuffer[4] = {};
rumGraphic* rumClientGraphic::s_pcTempGraphic[4] = {};

bool rumClientGraphic::s_bUsingFramebuffer{ false };
bool rumClientGraphic::s_bProgrammaticResizeRequested{ false };


void rumClientGraphic::Blit( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc, const rumPoint& i_rcDest,
                             uint32_t i_uiWidth, uint32_t i_uiHeight )
{
  if( i_uiWidth <= 0 || i_uiHeight <= 0 )
  {
    return;
  }

  const bool bTargetIsTexture{ this != &GetBackBuffer() };

  RenderQuad( i_rcGraphic, i_rcSrc, i_uiWidth, i_uiHeight, *this, i_rcDest, bTargetIsTexture );
}


void rumClientGraphic::BlitAdd( const rumGraphic& i_rcGraphic, const rumPoint& i_rcPos, const rumColor& i_rcColor )
{
  const bool bTargetIsTexture{ this != &GetBackBuffer() };

  glPushAttrib( GL_CURRENT_BIT );
  glEnable( GL_BLEND );
  glColor4f( i_rcColor.GetRed() / 255.f, i_rcColor.GetGreen() / 255.f, i_rcColor.GetBlue() / 255.f,
             i_rcColor.GetAlpha() / 255.f );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE );

  RenderQuad( i_rcGraphic, rumPoint(), i_rcGraphic.GetWidth(), i_rcGraphic.GetHeight(), *this, i_rcPos,
              bTargetIsTexture );

  glDisable( GL_BLEND );
  glPopAttrib();
}


void rumClientGraphic::BlitAlpha( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc, const rumPoint& i_rcDest,
                                  uint32_t i_uiWidth, uint32_t i_uiHeight, bool i_bUseSrcColor ) const
{
  const bool bTargetIsTexture{ this != &GetBackBuffer() };

  glPushAttrib( GL_CURRENT_BIT );
  glEnable( GL_BLEND );
  glColor4f( 1.0f, 1.0f, 1.0f, i_rcGraphic.GetAttributes().m_uiTransparentLevel / (float)RUM_ALPHA_OPAQUE );
  glBlendFunc( GL_SRC_ALPHA, i_bUseSrcColor ? GL_ONE_MINUS_SRC_COLOR : GL_ONE_MINUS_SRC_ALPHA );

  RenderQuad( i_rcGraphic, i_rcSrc, i_uiWidth, i_uiHeight, *this, i_rcDest, bTargetIsTexture );

  glDisable( GL_BLEND );
  glPopAttrib();
}


void rumClientGraphic::BlitAlphaPreserve( const rumGraphic& i_rcGraphic, const rumPoint& i_rcDest ) const
{
  const uint32_t uiSrcHeight{ i_rcGraphic.GetHeight() };
  const uint32_t uiSrcWidth{ i_rcGraphic.GetWidth() };

  const uint32_t uiDestHeight{ GetHeight() };
  const uint32_t uiDestWidth{ GetWidth() };

  // The destination must be larger than the source must fit into the bounds of the destination at the specified offset

  //const bool bReqSize = ( uiDestWidth == uiSrcWidth && uiDestHeight == uiSrcHeight );
  //rumAssert(bReqSize);
  //if( !bReqSize )
  //{
  //  return;
  //}

  // Fetch the source texture
  glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)i_rcGraphic.GetData() );
  GLubyte* piSrcImage{ s_pcBuffer[0] };
  const GLubyte* piSrcImagePtr{ piSrcImage };
  glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, piSrcImage );

  // Fetch the destination texture
  glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );
  GLubyte* pcDestImage{ s_pcBuffer[1] };
  GLubyte* piDestImagePtr{ pcDestImage };
  glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pcDestImage );

  for( uint32_t j = i_rcDest.m_iY; j < ( i_rcDest.m_iY + uiSrcHeight ); ++j )
  {
    piDestImagePtr = pcDestImage + ( uiDestWidth * j * 4 ) + ( i_rcDest.m_iX * 4 );

    for( uint32_t i = i_rcDest.m_iX; i < ( i_rcDest.m_iX + uiSrcWidth ); ++i )
    {
      // If the source alpha is > the destination alpha, replace the destination pixel
      if( piSrcImagePtr[3] > piDestImagePtr[3] )
      {
        piDestImagePtr[0] = piSrcImagePtr[0];
        piDestImagePtr[1] = piSrcImagePtr[1];
        piDestImagePtr[2] = piSrcImagePtr[2];
        piDestImagePtr[3] = piSrcImagePtr[3];
      }

      // Move to the next color values
      piDestImagePtr += 4;
      piSrcImagePtr += 4;
    }
  }

  // Make sure all src pixels were processed
  rumAssert( piSrcImagePtr == ( piSrcImage + ( uiSrcHeight * uiSrcWidth * 4 ) ) );

  // Store the modified image
  glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, uiDestWidth, uiDestHeight, GL_RGBA, GL_UNSIGNED_BYTE, pcDestImage );
  glBindTexture( GL_TEXTURE_2D, 0 );
}


void rumClientGraphic::BlitColorMask( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc, const rumPoint& i_rcDest,
                                      uint32_t i_uiWidth, uint32_t i_uiHeight )
{
  const uint32_t uiDestHeight{ GetHeight() };
  const uint32_t uiDestWidth{ GetWidth() };

  const uint32_t uiSrcWidth{ i_rcGraphic.GetWidth() };

  const bool bReqSize{ uiDestWidth >= i_uiWidth && uiDestHeight >= i_uiHeight };
  rumAssert( bReqSize );
  if( !bReqSize )
  {
    return;
  }

  // Fetch the source texture
  glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)i_rcGraphic.GetData() );
  GLubyte* piSrcImage{ s_pcBuffer[0] };
  glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, piSrcImage );

  // Fetch the destination texture
  glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );
  GLubyte* piDestImage = s_pcBuffer[1];
  glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, piDestImage );

  const rumColor& cColorMask{ GetMaskColor() };

  GLubyte* piSrcImagePtr{ nullptr };
  GLubyte* piDestImagePtr{ nullptr };

  for( uint32_t j = 0; j < i_uiHeight; ++j )
  {
    piDestImagePtr = piDestImage + ( j + i_rcDest.m_iY ) * ( uiDestWidth << 2 ) + ( i_rcDest.m_iX << 2 );
    piSrcImagePtr = piSrcImage + ( j + i_rcSrc.m_iY ) * ( uiSrcWidth << 2 ) + ( i_rcSrc.m_iX << 2 );

    for( uint32_t i = 0; i < i_uiWidth; ++i )
    {
      const rumColor cSrcColor( piSrcImagePtr[0], piSrcImagePtr[1], piSrcImagePtr[2], piSrcImagePtr[3] );
      if( cSrcColor == cColorMask )
      {
        piDestImagePtr[0] = piSrcImagePtr[0];
        piDestImagePtr[1] = piSrcImagePtr[1];
        piDestImagePtr[2] = piSrcImagePtr[2];
        piDestImagePtr[3] = piSrcImagePtr[3];
      }

      // Move to the next color values
      piDestImagePtr += 4;
      piSrcImagePtr += 4;
    }
  }

  // Store the modified image
  // TODO - this updates the entire destination texture, even if only a small part of it was updated
  glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, uiDestWidth, uiDestHeight, GL_RGBA, GL_UNSIGNED_BYTE, piDestImage );
  glBindTexture( GL_TEXTURE_2D, 0 );
}


void rumClientGraphic::BlitMultiply( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc, const rumPoint& i_rcDest,
                                     uint32_t i_uiWidth, uint32_t i_uiHeight, const rumColor& i_rcColor )
{
  const bool bTargetIsTexture{ this != &GetBackBuffer() };

  glPushAttrib( GL_CURRENT_BIT );
  glEnable( GL_BLEND );

  // Annoyingly, inverting the alpha value seems to make everything work. I don't have a clue why that is necessary.
  glColor4f( i_rcColor.GetRed() / 255.f, i_rcColor.GetGreen() / 255.f, i_rcColor.GetBlue() / 255.f,
             ( 255.f - i_rcColor.GetAlpha() ) / 255.f );

  glBlendFunc( GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA );

  RenderQuad( i_rcGraphic, i_rcSrc, i_uiWidth, i_uiHeight, *this, i_rcDest, bTargetIsTexture );

  glDisable( GL_BLEND );
  glPopAttrib();
}


void rumClientGraphic::BlitRandomEffect( const rumGraphic& i_rcGraphic, const rumPoint& i_rcPos,
                                         uint32_t i_uiWidth, uint32_t i_uiHeight ) const
{
  static int32_t myArray[11] = { GL_ZERO, GL_ONE, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR, GL_SRC_ALPHA,
                                 GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA, GL_DST_COLOR,
                                 GL_ONE_MINUS_DST_COLOR, GL_SRC_ALPHA_SATURATE };

  const bool bTargetIsTexture{ this != &GetBackBuffer() };

  glPushAttrib( GL_CURRENT_BIT );
  glEnable( GL_BLEND );

  switch( rand() % 7 )
  {
    case 0: glColor4f( rand() % 256 / 255.f, rand() % 256 / 255.f, rand() % 256 / 255.f, rand() % 256 / 255.f ); break;
    case 1: glColor4f( 1.f, 1.f, 1.f, 1.f ); break;
    case 2: glColor4f( 0.f, 0.f, 0.f, 0.f ); break;
    case 3: glColor4f( 1.f, 1.f, 1.f, 0.f ); break;
    case 4: glColor4f( 0.f, 0.f, 0.f, 1.f ); break;
    case 5: glColor4f( rand() % 256 / 255.f, rand() % 256 / 255.f, rand() % 256 / 255.f, 1.f ); break;
    case 6: glColor4f( rand() % 256 / 255.f, rand() % 256 / 255.f, rand() % 256 / 255.f, 0.f ); break;
  }

  const int32_t iSrcFactor{ myArray[rand() % 11] };
  const int32_t iDestFactor{ myArray[rand() % 11] };

  glBlendFunc( myArray[iSrcFactor], myArray[iDestFactor] );

  RenderQuad( i_rcGraphic, rumPoint(), i_uiWidth, i_uiHeight, *this, i_rcPos, bTargetIsTexture );

  glDisable( GL_BLEND );
  glPopAttrib();
}


void rumClientGraphic::BlitStretch( rumGraphic& i_rcGraphic,
                                    const rumPoint& i_rcSrc,
                                    const rumPoint& i_rcDest,
                                    uint32_t i_uiDestWidth,
                                    uint32_t i_uiDestHeight )
{
  if( i_rcGraphic.GetFrameWidth() <= 0 || i_rcGraphic.GetFrameHeight() <= 0 )
  {
    return;
  }

  if( i_rcGraphic.GetFrameWidth() == i_uiDestWidth || i_rcGraphic.GetFrameHeight() == i_uiDestHeight )
  {
    Blit( i_rcGraphic, i_rcSrc, i_rcDest, i_rcGraphic.GetFrameWidth(), i_rcGraphic.GetFrameHeight() );
    return;
  }

  auto& rcAttributes{ i_rcGraphic.GetAttributes() };
  const auto cAttributeCopy{ rcAttributes };

  rcAttributes.m_bDrawScaled = true;
  rcAttributes.m_fHorizontalScale = i_uiDestWidth / static_cast<float>( i_rcGraphic.GetFrameWidth() );
  rcAttributes.m_fVerticalScale = i_uiDestHeight / static_cast<float>( i_rcGraphic.GetFrameHeight() );

  i_rcGraphic.SetAttributes( rcAttributes );

  const bool bTargetIsTexture{ this != &GetBackBuffer() };

  RenderQuad( i_rcGraphic, i_rcSrc, i_rcGraphic.GetFrameWidth(), i_rcGraphic.GetFrameHeight(), *this, i_rcDest, bTargetIsTexture );

  m_cGraphicAttributes = cAttributeCopy;
  i_rcGraphic.SetAttributes( cAttributeCopy );
}


void rumClientGraphic::BlitTransparent( const rumGraphic& i_rcGraphic, const rumPoint& i_rcPos, uint32_t i_iAlpha )
{
  const bool bTargetIsTexture{ this != &GetBackBuffer() };

  glPushAttrib( GL_CURRENT_BIT );
  glEnable( GL_BLEND );

  glColor4f( 1.0f, 1.0f, 1.0f, 1 - ( i_iAlpha / (float)RUM_ALPHA_OPAQUE ) );

  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

  RenderQuad( i_rcGraphic, rumPoint(), i_rcGraphic.GetWidth(), i_rcGraphic.GetHeight(), *this, i_rcPos,
              bTargetIsTexture );

  glDisable( GL_BLEND );
  glPopAttrib();
}


void rumClientGraphic::Clear( const rumColor& i_rcColor )
{
  if( this == &GetBackBuffer() )
  {
    glClearColor( i_rcColor.GetRed() / 255.f, i_rcColor.GetGreen() / 255.f, i_rcColor.GetBlue() / 255.f,
                  i_rcColor.GetAlpha() / 255.f );
    glClear( GL_COLOR_BUFFER_BIT );
  }
  else
  {
    const uint32_t uiHeight{ GetHeight() };
    const uint32_t uiWidth{ GetWidth() };

    if( uiWidth > 0 && uiHeight > 0 )
    {
      const uint32_t uiSize{ uiWidth * uiHeight };

      BYTE* piImage{ s_pcBuffer[0] };
      BYTE* piImagePtr{ piImage };

      for( uint32_t i = 0; i < uiSize; ++i )
      {
        piImagePtr[0] = i_rcColor.GetRed();
        piImagePtr[1] = i_rcColor.GetGreen();
        piImagePtr[2] = i_rcColor.GetBlue();
        piImagePtr[3] = i_rcColor.GetAlpha();
        piImagePtr += 4;
      }

      // Store the new image
      glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );
      glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, uiWidth, uiHeight, GL_RGBA, GL_UNSIGNED_BYTE, piImage );
      //rumAssert(glGetError() == GL_NO_ERROR);
      glBindTexture( GL_TEXTURE_2D, 0 );
    }
  }
}


void rumClientGraphic::ClearRect( const rumRectangle& i_rcRect, const rumColor& i_rcColor )
{
  rumAssert( i_rcRect.m_cPoint2.m_iX >= i_rcRect.m_cPoint1.m_iX &&
             i_rcRect.m_cPoint2.m_iY >= i_rcRect.m_cPoint1.m_iY );

  if( this == &GetBackBuffer() )
  {
    glEnable( GL_SCISSOR_TEST );
    glScissor( i_rcRect.m_cPoint1.m_iX, i_rcRect.m_cPoint1.m_iY, i_rcRect.GetWidth(), i_rcRect.GetHeight() );

    glClearColor( i_rcColor.GetRed() / 255.f, i_rcColor.GetGreen() / 255.f, i_rcColor.GetBlue() / 255.f,
                  i_rcColor.GetAlpha() / 255.f );
    glClear( GL_COLOR_BUFFER_BIT );
    glDisable( GL_SCISSOR_TEST );
  }
  else
  {
    const uint32_t uiHeight{ GetHeight() };
    const uint32_t uiWidth{ GetWidth() };
    if( uiWidth > 0 && uiHeight > 0 )
    {
      const int32_t uiSubHeight{ i_rcRect.GetHeight() };
      const int32_t uiSubWidth{ i_rcRect.GetWidth() };

      if( uiSubHeight > 0 && uiSubWidth > 0 )
      {
        // Clear the rectangle to the specified color
        BYTE* piImage{ s_pcBuffer[0] };
        const uint32_t uiSize{ static_cast<uint32_t>( uiSubHeight * uiSubWidth ) };

        for( uint32_t i = 0; i < uiSize; ++i )
        {
          piImage[0] = i_rcColor.GetRed();
          piImage[1] = i_rcColor.GetGreen();
          piImage[2] = i_rcColor.GetBlue();
          piImage[3] = i_rcColor.GetAlpha();
          piImage += 4;
        }

        // Store the new image
        glBindTexture( GL_TEXTURE_2D, static_cast<GLuint>( (size_t)GetData() ) );
        glTexSubImage2D( GL_TEXTURE_2D, 0, i_rcRect.m_cPoint1.m_iX, i_rcRect.m_cPoint1.m_iY, uiSubWidth, uiSubHeight,
                         GL_RGBA, GL_UNSIGNED_BYTE, s_pcBuffer[0] );
        glBindTexture( GL_TEXTURE_2D, 0 );
      }
    }
  }
}


void rumClientGraphic::CopyAlpha( const rumGraphic& i_rcGraphic )
{
  const uint32_t uiSrcHeight{ i_rcGraphic.GetHeight() };
  const uint32_t uiSrcWidth{ i_rcGraphic.GetWidth() };

  const uint32_t uiDestHeight{ GetHeight() };
  const uint32_t uiDestWidth{ GetWidth() };

  /*const bool bReqSize = (uiDestWidth == uiSrcWidth && uiDestHeight == uiSrcHeight);
  rumAssert(bReqSize);
  if (bReqSize)*/
  {
    // Fetch the source texture
    glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)i_rcGraphic.GetData() );
    GLubyte* piSrcImage{ s_pcBuffer[0] };
    GLubyte* piSrcImagePtr{ piSrcImage };
    glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, piSrcImage );

    // Fetch the destination texture
    glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );
    GLubyte* piDestImage{ s_pcBuffer[1] };
    GLubyte* piDestImagePtr{ piDestImage };
    glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, piDestImage );

    const uint32_t uiSize{ uiSrcHeight * uiSrcWidth };
    for( uint32_t i = 0; i < uiSize; ++i )
    {
      piDestImagePtr[3] = piSrcImagePtr[3];

      // Move to the next color values
      piDestImagePtr += 4;
      piSrcImagePtr += 4;
    }

    // Store the modified image
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, uiDestWidth, uiDestHeight, GL_RGBA, GL_UNSIGNED_BYTE, piDestImage );
    glBindTexture( GL_TEXTURE_2D, 0 );
  }
}


// static
void rumClientGraphic::CopyFramebuffer( const rumGraphic& i_rcGraphic, const rumPoint& i_rcSrc,
                                        const rumPoint& i_rcDest, uint32_t i_uiWidth, uint32_t i_uiHeight )
{
  // We are going to copy the texture from our main framebuffer to a texture on the secondary framebuffer. Normally,
  // we would need to bind to both, but we're already bound for read/draw on the main framebuffer.
  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, s_hFramebuffer[1] );

  // Bind the destination draw to the second framebuffer's texture. We would normally need to bind GL_READ_FRAMEBUFFER
  // as well, but in our case that is bound at init and never changes. Example
  glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                          (GLuint)(size_t)i_rcGraphic.GetData(), 0 );

  rumAssert( glCheckFramebufferStatus( GL_DRAW_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE );

  glBlitFramebuffer( i_rcSrc.m_iX, i_rcSrc.m_iY, i_rcSrc.m_iX + i_uiWidth, i_rcSrc.m_iY + i_uiHeight,
                     i_rcDest.m_iX, i_rcDest.m_iY, i_rcDest.m_iX + i_uiWidth, i_rcDest.m_iY + i_uiHeight,
                     GL_COLOR_BUFFER_BIT, GL_NEAREST );

  glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );
  //glBindFramebuffer( GL_READ_FRAMEBUFFER, 0 );

  // Re-bind for read/draw on the original framebuffer
  glBindFramebuffer( GL_FRAMEBUFFER, s_hFramebuffer[0] );
}


// static
void rumClientGraphic::DisableFramebuffer()
{
  rumAssert( s_bUsingFramebuffer );
  s_bUsingFramebuffer = false;

  glPopAttrib();

  //glBindFramebufferFunc(GL_FRAMEBUFFER, 0);

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  // left, right, bottom, top, znear, zfar
  glOrtho( 0.0, (GLdouble)GetScreenBufferWidth(), (GLdouble)GetScreenBufferHeight(), 0.0, -1.0, 1.0 );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  // VERY IMPORTANT! Don't unbind until after resetting the screen projection! Otherwise, any texture read from
  // the framebuffer will come out flipped!
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}


void rumClientGraphic::Draw( const rumPoint& i_rcPos ) const
{
  glPushAttrib( GL_ENABLE_BIT );

  const float fOffsetY1{ (float)i_rcPos.m_iY };
  const float fOffsetY2{ fOffsetY1 + GetHeight() * m_cGraphicAttributes.m_fHorizontalScale };

  const float fOffsetX1{ (float)i_rcPos.m_iX };
  const float fOffsetX2{ fOffsetX1 + GetWidth() * m_cGraphicAttributes.m_fVerticalScale };

  if( m_cGraphicAttributes.m_bDrawMasked )
  {
    glPushAttrib( GL_CURRENT_BIT );
    glEnable( GL_BLEND );
    glColor4f( 1.0f, 1.0f, 1.0f, m_cGraphicAttributes.m_uiTransparentLevel / (float)RUM_ALPHA_OPAQUE );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

    TextureQuad( (GLuint)(size_t)GetData(), 0.0f, 1.0f, 1.0f, 0.0f, fOffsetX1, fOffsetY1, fOffsetX2, fOffsetY2 );

    glDisable( GL_BLEND );
    glPopAttrib();
  }
  else
  {
    TextureQuad( (GLuint)(size_t)GetData(), 0.0f, 1.0f, 1.0f, 0.0f, fOffsetX1, fOffsetY1, fOffsetX2, fOffsetY2 );
  }

  glPopAttrib();
}


void rumClientGraphic::Draw( const rumPoint& i_rcDest, const rumPoint& i_rcSrc, uint32_t i_uiWidth,
                             uint32_t i_uiHeight ) const
{
  glPushAttrib( GL_ENABLE_BIT );

  const rumGraphic* pcSrcGraphic{ this };

  float fHeight{ (float)GetHeight() };
  float fWidth{ (float)GetWidth() };

  if( fHeight < 1.f || fWidth < 1.f )
  {
    return;
  }

  // Y offset
  const float fY1{ (float)i_rcDest.m_iY };
  const float fY2{ fY1 + i_uiHeight * m_cGraphicAttributes.m_fHorizontalScale };

  // X offset
  const float fX1{ (float)i_rcDest.m_iX };
  const float fX2{ fX1 + i_uiWidth * m_cGraphicAttributes.m_fVerticalScale };

  // Offset percentage
  float fXOffsetPercent1{ i_rcSrc.m_iX / fWidth };
  float fXOffsetPercent2{ ( i_rcSrc.m_iX + i_uiWidth ) / fWidth };

  float fYOffsetPercent1 = i_rcSrc.m_iY / fHeight;
  float fYOffsetPercent2 = ( i_rcSrc.m_iY + i_uiHeight ) / fHeight;

  glEnable( GL_TEXTURE_2D );

  rumPoint cDestPos( i_rcSrc );
  const rumPoint cZero;

  bool bUsedFrameBuffer{ false };

  if( m_cGraphicAttributes.m_bDrawLit )
  {
    // Use a temporary graphic for light adjustments
    s_pcTempGraphic[1]->ClearRect( rumRectangle( cZero, i_uiWidth, i_uiHeight ), rumColor::s_cBlack );

    glPushAttrib( GL_CURRENT_BIT );
    glEnable( GL_BLEND );

    glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );

    // Create a color based on the light level
    glColor4f( m_cGraphicAttributes.m_fLightLevel, m_cGraphicAttributes.m_fLightLevel,
               m_cGraphicAttributes.m_fLightLevel, 1.0f );

    // Use the sources color values, blended with the light color above. Do not factor the destination color in at all.
    glBlendFunc( GL_ONE, GL_ZERO );

    glPushAttrib( GL_ENABLE_BIT );

    // Note the sy is flipped because we're using a framebuffer
    s_pcTempGraphic[1]->Blit( *this, rumPoint( i_rcSrc.m_iX, (uint32_t)fHeight - i_rcSrc.m_iY ), cZero,
                              i_uiWidth, i_uiHeight );

    glPopAttrib();

    glDisable( GL_BLEND );
    glPopAttrib();

    // Recompute texture offsets for the new source texture
    fHeight = (float)s_pcTempGraphic[1]->GetHeight();
    fWidth = (float)s_pcTempGraphic[1]->GetWidth();

    // Offset percentage
    fXOffsetPercent1 = 0.f;
    fXOffsetPercent2 = i_uiWidth / fWidth;

    fYOffsetPercent1 = 0.f;
    fYOffsetPercent2 = i_uiHeight / fHeight;

    // Switch to using the temp graphic from here on out
    pcSrcGraphic = s_pcTempGraphic[1];

    // The new source offsets
    cDestPos = cZero;

    // So that we know to flip the image coordinates later
    bUsedFrameBuffer = true;
  }

  if( m_cGraphicAttributes.m_eBlendType != rumGraphicAttributes::BlendType_None )
  {
    glPushAttrib( GL_ENABLE_BIT );

    // Use a temporary graphic for blending
    s_pcTempGraphic[2]->Blit( *pcSrcGraphic, cDestPos, cZero, i_uiWidth, i_uiHeight );
    s_pcTempGraphic[3]->ClearRect( rumRectangle( cZero, i_uiWidth, i_uiHeight ), m_cGraphicAttributes.m_cBufferColor );

    rumColor cColor( m_cGraphicAttributes.m_cBlendColor );

    switch( m_cGraphicAttributes.m_eBlendType )
    {
      case rumGraphicAttributes::BlendType_Translucent:
      {
        s_pcTempGraphic[2]->BlitMultiply( *s_pcTempGraphic[3], cZero, cZero, i_uiWidth, i_uiHeight, cColor );

        // Grab the alpha values from the original
        s_pcTempGraphic[2]->BlitAlpha( *this, i_rcSrc, cZero, i_uiWidth, i_uiHeight, true /* src color */ );

        break;
      }

      //case rumGraphicAttributes::BlendType_Screen:
      //  break;
      //
      //case rumGraphicAttributes::BlendType_Add:
      //  break;
      //
      //case rumGraphicAttributes::BlendType_Burn:
      //  break;
      //
      //case rumGraphicAttributes::BlendType_Color:
      //  break;
      //
      //case rumGraphicAttributes::BlendType_Difference:
      //  break;
      //
      //case rumGraphicAttributes::BlendType_Dissolve:
      //  break;
      //
      //case rumGraphicAttributes::BlendType_Dodge:
      //  break;
      //
      //case rumGraphicAttributes::BlendType_Hue:
      //  break;
      //
      //case rumGraphicAttributes::BlendType_Invert:
      //  break;
      //
      //case rumGraphicAttributes::BlendType_Luminance:
      //  break;

      case rumGraphicAttributes::BlendType_Multiply:
        cColor.SetRed( m_cGraphicAttributes.m_cBufferColor.GetRed() );
        cColor.SetGreen( m_cGraphicAttributes.m_cBufferColor.GetGreen() );
        cColor.SetBlue( m_cGraphicAttributes.m_cBufferColor.GetBlue() );

        // Note that the alpha is not copied
        s_pcTempGraphic[2]->BlitMultiply( *s_pcTempGraphic[3], cZero, cZero, i_uiWidth, i_uiHeight, cColor );

        // Grab the alpha values from the original
        s_pcTempGraphic[2]->BlitAlpha( *this, i_rcSrc, cZero, i_uiWidth, i_uiHeight, true /* src color */ );
        break;

      //case rumGraphicAttributes::BlendType_Saturation:
      //  break;
    }

    if( m_cGraphicAttributes.m_bDrawMasked && m_cGraphicAttributes.m_bRestoreAlphaPostBlend )
    {
      // Restore the color mask - these values were undoubtedly affected
      s_pcTempGraphic[2]->BlitColorMask( *this, i_rcSrc, cZero, i_uiWidth, i_uiHeight );
    }

    // Recompute texture offsets for the new source texture
    fHeight = (float)s_pcTempGraphic[2]->GetHeight();
    fWidth = (float)s_pcTempGraphic[2]->GetWidth();

    // Offset percentage
    fXOffsetPercent1 = 0.f;
    fXOffsetPercent2 = i_uiWidth / fWidth;

    fYOffsetPercent1 = 0.f;
    fYOffsetPercent2 = i_uiHeight / fHeight;

    pcSrcGraphic = s_pcTempGraphic[2];

    // The new source offsets
    cDestPos = cZero;

    glPopAttrib();

    // So that we know to flip the image coordinates later
    bUsedFrameBuffer = true;
  }

  if( m_cGraphicAttributes.m_uiTransparentLevel != RUM_ALPHA_OPAQUE )
  {
    glPushAttrib( GL_CURRENT_BIT );
    glEnable( GL_BLEND );
    glColor4f( 1.f, 1.f, 1.f, m_cGraphicAttributes.m_uiTransparentLevel / (float)RUM_ALPHA_OPAQUE );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
  }

  if( m_cGraphicAttributes.m_bDrawMasked )
  {
    glPushAttrib( GL_CURRENT_BIT );
    glEnable( GL_BLEND );
    glColor4f( 1.f, 1.f, 1.f, m_cGraphicAttributes.m_uiTransparentLevel / (float)RUM_ALPHA_OPAQUE );

    // Standard transparency blend
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }

  glBegin( GL_QUADS );

  if( bUsedFrameBuffer )
  {
    // Note that source y-coordinates are flipped
    TextureQuad( (GLuint)(size_t)GetData(),
                 fXOffsetPercent1, fYOffsetPercent2, fXOffsetPercent2, fYOffsetPercent1,
                 fX1, fY1, fX2, fY2 );
  }
  else
  {
    TextureQuad( (GLuint)(size_t)GetData(),
                 fXOffsetPercent1, 1.f - fYOffsetPercent1, fXOffsetPercent2, 1.f - fYOffsetPercent2,
                 fX1, fY1, fX2, fY2 );
  }

  glEnd();

  if( m_cGraphicAttributes.m_bDrawMasked )
  {
    glDisable( GL_BLEND );
    glPopAttrib();
  }

  if( m_cGraphicAttributes.m_uiTransparentLevel != RUM_ALPHA_OPAQUE )
  {
    glDisable( GL_BLEND );
    glPopAttrib();
  }

  glPopAttrib();
}


void rumClientGraphic::DrawCircle( const rumPoint& i_rcPos, int32_t i_iRadius, const rumColor& i_rcColor )
{
  glPushAttrib( GL_CURRENT_BIT );

  glColor4f( i_rcColor.GetRed() / 255.f, i_rcColor.GetGreen() / 255.f, i_rcColor.GetBlue() / 255.f,
             i_rcColor.GetAlpha() / 255.f );

  glBegin( GL_LINE_STRIP );
  for( int32_t i = 0; i < 360; ++i )
  {
    // Get the current angle
    const float fTheta{ 2.0f * 3.1415926f * float( i ) / float( 360.f ) };

    // Output vertex
    const float fX{ i_iRadius * cosf( fTheta ) };
    const float fY{ i_iRadius * sinf( fTheta ) };
    glVertex2f( fX + i_rcPos.m_iX, fY + i_rcPos.m_iY );
  }
  glEnd();

  glPopAttrib();
}


void rumClientGraphic::DrawLine( const rumPoint& i_rcPos1, const rumPoint& i_rcPos2, const rumColor& i_rcColor )
{
  glPushAttrib( GL_CURRENT_BIT );

  glColor4f( i_rcColor.GetRed() / 255.f, i_rcColor.GetGreen() / 255.f, i_rcColor.GetBlue() / 255.f,
             i_rcColor.GetAlpha() / 255.f );

  glBegin( GL_LINES );
  glVertex2f( (float)i_rcPos1.m_iX, (float)i_rcPos1.m_iY );
  glVertex2f( (float)i_rcPos2.m_iX, (float)i_rcPos2.m_iY );
  glEnd();

  glPopAttrib();
}


void rumClientGraphic::DrawRect( const rumRectangle& i_rcRect, const rumColor& i_rcColor )
{
  glPushAttrib( GL_CURRENT_BIT );
  glColor4f( i_rcColor.GetRed() / 255.f, i_rcColor.GetGreen() / 255.f, i_rcColor.GetBlue() / 255.f,
             i_rcColor.GetAlpha() / 255.f );
  glRectd( i_rcRect.m_cPoint1.m_iX, i_rcRect.m_cPoint1.m_iY, i_rcRect.GetWidth(), i_rcRect.GetHeight() );
  glPopAttrib();
}


void rumClientGraphic::DrawRectUnfilled( const rumRectangle& i_rcRect, const rumColor& i_rcColor )
{
  glPushAttrib( GL_CURRENT_BIT );

  glColor4f( i_rcColor.GetRed() / 255.f, i_rcColor.GetGreen() / 255.f, i_rcColor.GetBlue() / 255.f,
             i_rcColor.GetAlpha() / 255.f );

  glBegin( GL_LINE_STRIP );
  glVertex2f( (float)i_rcRect.m_cPoint1.m_iX, (float)i_rcRect.m_cPoint1.m_iY );
  glVertex2f( (float)i_rcRect.m_cPoint2.m_iX, (float)i_rcRect.m_cPoint1.m_iY );
  glVertex2f( (float)i_rcRect.m_cPoint2.m_iX, (float)i_rcRect.m_cPoint2.m_iY );
  glVertex2f( (float)i_rcRect.m_cPoint1.m_iX, (float)i_rcRect.m_cPoint2.m_iY );
  glVertex2f( (float)i_rcRect.m_cPoint1.m_iX, (float)i_rcRect.m_cPoint1.m_iY );
  glEnd();

  glPopAttrib();
}


void rumClientGraphic::DrawTriangle( const rumPoint& i_rcPos1, const rumPoint& i_rcPos2, const rumPoint& i_rcPos3,
                                     const rumColor& i_rcColor )
{
  glPushAttrib( GL_CURRENT_BIT );

  glColor4f( i_rcColor.GetRed() / 255.f, i_rcColor.GetGreen() / 255.f, i_rcColor.GetBlue() / 255.f,
             i_rcColor.GetAlpha() / 255.f );

  glBegin( GL_LINE_LOOP );
  glVertex2f( (float)i_rcPos1.m_iX, (float)i_rcPos1.m_iY );
  glVertex2f( (float)i_rcPos2.m_iX, (float)i_rcPos2.m_iY );
  glVertex2f( (float)i_rcPos3.m_iX, (float)i_rcPos3.m_iY );
  glEnd();

  glPopAttrib();
}


// static
void rumClientGraphic::EnableFramebuffer( GLdouble i_fWidth, GLdouble i_fHeight )
{
  rumAssert( !s_bUsingFramebuffer );
  s_bUsingFramebuffer = true;

  glBindFramebuffer( GL_FRAMEBUFFER, s_hFramebuffer[0] );

  // Always check that our framebuffer is ok
  if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
  {
    rumAssertArgs( false, "glCheckFramebufferStatus = ", glCheckFramebufferStatus( GL_FRAMEBUFFER ) );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    return;
  }

  // Clear buffers
  glClear( GL_COLOR_BUFFER_BIT );

  // Switch to the framebuffer's view
  glPushAttrib( GL_VIEWPORT_BIT );
  glViewport( 0, 0, (GLsizei)i_fWidth, (GLsizei)i_fHeight );

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();
  glOrtho( 0.0, (GLdouble)i_fWidth, (GLdouble)i_fHeight, 0.0, -1.0, 1.0 );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  // Really cool vibration effect by playing around with the projection matrix
  /*float fRand1 = GetRandomFloat32_Range( -1.f, 1.f );
  float fRand2 = GetRandomFloat32_Range( -1.f, 1.f );
  float fRand3 = GetRandomFloat32_Range( -1.f, 1.f );

  float fModelView[16] =
  {
    1.0f, 0.0f, 0.0f, 0.f,
    0.0f, 1.0f, 0.0f, 0.f,
    0.0f, 0.0f, 1.0f, 0.f,
    fRand1, fRand2, fRand3, 1.0f
  };

  glMultMatrixf( fModelView );*/
}


// static
void rumClientGraphic::ErrorCallback( int32_t i_iError, const char* i_strDescription )
{
  RUM_COUT_IFDEF_DBG( GRAPHIC_DEBUG, "GLFW error " << i_iError << ": " << i_strDescription << '\n' );
}


void rumClientGraphic::Flip()
{
  glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );

  const uint32_t uiHeight{ GetHeight() };
  const uint32_t uiWidth{ GetWidth() };

  // Fetch the current texture
  GLubyte* piImage{ s_pcBuffer[0] };
  GLubyte* piNewImage{ s_pcBuffer[1] };

  glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, piImage );

  GLubyte* piImagePtr{ piImage };
  GLubyte* piNewImagePtr{ piNewImage };

  for( uint32_t j = 0; j < uiHeight; ++j )
  {
    piNewImagePtr = piNewImage + ( ( uiHeight - j - 1 ) * uiWidth * 4 );

    for( uint32_t i = 0; i < uiWidth; ++i )
    {
      piNewImagePtr[0] = piImagePtr[0];
      piNewImagePtr[1] = piImagePtr[1];
      piNewImagePtr[2] = piImagePtr[2];
      piNewImagePtr[3] = piImagePtr[3];

      // Move to the next color values
      piNewImagePtr += 4;
      piImagePtr += 4;
    }
  }

  // Store the modified image
  glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, uiWidth, uiHeight, GL_RGBA, GL_UNSIGNED_BYTE, piNewImage );
  glBindTexture( GL_TEXTURE_2D, 0 );
}


void rumClientGraphic::FlipMirror()
{
  glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );

  const uint32_t uiHeight{ GetHeight() };
  const uint32_t uiWidth{ GetWidth() };

  // Fetch the current texture
  GLubyte* piImage{ s_pcBuffer[0] };
  GLubyte* piNewImage{ s_pcBuffer[1] };

  glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, piImage );

  GLubyte* piImagePtr{ piImage };
  GLubyte* piNewImagePtr{ piNewImage };

  for( uint32_t j = 0; j < uiHeight; ++j )
  {
    // Flip and Mirror
    piNewImagePtr = piNewImage + ( ( uiHeight - j ) * uiWidth * 4 ) - 4;

    for( uint32_t i = 0; i < uiWidth; ++i )
    {
      piNewImagePtr[0] = piImagePtr[0];
      piNewImagePtr[1] = piImagePtr[1];
      piNewImagePtr[2] = piImagePtr[2];
      piNewImagePtr[3] = piImagePtr[3];

      // Move to the next color values
      piNewImagePtr -= 4;
      piImagePtr += 4;
    }
  }

  // Store the modified image
  glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, uiWidth, uiHeight, GL_RGBA, GL_UNSIGNED_BYTE, piNewImage );
  glBindTexture( GL_TEXTURE_2D, 0 );
}


// static
void rumClientGraphic::FramebufferSizeCallback( GLFWwindow* i_pcWindow, int32_t i_iWidth, int32_t i_iHeight )
{
  RUM_COUT_IFDEF_DBG( GRAPHIC_DEBUG, "GLFW framebuffer resized " << i_iWidth << " x  " << i_iHeight << '\n' );

  if( s_bFullscreen )
  {
    glViewport( 0, 0, s_uiFullScreenWidth, s_uiFullScreenHeight );
  }
  else
  {
    if( !s_bProgrammaticResizeRequested )
    {
      // The user is resizing the window by dragging one of its edges
      // Note: Interestingly, commenting the windowed screen size updates here lets the window resize while showing,
      // but the end result is wrong...
      s_uiWindowedScreenWidth = static_cast<uint32_t>( i_iWidth );
      s_uiWindowedScreenHeight = static_cast<uint32_t>( i_iHeight );
      glViewport( 0, 0, s_uiWindowedScreenWidth, s_uiWindowedScreenHeight );
    }
    else
    {
      glViewport( 0, 0, i_iWidth, i_iHeight );
    }
  }

  OnWindowSizeChanged();
}


void rumClientGraphic::Free()
{
  FreeInternal();
  super::Free();
}


void rumClientGraphic::FreeInternal()
{
  // Graphic memory is stored internally in OpenGL
  if( m_pcData != nullptr )
  {
    GLuint hTexture{ (GLuint)(size_t)GetData() };
    glDeleteTextures( 1, &hTexture );

    m_pcData = nullptr;
  }
}


void rumClientGraphic::GenerateAlpha()
{
  const uint32_t uiHeight{ GetHeight() };
  const uint32_t uiWidth{ GetWidth() };

  glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );

  // Fetch the current texture
  GLubyte* piImage{ s_pcBuffer[0] };
  glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, piImage );

  GLubyte* piImagePtr{ piImage };
  const uint32_t uiSize{ uiWidth * uiHeight };
  for( uint32_t i = 0; i < uiSize; ++i )
  {
    const rumColor cColor( piImagePtr[0], piImagePtr[1], piImagePtr[2], piImagePtr[3] );
    if( ( cColor.GetRed() == 0xff ) && ( cColor.GetGreen() == 0x00 ) && ( cColor.GetBlue() == 0xff ) )
    {
      piImagePtr[3] = RUM_ALPHA_TRANSPARENT;
    }

    // Move to the next color value
    piImagePtr += 4;
  }

  // Store the modified image
  glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, uiWidth, uiHeight, GL_RGBA, GL_UNSIGNED_BYTE, piImage );
  glBindTexture( GL_TEXTURE_2D, 0 );
}


// static
rumPoint rumClientGraphic::GetBestResolutionForAspectRatio( int32_t i_iMaxWidth, int32_t i_iMaxHeight, float i_fAspectRatio )
{
  int32_t iCount{ 0 };
  const GLFWvidmode* pcModes{ nullptr };

  GLFWmonitor* pcMonitor{ glfwGetPrimaryMonitor() };
  if( pcMonitor )
  {
    pcModes = glfwGetVideoModes(pcMonitor, &iCount);
  }

  if( iCount > 0 )
  {
    for( int32_t i{ iCount - 1 }; i >= 0; --i )
    {
      if( pcModes[i].width <= i_iMaxWidth && pcModes[i].height <= i_iMaxHeight )
      {
        const float fAspectRatio{ pcModes[i].width / static_cast<float>( pcModes[i].height ) };
        if( rumNumberUtils::ValuesEqual<float>( fAspectRatio, i_fAspectRatio ) )
        {
          return { pcModes[i].width, pcModes[i].height };
        }
      }
    }
  }

  return { i_iMaxWidth, i_iMaxHeight };
}


uint32_t rumClientGraphic::GetHeight() const
{
  glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );

  GLint iHeight{ 0 };
  glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &iHeight );

  return (uint32_t)iHeight;
}


rumColor rumClientGraphic::GetMaskColor()
{
  return rumColor( 0xff, 0x00, 0xff, 0xff );
}


// static
rumPoint rumClientGraphic::GetMaxScreenResolution()
{
  rumPoint cResolution;

  GLFWmonitor* pcMonitor{ glfwGetPrimaryMonitor() };
  if( !pcMonitor )
  {
    return cResolution;
  }

  int32_t iNumModes;
  const GLFWvidmode* pcModes{ glfwGetVideoModes( pcMonitor, &iNumModes ) };
  for( int32_t i = 0; i < iNumModes; ++i )
  {
    if( cResolution.m_iX < pcModes[i].width )
    {
      cResolution.m_iX = pcModes[i].width;
    }

    if( cResolution.m_iY < pcModes[i].height )
    {
      cResolution.m_iY = pcModes[i].height;
    }
  }

  return cResolution;
}


// static
Sqrat::Array rumClientGraphic::GetResolutions()
{
  int32_t iCount{ 0 };
  const GLFWvidmode* pcModes{ nullptr };

  GLFWmonitor* pcMonitor{ glfwGetPrimaryMonitor() };
  if( pcMonitor )
  {
    pcModes = glfwGetVideoModes( pcMonitor, &iCount );
  }

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  // It's okay to have zero length squirrel arrays
  Sqrat::Array sqArray( pcVM, iCount );

  if( iCount > 0 )
  {
    for( int32_t i{ 0 }; i < iCount; ++i )
    {
      Sqrat::Array sqResolution( pcVM, 2 );
      sqResolution.SetValue( 0, pcModes[i].width );
      sqResolution.SetValue( 1, pcModes[i].height );

      // Add the key
      sqArray.SetValue( i, sqResolution );
    }
  }

  return sqArray;
}


uint32_t rumClientGraphic::GetWidth() const
{
  glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );

  GLint iWidth{ 0 };
  glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &iWidth );

  return (uint32_t)iWidth;
}


#ifdef WIN32

// static
HWND rumClientGraphic::GetWindowHandle()
{
  return glfwGetWin32Window( s_pcWindow );
}

#endif // WIN32


bool rumClientGraphic::HasAlpha() const
{
  bool bAlpha{ false };

  const uint32_t uiHeight{ GetHeight() };
  const uint32_t uiWidth{ GetWidth() };

  glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );

  // Fetch the current texture
  GLubyte* piImage{ s_pcBuffer[0] };
  glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, piImage );

  GLubyte* piImagePtr{ piImage };
  const uint32_t uiSize{ uiWidth * uiHeight };
  for( uint32_t i = 0; i < uiSize && !bAlpha; ++i )
  {
    const rumColor cColor( piImagePtr[0], piImagePtr[1], piImagePtr[2], piImagePtr[3] );
    if( cColor.GetAlpha() > 0 )
    {
      bAlpha = true;
    }

    // Move to the next color value
    piImagePtr += 4;
  }

  glBindTexture( GL_TEXTURE_2D, 0 );

  return bAlpha;
}


// static
int32_t rumClientGraphic::Init( const rumConfig& i_rcConfig )
{
  if( super::Init( i_rcConfig ) != RESULT_SUCCESS )
  {
    return RESULT_FAILED;
  }

  using namespace FreeImage;

  FreeImage_Initialise();

  RUM_COUT( "Using: FreeImage " << FreeImage_GetVersion() << "\n" );
  RUM_COUT( FreeImage_GetCopyrightMessage() << "\n" );

  // Initialize GLFW
  // Create a GLFW window without an OpenGL context
  glfwSetErrorCallback( ErrorCallback );

  RUM_COUT( "Using: GLFW " << glfwGetVersionString() << "\n" );

  if( !glfwInit() )
  {
    return RESULT_FAILED;
  }

  int32_t iNumMonitors{ 0 };
  //GLFWmonitor** aMonitors{ glfwGetMonitors( &iNumMonitors ) };
  RUM_COUT_IFDEF_DBG( GRAPHIC_DEBUG, "User has " << iNumMonitors << " monitor(s)\n" );

  GLFWmonitor* pcMonitor{ glfwGetPrimaryMonitor() };
  if( !pcMonitor )
  {
    return RESULT_FAILED;
  }

  RUM_COUT_IFDEF_DBG( GRAPHIC_DEBUG, "GLFW using monitor: " << glfwGetMonitorName( pcMonitor ) << '\n' );

  glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );
  glfwWindowHint( GLFW_SCALE_TO_MONITOR, GLFW_FALSE );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 2 );
  glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 1 );

  s_uiFullScreenHeight = i_rcConfig.m_uiFullScreenHeight;
  s_uiFullScreenWidth = i_rcConfig.m_uiFullScreenWidth;

  const GLFWvidmode* pcMode{ glfwGetVideoMode( pcMonitor ) };
  uint32_t uiWindowHeight{ pcMode != nullptr ? static_cast<uint32_t>( pcMode->height ) : DEFAULT_FULLSCREEN_HEIGHT };
  uint32_t uiWindowWidth{ pcMode != nullptr ? static_cast<uint32_t>( pcMode->width ) : DEFAULT_FULLSCREEN_WIDTH };

  if( ( 0 == s_uiFullScreenHeight ) || ( 0 == s_uiFullScreenWidth ) || 
      ( s_uiFullScreenHeight > uiWindowHeight ) || ( s_uiFullScreenWidth > uiWindowWidth ) )
  {
    const float fAspectRatio{ DEFAULT_FULLSCREEN_WIDTH / static_cast<float>( DEFAULT_FULLSCREEN_HEIGHT ) };
    const auto cResolution{ GetBestResolutionForAspectRatio( uiWindowWidth, uiWindowHeight, fAspectRatio ) };
    s_uiFullScreenHeight = cResolution.m_iY;
    s_uiFullScreenWidth = cResolution.m_iX;
  }

  s_uiWindowedScreenHeight = i_rcConfig.m_uiWindowedScreenHeight;
  s_uiWindowedScreenWidth = i_rcConfig.m_uiWindowedScreenWidth;

  uiWindowHeight = pcMode != nullptr ? static_cast<uint32_t>( pcMode->height ) : DEFAULT_SCREEN_HEIGHT;
  uiWindowWidth = pcMode != nullptr ? static_cast<uint32_t>( pcMode->width ) : DEFAULT_SCREEN_WIDTH;

  if( ( 0 == s_uiWindowedScreenHeight ) || ( 0 == s_uiWindowedScreenWidth ) ||
      ( s_uiWindowedScreenHeight > uiWindowHeight ) || ( s_uiWindowedScreenWidth > uiWindowWidth ) )
  {
    const float fAspectRatio{ DEFAULT_SCREEN_WIDTH / static_cast<float>( DEFAULT_SCREEN_HEIGHT ) };
    const auto cResolution{ GetBestResolutionForAspectRatio( uiWindowWidth, uiWindowHeight, fAspectRatio )};
    s_uiWindowedScreenHeight = cResolution.m_iY;
    s_uiWindowedScreenWidth = cResolution.m_iX;
  }

  GLFWwindow* pcWindow{ glfwCreateWindow( s_uiWindowedScreenWidth, s_uiWindowedScreenHeight, "RUM Client", nullptr,
                                          nullptr ) };
  if( !pcWindow )
  {
    return RESULT_FAILED;
  }

  glfwSetWindowSizeCallback( pcWindow, WindowSizeCallback );
  glfwSetWindowCloseCallback( pcWindow, WindowCloseCallback );
  glfwSetFramebufferSizeCallback( pcWindow, FramebufferSizeCallback );
  glfwSetWindowContentScaleCallback( pcWindow, WindowContentScaleCallback );
  glfwSetWindowFocusCallback( pcWindow, WindowFocusCallback );
  glfwSetWindowMaximizeCallback( pcWindow, WindowMaximizedCallback );
  glfwSetWindowPosCallback( pcWindow, WindowPositionCallback );
  glfwSetWindowRefreshCallback( pcWindow, WindowRefreshCallback );
  glfwSetMonitorCallback( MonitorCallback );

  s_pcWindow = pcWindow;

  rumInput::SetWindow( pcWindow );

  glfwMakeContextCurrent( pcWindow );

  // Access to higher versions of OpenGL (generated from https://glad.dav1d.de/)
  gladLoadGL();

  // Set vsync to a single frame to prevent tearing
  glfwSwapInterval( 1 );

  glBindTexture( GL_TEXTURE_2D, 0 );

  // Create framebuffers for temporary blits
  for( uint32_t i = 0; i < 2; ++i )
  {
    glGenFramebuffers( 1, &s_hFramebuffer[i] );
    glBindFramebuffer( GL_FRAMEBUFFER, s_hFramebuffer[i] );

    // Bind a texture to the framebuffer
    glGenTextures( 1, &s_hFramebufferTexture[i] );
    glBindTexture( GL_TEXTURE_2D, s_hFramebufferTexture[i] );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0 );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 ); // only allocating space
    glBindTexture( GL_TEXTURE_2D, 0 );

    // Attach the destination texture as color attachment #0
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, s_hFramebufferTexture[i], 0 );
  }

  s_pcTempGraphic[0] = Create();
  s_pcTempGraphic[0]->InitData( 1024, 1024 );
  s_pcTempGraphic[1] = Create();
  s_pcTempGraphic[1]->InitData( 1024, 1024 );
  s_pcTempGraphic[2] = Create();
  s_pcTempGraphic[2]->InitData( 1024, 1024 );
  s_pcTempGraphic[3] = Create();
  s_pcTempGraphic[3]->InitData( 1024, 1024 );

  s_pcBuffer[0] = new GLubyte[2048 * 2048 * 4];
  s_pcBuffer[1] = new GLubyte[2048 * 2048 * 4];
  s_pcBuffer[2] = new GLubyte[1024 * 1024 * 4];
  s_pcBuffer[3] = new GLubyte[1024 * 1024 * 4];

  // Setup the primary screen framebuffer
  glClearColor( 0.f, 0.f, 0.f, 1.f );

  glViewport( 0, 0, s_uiWindowedScreenWidth, s_uiWindowedScreenHeight );

  glMatrixMode( GL_PROJECTION );
  glLoadIdentity();

  const float fAspectRatio{ 1.f };

  // left, right, bottom, top, znear, zfar
  glOrtho( 0.0,
           GLdouble( DEFAULT_SCREEN_WIDTH * fAspectRatio ),
           GLdouble( DEFAULT_SCREEN_HEIGHT * fAspectRatio ),
           0.0, -1.0, 1.0 );

  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  glDisable( GL_CULL_FACE );
  glDisable( GL_BLEND );
  glDisable( GL_LIGHTING );
  glDisable( GL_DEPTH_TEST );

  // Create Pseudo primary and back buffer
  GetPrimaryBuffer().InitData( DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT );
  GetBackBuffer().InitData( DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT );

  // Save the current window position
  glfwGetWindowPos( s_pcWindow, &s_iWindowPosX, &s_iWindowPosY );

  if( i_rcConfig.m_bFullscreen )
  {
    SetFullscreenMode( s_uiFullScreenWidth, s_uiFullScreenHeight );
  }

#ifdef _DEBUG
  ImGui_ImplGlfw_InitForOpenGL( pcWindow, true );
  ImGui_ImplOpenGL3_Init();
#endif

  return RESULT_SUCCESS;
}


bool rumClientGraphic::InitData()
{
  using namespace FreeImage;

  rumAssert( m_pcAsset );
  if( !m_pcAsset || !m_pcAsset->GetData() )
  {
    return false;
  }

  bool bSuccess{ true };

  RUM_COUT_IFDEF_DBG( GRAPHIC_DEBUG, "Loading texture\n" );

  FreeInternal();

  GLuint hTexture{ 0 };
  glGenTextures( 1, &hTexture );

  // Data is the OpenGL allocated texture name
  m_pcData = reinterpret_cast<void*>( (size_t)hTexture );

  const uint32_t uiNumBytes{ m_pcAsset->GetDataAllocSize() };

  FIMEMORY* pcStream{ FreeImage_OpenMemory( (BYTE*)m_pcAsset->GetData(), uiNumBytes ) };
  if( pcStream )
  {
    const FREE_IMAGE_FORMAT eFormat{ FreeImage_GetFileTypeFromMemory( pcStream, uiNumBytes ) };
    FIBITMAP* pcBitmap{ FreeImage_LoadFromMemory( eFormat, (FIMEMORY*)pcStream ) };
    if( pcBitmap )
    {
      // Now, there is no guarantee that the image file loaded will be GL_RGB, so we force FreeImage to convert the
      // image to GL_RGB
      pcBitmap = FreeImage_ConvertTo32Bits( pcBitmap );

      glBindTexture( GL_TEXTURE_2D, hTexture );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0 );

      const uint32_t uiWidth{ FreeImage_GetWidth( pcBitmap ) };
      const uint32_t uiHeight{ FreeImage_GetHeight( pcBitmap ) };
      const uint32_t uiSize{ uiWidth * uiHeight };

      // Firstly, allocate the new bit data for the image
      rumAssert( uiSize <= ( 2048 * 2048 ) );
      BYTE* pcNewImage{ s_pcBuffer[0] };

      // Get a pointer to FreeImage's data
      const BYTE* piImage{ (BYTE*)FreeImage_GetBits( pcBitmap ) };

      // Iterate through the pixels, copying the data from 'pixels' to 'pNewImage' except in RGB format
      for( uint32_t i = 0; i < uiSize; ++i )
      {
        const uint32_t iOffset{ i * 4 };
        pcNewImage[iOffset + 0] = piImage[iOffset + 0];
        pcNewImage[iOffset + 1] = piImage[iOffset + 1];
        pcNewImage[iOffset + 2] = piImage[iOffset + 2];

        // The original pcx assets use hot pink as the transparency color, so we detect that here
        if( ( pcNewImage[iOffset + 0] == 0xff ) &&
            ( pcNewImage[iOffset + 1] == 0x00 ) &&
            ( pcNewImage[iOffset + 2] == 0xff ) )
        {
          // Alpha full
          pcNewImage[iOffset + 3] = 0x00;
        }
        else
        {
          // Opaque
          pcNewImage[iOffset + 3] = 0xff;
        }
      }

      // The new 'glTexImage2D' function, the prime difference being that it gets the width, height and pixel
      // information from 'pNewImage', which is the RGB pixel data
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, uiWidth, uiHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE, pcNewImage );

      // bitmap successfully loaded!
      FreeImage_Unload( pcBitmap );
      FreeImage_CloseMemory( pcStream );
    }
    else
    {
      RUM_COUT_IFDEF_DBG( GRAPHIC_DEBUG, "Error: Failed loading texture\n" );
      bSuccess = false;
    }
  }
  else
  {
    RUM_COUT_IFDEF_DBG( GRAPHIC_DEBUG, "Error: Failed loading texture\n" );
    bSuccess = false;
  }

  GenerateAlpha();
  CalcAnimation();

  return bSuccess;
}


bool rumClientGraphic::InitData( uint32_t i_uiWidth, uint32_t i_uiHeight )
{
  FreeInternal();

  GLuint hTexture{ 0 };
  glGenTextures( 1, &hTexture );

  // Data is the OpenGL allocated texture name
  m_pcData = reinterpret_cast<void*>( (size_t)hTexture );
  rumAssert( m_pcData );

  if( i_uiWidth > 0 && i_uiHeight > 0 )
  {
    glBindTexture( GL_TEXTURE_2D, hTexture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0 );

    BYTE* piBits{ s_pcBuffer[0] };

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, i_uiWidth, i_uiHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, piBits );
    glBindTexture( GL_TEXTURE_2D, 0 );
  }

  return true;
}


bool rumClientGraphic::InitData( const rumColor* i_pcImage, uint32_t i_uiWidth, uint32_t i_uiHeight )
{
  rumAssert( i_pcImage );

  if( i_pcImage && InitData( i_uiWidth, i_uiHeight ) )
  {
    glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );
    glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, i_uiWidth, i_uiHeight, GL_RGBA, GL_UNSIGNED_BYTE, i_pcImage );

    glBindTexture( GL_TEXTURE_2D, 0 );

    return true;
  }

  return false;
}


void rumClientGraphic::Mirror()
{
  glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );

  const uint32_t uiHeight{ GetHeight() };
  const uint32_t uiWidth{ GetWidth() };

  // Fetch the current texture
  GLubyte* piImage{ s_pcBuffer[0] };
  GLubyte* piNewImage{ s_pcBuffer[1] };

  glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, piImage );

  GLubyte* piImagePtr = piImage;
  GLubyte* piNewImagePtr = piNewImage;

  for( uint32_t j = 0; j < uiHeight; ++j )
  {
    piNewImagePtr = piNewImage + ( ( j + 1 ) * uiWidth * 4 ) - 4;

    for( uint32_t i = 0; i < uiWidth; ++i )
    {
      piNewImagePtr[0] = piImagePtr[0];
      piNewImagePtr[1] = piImagePtr[1];
      piNewImagePtr[2] = piImagePtr[2];
      piNewImagePtr[3] = piImagePtr[3];

      // Move to the next color values
      piNewImagePtr -= 4;
      piImagePtr += 4;
    }
  }

  // Store the modified image
  glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, uiWidth, uiHeight, GL_RGBA, GL_UNSIGNED_BYTE, piNewImage );
  glBindTexture( GL_TEXTURE_2D, 0 );
}


// static
void rumClientGraphic::MonitorCallback( GLFWmonitor* i_pcMonitor, int32_t i_iEvent )
{
#if GRAPHIC_DEBUG
  const char* strName{ glfwGetMonitorName( i_pcMonitor ) };

  if( GLFW_CONNECTED == i_iEvent )
  {
    // The monitor was connected
    RUM_COUT_DBG( "GLFW monitor connected: " << strName << '\n' );
  }
  else if( GLFW_DISCONNECTED == i_iEvent )
  {
    // The monitor was disconnected
    RUM_COUT_DBG( "GLFW monitor disconnected: " << strName << '\n' );
  }
#endif // GRAPHIC_DEBUG
}


// static
void rumClientGraphic::OutputText( const std::string& i_str, const rumPoint& i_rcPos, const rumColor& i_rcColor )
{
  // TODO
  rumAssert( false );
}


// static
GLboolean rumClientGraphic::QueryExtension( char* i_strExtension )
{
  // Search for extName in the extensions string. Use of strstr() is not sufficient because extension names can
  // be prefixes of other extension names.

  const int32_t iNameLength{ (int32_t)strlen( i_strExtension ) };

  char* strExtension{ (char*)glGetString( GL_EXTENSIONS ) };
  if( !strExtension )
  {
    return GL_FALSE;
  }

  char* strEnd{ strExtension + strlen( strExtension ) };

  while( strExtension < strEnd )
  {
    int32_t iIndex{ (int32_t)strcspn( strExtension, " " ) };
    if( ( iNameLength == iIndex ) && ( strncmp( i_strExtension, strExtension, iIndex ) == 0 ) )
    {
      return GL_TRUE;
    }

    strExtension += ( iIndex + 1 );
  }

  return GL_FALSE;
}


// static
void rumClientGraphic::RenderGame()
{
  s_bProgrammaticResizeRequested = false;

  glfwPollEvents();

  // Clear screen
  /*glClearColor(ColorBlackOpaque.r, ColorBlackOpaque.g, ColorBlackOpaque.b, ColorBlackOpaque.a);
  glClear(GL_COLOR_BUFFER_BIT);

  glLoadIdentity();
  glTranslatef(1, 1, 0);*/

#ifdef _DEBUG
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
#endif

  super::RenderGame();

#ifdef _DEBUG
  ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
#endif

  // Technically input, but for a graphical reason
  glfwSetInputMode( s_pcWindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN );

  glfwSwapBuffers( s_pcWindow );
}


// static
void rumClientGraphic::RenderQuad( const rumGraphic& i_rcSrcGraphic,
                                   const rumPoint& i_rcSrc, uint32_t i_uiSrcWidth, uint32_t i_uiSrcHeight,
                                   const rumGraphic& i_rcDestGraphic, rumPoint i_rcDest,
                                   bool i_bUseFramebuffer )
{
  const float fSourceGraphicHeight{ (float)i_rcSrcGraphic.GetHeight() };
  const float fSourceGraphicWidth{ (float)i_rcSrcGraphic.GetWidth() };

  if( ( 0.f == fSourceGraphicWidth ) || ( 0.f == fSourceGraphicHeight ) )
  {
    return;
  }

  if( i_rcDest.m_iX < 0 )
  {
    i_uiSrcWidth -= 0 - i_rcDest.m_iX;
    i_rcDest.m_iX = 0;
  }

  if( i_rcDest.m_iY < 0 )
  {
    i_uiSrcHeight -= 0 - i_rcDest.m_iY;
    i_rcDest.m_iY = 0;
  }

  if( i_uiSrcWidth < 0 || i_uiSrcHeight < 0 )
  {
    return;
  }

  const auto& i_rcAttributes{ i_rcSrcGraphic.GetAttributes() };

  if( i_bUseFramebuffer )
  {
    EnableFramebuffer( (GLdouble)i_uiSrcWidth * i_rcAttributes.m_fHorizontalScale,
                       (GLdouble)i_uiSrcHeight * i_rcAttributes.m_fVerticalScale );

    // Copy the destination to the framebuffer so that it can be rendered over

    const float fDestGraphicHeight{ (float)i_rcDestGraphic.GetHeight() };
    const float fDestGraphicWidth{ (float)i_rcDestGraphic.GetWidth() };

    if( fDestGraphicHeight < 1.f || fDestGraphicWidth < 1.f )
    {
      DisableFramebuffer();
      return;
    }

    float fSourceWidth{ (GLfloat)i_uiSrcWidth * i_rcAttributes.m_fHorizontalScale };
    float fSourceHeight{ (GLfloat)i_uiSrcHeight * i_rcAttributes.m_fVerticalScale };

    float fSourceXOffset{ 0.f };
    float fSourceYOffset{ 0.f };

    if( fSourceWidth > fDestGraphicWidth )
    {
      // The source texture is wider than the destination, so calculate an offset for where the source should
      // be place in order to overlap the destination at each images' center points
      fSourceXOffset = ( fSourceWidth - fDestGraphicWidth ) / 2.f;
      fSourceWidth = fDestGraphicWidth;
    }

    if( fSourceHeight > fDestGraphicHeight )
    {
      // The source texture is taller than the destination
      fSourceYOffset = ( fSourceHeight - fDestGraphicHeight ) / 2.f;
      fSourceHeight = fDestGraphicHeight;
    }

    // Source offsets
    float i_fSrcX1{ i_rcDest.m_iX / fDestGraphicWidth };
    float i_fSrcX2{ ( i_rcDest.m_iX + fSourceWidth ) / fDestGraphicWidth };
    rumNumberUtils::Clamp<float>( i_fSrcX2, 0.f, 1.f );

    // Note that this is flipped
    float i_fSrcY1{ ( i_rcDest.m_iY + fSourceHeight ) / fDestGraphicHeight };
    float i_fSrcY2{ i_rcDest.m_iY / fDestGraphicHeight };
    rumNumberUtils::Clamp<float>( i_fSrcY1, 0.f, 1.f );

    // Copy only the part of the destination texture that will be blended
    TextureQuad( (GLuint)(size_t)i_rcDestGraphic.GetData(), i_fSrcX1, i_fSrcY1, i_fSrcX2, i_fSrcY2,
                 (GLfloat)i_rcDest.m_iX, (GLfloat)i_rcDest.m_iY,
                 i_rcDest.m_iX + fSourceWidth, i_rcDest.m_iY + fSourceHeight );

    // Source offsets
    i_fSrcX1 = i_rcSrc.m_iX / fSourceGraphicWidth;
    i_fSrcX2 = ( i_rcSrc.m_iX + i_uiSrcWidth ) / fSourceGraphicWidth;

    // Note that this is flipped
    i_fSrcY1 = ( i_rcSrc.m_iY + i_uiSrcHeight ) / fSourceGraphicHeight;
    i_fSrcY2 = i_rcSrc.m_iY / fSourceGraphicHeight;

    // Merge the source texture with the destination in the framebuffer
    TextureQuad( (GLuint)(size_t)i_rcSrcGraphic.GetData(),
                 i_fSrcX1, i_fSrcY1, i_fSrcX2, i_fSrcY2, 0.f - fSourceXOffset, 0.f - fSourceYOffset,
                 fSourceWidth, fSourceHeight );

    // Copy the results back to the original texture
    CopyFramebuffer( i_rcDestGraphic, rumPoint(), i_rcDest, (GLuint)fSourceWidth, (GLuint)fSourceHeight );

    DisableFramebuffer();
  }
  else
  {
    // Source offsets
    const float i_fSrcX1{ i_rcSrc.m_iX / fSourceGraphicWidth };
    const float i_fSrcX2{ ( i_rcSrc.m_iX + i_uiSrcWidth ) / fSourceGraphicWidth };

    const float i_fSrcY1{ i_rcSrc.m_iY / fSourceGraphicHeight };
    const float i_fSrcY2{ ( i_rcSrc.m_iY + i_uiSrcHeight ) / fSourceGraphicHeight };

    // Destination offsets
    const float i_fDestX1{ (float)i_rcDest.m_iX };
    const float i_fDestX2{ i_fDestX1 + i_uiSrcWidth * i_rcAttributes.m_fVerticalScale };

    const float i_fDestY1{ (float)i_rcDest.m_iY };
    const float i_fDestY2{ i_fDestY1 + i_uiSrcHeight * i_rcAttributes.m_fHorizontalScale };

    TextureQuad( (GLuint)(size_t)i_rcSrcGraphic.GetData(),
                 i_fSrcX1, i_fSrcY1, i_fSrcX2, i_fSrcY2,
                 i_fDestX1, i_fDestY1, i_fDestX2, i_fDestY2 );
  }
}


// static
void rumClientGraphic::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM )
    .Func( "rumGetMaxScreenResolution", GetMaxScreenResolution )
    .Func( "rumSetFullscreenMode", SetFullscreenMode )
    .Func( "rumGetResolutions", GetResolutions )
    .Func( "rumSetWindowedMode", SetWindowedMode )
    .Func( "rumSetWindowSize", SetWindowSize );

  Sqrat::DerivedClass<rumClientGraphic, rumClientGraphicBase> cClientGraphic( pcVM, "rumClientGraphic" );
  Sqrat::RootTable( pcVM ).Bind( SCRIPT_GRAPHIC_NATIVE_CLASS, cClientGraphic );
}


// static
void rumClientGraphic::SetFullscreenMode( uint32_t i_uiWidth, uint32_t i_uiHeight )
{
  s_bProgrammaticResizeRequested = true;

  s_uiFullScreenHeight = i_uiHeight;
  s_uiFullScreenWidth = i_uiWidth;

  if( !s_bFullscreen )
  {
    GLFWmonitor* pcMonitor{ glfwGetPrimaryMonitor() };
    if( pcMonitor != nullptr )
    {
      // Switch to full screen
      s_bFullscreen = true;

      glfwSetWindowMonitor( s_pcWindow, pcMonitor, 0, 0, i_uiWidth, i_uiHeight, GLFW_DONT_CARE );
    }
  }
  else
  {
    glfwSetWindowSize( s_pcWindow, i_uiWidth, i_uiHeight );
  }
}


// static
void rumClientGraphic::SetWindowedMode( uint32_t i_uiWidth, uint32_t i_uiHeight )
{
  s_bProgrammaticResizeRequested = true;

  s_uiWindowedScreenWidth = i_uiWidth;
  s_uiWindowedScreenHeight = i_uiHeight;

  if( s_bFullscreen )
  {
    // Restore last window size and position
    s_bFullscreen = false;

    // Note that we have to do both of these because the frame buffer adjusts after glfwSetWindowMonitor, which tends
    // to shave off a few pixels for some reason
    glfwSetWindowMonitor( s_pcWindow, nullptr, s_iWindowPosX, s_iWindowPosY, i_uiWidth, i_uiHeight, NULL );
    glfwSetWindowSize( s_pcWindow, s_uiWindowedScreenWidth, s_uiWindowedScreenHeight );
  }
  else
  {
    glViewport( 0, 0, i_uiWidth, i_uiHeight );
    glfwSetWindowMonitor( s_pcWindow, nullptr, s_iWindowPosX, s_iWindowPosY, i_uiWidth, i_uiHeight, NULL );
  }
}


// static
void rumClientGraphic::SetWindowSize( uint32_t i_uiWidth, uint32_t i_uiHeight )
{
  SetWindowedMode( i_uiWidth, i_uiHeight );
}


// static
void rumClientGraphic::SetWindowTitle( const std::string& i_strTitle )
{
  glfwSetWindowTitle( s_pcWindow, i_strTitle.c_str() );
}


void rumClientGraphic::Shift( const rumVector& i_rcVector )
{
  int32_t iOffsetX{ (int32_t)i_rcVector.m_fX };
  int32_t iOffsetY{ (int32_t)i_rcVector.m_fY };

  if( ( 0 == iOffsetX ) && ( 0 == iOffsetY ) )
  {
    // Nothing to do
    return;
  }

  const GLuint uiHeight{ GetHeight() };
  const GLuint uiWidth{ GetWidth() };
  if( ( 0 == uiHeight ) || ( 0 == uiWidth ) )
  {
    // Guard against div by zero
    return;
  }

  iOffsetX %= uiWidth;
  iOffsetY %= uiHeight;

  rumAssert( s_pcTempGraphic[0]->GetWidth() >= GetWidth() );
  rumAssert( s_pcTempGraphic[0]->GetHeight() >= GetHeight() );

  // Create a quadrant of the texture to shift
  const rumPoint cZero;

  if( iOffsetY < 0 )
  {
    // Shift up
    iOffsetY = abs( iOffsetY );
    s_pcTempGraphic[0]->Blit( *this, cZero, { 0, iOffsetY }, uiWidth, uiHeight - iOffsetY );
    s_pcTempGraphic[0]->Blit( *this, { 0, (int32_t)uiHeight - iOffsetY }, { 0, 0 }, uiWidth, iOffsetY );

    Blit( *s_pcTempGraphic[0], cZero, cZero, uiWidth, uiHeight );
  }
  else if( iOffsetY > 0 )
  {
    // Shift down
    s_pcTempGraphic[0]->Blit( *this, cZero, { 0, (int32_t)uiHeight - iOffsetY }, uiWidth, iOffsetY );
    s_pcTempGraphic[0]->Blit( *this, { 0, iOffsetY }, cZero, uiWidth, uiHeight - iOffsetY );

    Blit( *s_pcTempGraphic[0], cZero, cZero, uiWidth, uiHeight );
  }

  if( iOffsetX < 0 )
  {
    // Shift left
    iOffsetX = abs( iOffsetX );
    s_pcTempGraphic[0]->Blit( *this, cZero, { (int32_t)uiWidth - iOffsetX, 0 }, iOffsetX, uiHeight );
    s_pcTempGraphic[0]->Blit( *this, { iOffsetX, 0 }, cZero, uiWidth - iOffsetX, uiHeight );

    Blit( *s_pcTempGraphic[0], cZero, cZero, uiWidth, uiHeight );
  }
  else if( iOffsetX > 0 )
  {
    // Shift right
    s_pcTempGraphic[0]->Blit( *this, cZero, { iOffsetX, 0 }, uiWidth - iOffsetX, uiHeight );
    s_pcTempGraphic[0]->Blit( *this, { (int32_t)uiWidth - iOffsetX, 0 }, cZero, iOffsetX, uiHeight );

    Blit( *s_pcTempGraphic[0], cZero, cZero, uiWidth, uiHeight );
  }
}


// static
void rumClientGraphic::Shutdown()
{
  RUM_COUT_IFDEF_DBG( GRAPHIC_DEBUG, "Shutting down GLFW Graphics handler\n" );

  for( uint32_t i = 0; i < 2; ++i )
  {
    if( s_hFramebufferTexture[i] != 0 )
    {
      glDeleteTextures( 1, &s_hFramebufferTexture[i] );
    }

    if( s_hFramebuffer[i] != 0 )
    {
      glBindFramebuffer( GL_FRAMEBUFFER_EXT, 0 );
      glDeleteFramebuffers( 1, &s_hFramebuffer[i] );
    }
  }

  for( uint32_t i = 0; i < 4; ++i )
  {
    if( s_pcTempGraphic[i] )
    {
      s_pcTempGraphic[i]->Free();
    }

    SAFE_ARRAY_DELETE( s_pcBuffer[i] );
  }

#ifdef _DEBUG
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
#endif

  glfwDestroyWindow( s_pcWindow );
  glfwTerminate();

  using namespace FreeImage;
  FreeImage_DeInitialise();

  super::Shutdown();
}


// static
void rumClientGraphic::TextureQuad( GLuint i_hTexture,
                                    float i_fSrcX1, float i_fSrcY1, float i_fSrcX2, float i_fSrcY2,
                                    float i_fDestX1, float i_fDestY1, float i_fDestX2, float i_fDestY2 )
{
  // TEMP! commented out for now
  /*
  rumAssert(i_fSrcX1 >= 0.f && i_fSrcX1 <= 1.f);
  rumAssert(i_fSrcX2 >= 0.f && i_fSrcX2 <= 1.f);
  rumAssert(i_fSrcY1 >= 0.f && i_fSrcY1 <= 1.f);
  rumAssert(i_fSrcY2 >= 0.f && i_fSrcY2 <= 1.f);
  */

  glEnable( GL_TEXTURE_2D );
  glBindTexture( GL_TEXTURE_2D, i_hTexture );

  glBegin( GL_QUADS );

  // Bottom left of texture
  glTexCoord2f( i_fSrcX1, i_fSrcY1 );
  glVertex2f( i_fDestX1, i_fDestY1 );

  // Bottom right of texture
  glTexCoord2f( i_fSrcX2, i_fSrcY1 );
  glVertex2f( i_fDestX2, i_fDestY1 );

  // Top right of texture
  glTexCoord2f( i_fSrcX2, i_fSrcY2 );
  glVertex2f( i_fDestX2, i_fDestY2 );

  // Top left of texture
  glTexCoord2f( i_fSrcX1, i_fSrcY2 );
  glVertex2f( i_fDestX1, i_fDestY2 );

  glEnd();

  glDisable( GL_TEXTURE_2D );
}


// static
void rumClientGraphic::WindowCloseCallback( GLFWwindow* i_pcWindow )
{
  glfwSetWindowShouldClose( i_pcWindow, GLFW_TRUE );
}


// static
bool rumClientGraphic::WindowClosed()
{
  return glfwWindowShouldClose( s_pcWindow );
}


// static
void rumClientGraphic::WindowContentScaleCallback( GLFWwindow* i_pcWindow, float i_fXScale, float i_fYScale )
{
  RUM_COUT_IFDEF_DBG( GRAPHIC_DEBUG, "GLFW content scale changed " << i_fXScale << ":" << i_fYScale << '\n' );
}


// static
void rumClientGraphic::WindowFocusCallback( GLFWwindow* i_pcWindow, int32_t i_iFocused )
{
#if GRAPHIC_DEBUG
  if( i_iFocused )
  {
    RUM_COUT_DBG( "GLFW window gained input focus\n" );
  }
  else
  {
    RUM_COUT_DBG( "GLFW window lost input focus\n" );
  }
#endif // GRAPHIC_DEBUG
}


// static
void rumClientGraphic::WindowMaximizedCallback( GLFWwindow* i_pcWindow, int32_t i_iMaximized )
{
#if GRAPHIC_DEBUG
  if( i_iMaximized )
  {
    RUM_COUT_DBG( "GLFW window maximized\n" );
  }
  else
  {
    RUM_COUT_DBG( "GLFW window restored\n" );
  }
#endif // GRAPHIC_DEBUG
}


// static
void rumClientGraphic::WindowPositionCallback( GLFWwindow* i_pcWindow, int32_t i_iXPos, int32_t i_iYPos )
{
  RUM_COUT_IFDEF_DBG( GRAPHIC_DEBUG, "GLFW window position changed " << i_iXPos << ", " << i_iYPos << '\n' );

  if( !s_bFullscreen )
  {
    s_iWindowPosX = i_iXPos;
    s_iWindowPosY = i_iYPos;
  }
}


// static
void rumClientGraphic::WindowRefreshCallback( GLFWwindow* i_pcWindow )
{
  RUM_COUT_IFDEF_DBG( GRAPHIC_DEBUG, "GLFW window must be refreshed\n" );
  glfwSwapBuffers( i_pcWindow );
}


// static
void rumClientGraphic::WindowSizeCallback( GLFWwindow* i_pcWindow, int32_t i_iWidth, int32_t i_iHeight )
{
  RUM_COUT_IFDEF_DBG( GRAPHIC_DEBUG, "GLFW window resized " << i_iWidth << " x  " << i_iHeight << '\n' );
}

#endif // USE_GLFW
