#include <e_graphics_opengl.h>

#include <u_utility.h>

#ifdef WIN32
#include <windows.h>
#endif

#include <gl/gl.h>
#include <gl/glu.h>

// Namespace required because of typedef BOOL conflict
namespace FreeImage
{
#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#include <freeimage.h>
}

#include <QOpenGLTexture>


EditorGraphic::~EditorGraphic()
{
  FreeInternal();
}


// virtual
void EditorGraphic::Draw( const rumPoint& i_rcPos ) const
{
  glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );

  const float fOffsetY1{ (float)i_rcPos.m_iY };
  const float fOffsetY2{ fOffsetY1 + GetHeight() * m_cGraphicAttributes.m_fHorizontalScale };

  const float fOffsetX1{ (float)i_rcPos.m_iX };
  const float fOffsetX2{ fOffsetX1 + GetWidth() * m_cGraphicAttributes.m_fVerticalScale };

  glBegin( GL_QUADS );

  // Top left of texture
  glTexCoord2f( 0.0, 1.0 );
  glVertex2f( fOffsetX1, fOffsetY1 );

  // Top right of texture
  glTexCoord2f( 1.0, 1.0 );
  glVertex2f( fOffsetX2, fOffsetY1 );

  // Bottom right of texture
  glTexCoord2f( 1.0, 0.0 );
  glVertex2f( fOffsetX2, fOffsetY2 );

  // Bottom left of texture
  glTexCoord2f( 0.0, 0.0 );
  glVertex2f( fOffsetX1, fOffsetY2 );

  glEnd();
}


// virtual
void EditorGraphic::Draw( const rumPoint& i_rcDest, const rumPoint& i_rcSrc,
                          uint32_t i_uiWidth, uint32_t i_uiHeight ) const
{
  glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );

  // Y offset
  const float fY1{ (float)i_rcDest.m_iY };
  const float fY2{ fY1 + i_uiHeight * m_cGraphicAttributes.m_fHorizontalScale };

  // X offset
  const float fX1{ (float)i_rcDest.m_iX };
  const float fX2{ fX1 + i_uiWidth * m_cGraphicAttributes.m_fVerticalScale };

  // Offset percentage
  const float fOffsetPercentX1{ i_rcSrc.m_iX / (float)GetWidth() };
  const float fOffsetPercentX2{ ( i_rcSrc.m_iX + i_uiWidth ) / (float)GetWidth() };

  const float fOffsetPercentY1{ i_rcSrc.m_iY / (float)GetHeight() };
  const float fOffsetPercentY2{ ( i_rcSrc.m_iY + i_uiHeight ) / (float)GetHeight() };

  if( m_cGraphicAttributes.m_uiTransparentLevel != RUM_ALPHA_OPAQUE )
  {
    glPushAttrib( GL_CURRENT_BIT );
    glEnable( GL_BLEND );
    glColor4f( 1.0f, 1.0f, 1.0f, m_cGraphicAttributes.m_uiTransparentLevel / (float)RUM_ALPHA_OPAQUE );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );
  }

  glBegin( GL_QUADS );

  // Top left of texture
  glTexCoord2f( fOffsetPercentX1, fOffsetPercentY2 );
  glVertex2f( fX1, fY1 );

  // Top right of texture
  glTexCoord2f( fOffsetPercentX2, fOffsetPercentY2 );
  glVertex2f( fX2, fY1 );

  // Bottom right of texture
  glTexCoord2f( fOffsetPercentX2, fOffsetPercentY1 );
  glVertex2f( fX2, fY2 );

  // Bottom left of texture
  glTexCoord2f( fOffsetPercentX1, fOffsetPercentY1 );
  glVertex2f( fX1, fY2 );

  glEnd();

  if( m_cGraphicAttributes.m_uiTransparentLevel != RUM_ALPHA_OPAQUE )
  {
    glDisable( GL_BLEND );
    glPopAttrib();
  }
}


// override
void EditorGraphic::Free()
{
  FreeInternal();
  super::Free();
}


void EditorGraphic::FreeInternal()
{
  // Graphic memory is stored internally in OpenGL
  if( m_pcData != nullptr )
  {
    const GLuint hTexture{ (GLuint)(size_t)GetData() };
    glDeleteTextures( 1, &hTexture );

    m_pcData = nullptr;
  }
}


// static
void EditorGraphic::EnableBlending( bool i_bEnable )
{
  if( i_bEnable )
  {
    glEnable( GL_BLEND );
    glTexEnvf( GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    // Note, GL_ONE does ghost style blending
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
  }
  else
  {
    glDisable( GL_BLEND );
  }
}


// virtual
uint32_t EditorGraphic::GetHeight() const
{
  glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );

  GLint iHeight;
  glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &iHeight );

  return (uint32_t)iHeight;
}


// virtual
uint32_t EditorGraphic::GetWidth() const
{
  glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );

  GLint iWidth;
  glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &iWidth );

  return (uint32_t)iWidth;
}


// override
bool EditorGraphic::InitData( uint32_t i_uiWidth, uint32_t i_uiHeight )
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

    const BYTE* piBits{ new BYTE[i_uiWidth * i_uiHeight * 4] };
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, i_uiWidth, i_uiHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, piBits );
    glBindTexture( GL_TEXTURE_2D, 0 );
  }

  return true;
}


// override
bool EditorGraphic::InitData()
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
  if( !m_pcData )
  {
    rumAssert( false );
    const GLenum eError{ glGetError() };
    RUM_COUT_IFDEF_DBG( GRAPHIC_DEBUG, "OpenGL error: " << rumStringUtils::ToHexString( eError ) << '\n' );
  }

  const uint32_t uiNumBytes{ m_pcAsset->GetDataAllocSize() };

  FIMEMORY* pcStream{ FreeImage_OpenMemory( (BYTE*)m_pcAsset->GetData(), uiNumBytes ) };
  if( pcStream )
  {
    const FREE_IMAGE_FORMAT eFormat{ FreeImage_GetFileTypeFromMemory( pcStream, uiNumBytes ) };
    FIBITMAP* pcBitmap{ FreeImage_LoadFromMemory( eFormat, (FIMEMORY*)pcStream ) };
    if( pcBitmap )
    {
      // Now, there is no guarantee that the image file loaded will be GL_RGB, so we force FreeImage to convert the
      // image to GL_RGB.
      pcBitmap = FreeImage_ConvertTo32Bits( pcBitmap );

      glBindTexture( GL_TEXTURE_2D, hTexture );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
      //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0 );
      //glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0 );

      const uint32_t uiWidth{ FreeImage_GetWidth( pcBitmap ) };
      const uint32_t uiHeight{ FreeImage_GetHeight( pcBitmap ) };
      const uint32_t uiSize{ uiWidth * uiHeight };

      // Firstly, allocate the new bit data for the image.
      //rumAssert( uiSize <= ( 2048 * 2048 ) );
      //BYTE* pcNewImage = s_pcBuffer[0];
      BYTE* piBits{ new BYTE[FreeImage_GetWidth( pcBitmap ) * FreeImage_GetHeight( pcBitmap ) * 4] };

      // Get a pointer to FreeImage's data.
      const BYTE* piImage{ (BYTE*)FreeImage_GetBits( pcBitmap ) };

      // Iterate through the pixels, copying the data from 'pixels' to 'pNewImage' except in RGB format
      for( uint32_t i = 0; i < uiSize; ++i )
      {
        const uint32_t uiOffset{ i * 4 };
        piBits[uiOffset + 0] = piImage[uiOffset + 0];
        piBits[uiOffset + 1] = piImage[uiOffset + 1];
        piBits[uiOffset + 2] = piImage[uiOffset + 2];

        // TEMP! The original pcx assets used hot pink as the transparency color, so we detect that here
        if( ( piBits[uiOffset + 0] == 0xff ) && ( piBits[uiOffset + 1] == 0x00 ) && ( piBits[uiOffset + 2] == 0xff ) )
        {
          // Alpha full
          piBits[uiOffset + 3] = 0x00;
        }
        else
        {
          // Opaque
          piBits[uiOffset + 3] = 0xff;
        }
      }

      // The new 'glTexImage2D' function, the prime difference
      // being that it gets the width, height and pixel information
      // from 'pNewImage', which is the RGB pixel data..
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, uiWidth, uiHeight, 0, QOpenGLTexture::BGRA, GL_UNSIGNED_BYTE, piBits );

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

  CalcAnimation();

  return bSuccess;
}


// override
bool EditorGraphic::InitData( const rumColor* i_pcImage, uint32_t i_uiWidth, uint32_t i_uiHeight )
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


// static
void EditorGraphic::Init()
{
  using namespace FreeImage;

  FreeImage_Initialise();

  RUM_COUT( "Using: FreeImage " << FreeImage_GetVersion() << '\n' );
  RUM_COUT( FreeImage_GetCopyrightMessage() << '\n' );

  super::Init( true /* force creation of all graphics */ );
}


// static
void EditorGraphic::ScriptBind()
{
  super::ScriptBind();

  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<EditorGraphic, EditorGraphicBase> cEditorGraphic( pcVM, "EditorGraphic" );
  Sqrat::RootTable( pcVM ).Bind( SCRIPT_GRAPHIC_NATIVE_CLASS, cEditorGraphic );
}


void EditorGraphic::Shift( const rumVector& i_rcVector )
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
  const GLuint uiSize{ uiHeight * uiWidth };
  if( uiSize == 0 )
  {
    // Guard against dive by zero
    return;
  }

  glBindTexture( GL_TEXTURE_2D, (GLuint)(size_t)GetData() );

  GLubyte* piPixels{ new GLubyte[uiSize * 4] };
  GLubyte* piPixels2{ new GLubyte[uiSize * 4] };
  glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, piPixels );

  const GLuint iRowSize{ sizeof( GLubyte ) * uiWidth * 4 };

  iOffsetX = iOffsetX % uiWidth;
  iOffsetY = iOffsetY % uiHeight;

  if( iOffsetY < 0 )
  {
    // Shift up
    iOffsetY = abs( iOffsetY );
    memcpy( piPixels2 + ( iRowSize * iOffsetY ), piPixels, iRowSize * ( uiHeight - iOffsetY ) );
    memcpy( piPixels2, piPixels + ( iRowSize * ( uiHeight - iOffsetY ) ), iRowSize * iOffsetY );
  }
  else if( iOffsetY > 0 )
  {
    // Shift down
    memcpy( piPixels2, piPixels + ( iRowSize * iOffsetY ), iRowSize * ( uiHeight - iOffsetY ) );
    memcpy( piPixels2 + iRowSize * ( uiHeight - iOffsetY ), piPixels, iRowSize * iOffsetY );
  }

  if( iOffsetX < 0 )
  {
    if( iOffsetY != 0 )
    {
      memcpy( piPixels, piPixels2, sizeof( GLubyte ) * uiSize * 4 );
    }

    const int32_t iOffsetSize{ iOffsetX * 4 };

    // Shift left
    iOffsetX = abs( iOffsetX );
    for( int32_t i = 0; i < (int32_t)uiHeight; ++i )
    {
      const int32_t iOffset{ (int32_t)iRowSize * i };
      memcpy( piPixels2 + iOffset, piPixels + iOffset + iOffsetSize, iRowSize - iOffsetSize );
      memcpy( piPixels2 + iOffset + ( iRowSize - iOffsetSize ), piPixels + iOffset, iOffsetSize );
    }
  }
  else if( iOffsetX > 0 )
  {
    if( iOffsetY != 0 )
    {
      memcpy( piPixels, piPixels2, sizeof( GLubyte ) * uiSize * 4 );
    }

    const int32_t iOffsetSize{ iOffsetX * 4 };

    // Shift right
    for( int32_t i = 0; i < (int32_t)uiHeight; ++i )
    {
      const int32_t iOffset{ (int32_t)iRowSize * i };
      memcpy( piPixels2 + iOffset + iOffsetSize, piPixels + iOffset, iRowSize - iOffsetSize );
      memcpy( piPixels2 + iOffset, piPixels + iOffset + ( iRowSize - iOffsetSize ), iOffsetSize );
    }
  }

  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, uiWidth, uiHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, piPixels2 );

  delete[] piPixels;
  delete[] piPixels2;
}


// static
void EditorGraphic::Shutdown()
{
  using namespace FreeImage;

  RUM_COUT_IFDEF( GRAPHIC_DEBUG, "Shutting down OpenGL\n" );
  FreeImage_DeInitialise();

  super::Shutdown();
}
