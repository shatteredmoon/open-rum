#include <c_font.h>

#include <u_log.h>

#include <filesystem>

// Static initialization
rumFont::FontHash rumFont::s_hashFonts;
std::string rumFont::s_strDefault{ "" };
rumGraphic* rumFont::s_pcSurface1{ nullptr };
rumGraphic* rumFont::s_pcSurface2{ nullptr };


// A rumGlyphStruct holds the graphic and size information of one font character.
// Many characters in a font make up a single glyphContainer.
struct rumGlyphStruct
{
  rumGraphic* m_pcGraphic{ nullptr };
  int32_t m_iAdvance{ 0 };
  int32_t m_iVerticalOffset{ 0 };
};


int32_t rumFont::AddGlyph( uint32_t i_uiCodePoint, const rumColor* i_pcColor, uint32_t i_uiWidth, uint32_t i_uiHeight,
                           int32_t i_iAdvance, int32_t i_iAscent )
{
  if( !i_pcColor )
  {
    return RESULT_FAILED;
  }

  int32_t eResult{ RESULT_FAILED };

  if( !GlyphExists( i_uiCodePoint ) )
  {
    // Create the new graphic
    rumGraphic* pcGraphic{ rumGraphic::Create() };
    if( pcGraphic )
    {
      if( pcGraphic->InitData( i_pcColor, i_uiWidth, i_uiHeight ) )
      {
        // Add the glyph to the fontset
        rumGlyphStruct* pcGlyph{ new rumGlyphStruct };
        if( pcGlyph )
        {
          pcGlyph->m_pcGraphic = pcGraphic;
          pcGlyph->m_iAdvance = i_iAdvance;
          pcGlyph->m_iVerticalOffset = m_cAttributes.m_iAscent - i_iAscent;

          const auto cPair{ m_hashGlyphs.insert( std::make_pair( i_uiCodePoint, pcGlyph ) ) };
          if( cPair.second )
          {
            if( i_uiWidth > m_cAttributes.m_uiWidestPixelWidth )
            {
              // The font going into the set is the widest yet
              m_cAttributes.m_uiWidestPixelWidth = i_uiWidth;
            }

            eResult = RESULT_SUCCESS;
          }
          else
          {
            delete pcGlyph;
          }
        }
      }

      if( eResult != RESULT_SUCCESS )
      {
        pcGraphic->Free();
      }
    }
  }

  return eResult;
}


int32_t rumFont::BlitText( rumGraphic& i_rcGraphic, const std::string& i_strText, const rumPoint& i_rcDestPos,
                           bool i_bAlphaPreserve, const rumColor* i_pcColor)
{
  int32_t eResult{ RESULT_FAILED };

  rumPoint cDestPos( i_rcDestPos );
  const rumPoint cZero( 0, 0 );

  int32_t iYOffset{ 0 };

  bool bChangeColor{ false };

  // Determine if the color change is necessary
  if( i_pcColor )
  {
    if( m_cAttributes.m_cFaceColor.GetRGBA() != i_pcColor->GetRGBA() )
    {
      bChangeColor = true;
    }
  }
  else
  {
    i_pcColor = &rumColor::s_cWhite;
  }

  rumGraphic* pcGlyph{ nullptr };
  const rumColor BlendColor( *i_pcColor );

  // A solid color graphic used for alpha blending to achieve the desired target color
  s_pcSurface2->InitData( m_cAttributes.m_uiPixelHeight, m_cAttributes.m_uiWidestPixelWidth );
  s_pcSurface2->Clear( *i_pcColor );

  // Store the width of the widest glyph in case a new wider glyph is added
  // and the color graphic has to grow
  uint32_t uiWidestPixelWidth{ m_cAttributes.m_uiWidestPixelWidth };

  // Display each character
  rumCodePoint cCodePoint;
  for( uint32_t i = 0; i < i_strText.length(); cCodePoint.IsValid() ? i += cCodePoint.m_uiNumBytes : ++i )
  {
    cCodePoint = GetCodePointFromUTF8String( &i_strText[i] );

    // Does the glyph exist?
    GlyphHash::const_iterator iter( m_hashGlyphs.find( cCodePoint.m_uiCodePoint ) );
    if( iter == m_hashGlyphs.end() )
    {
      // Glyph has not been bitmapped
      if( CreateGlyph( cCodePoint.m_uiCodePoint ) == RESULT_SUCCESS )
      {
        iter = m_hashGlyphs.find( cCodePoint.m_uiCodePoint );
        if( uiWidestPixelWidth < m_cAttributes.m_uiWidestPixelWidth )
        {
          // Recreate the color graphic
          s_pcSurface2->InitData( m_cAttributes.m_uiPixelHeight, m_cAttributes.m_uiWidestPixelWidth );
          s_pcSurface2->Clear( *i_pcColor );
          uiWidestPixelWidth = m_cAttributes.m_uiWidestPixelWidth;
        }
      }
    }

    if( iter != m_hashGlyphs.end() )
    {
      // Glyph exists, just point to it
      pcGlyph = iter->second->m_pcGraphic;
      iYOffset = cDestPos.m_iY + iter->second->m_iVerticalOffset;
    }

    if( pcGlyph )
    {
      if( bChangeColor )
      {
        s_pcSurface1->InitData( *pcGlyph );

        // Blend the requested color with the font face
        s_pcSurface1->BlitMultiply( *s_pcSurface2, cZero, cZero, s_pcSurface1->GetWidth(), s_pcSurface1->GetHeight(),
                                   *i_pcColor );

        // Restore the alpha values from the original bitmap since they were altered during the color blend
        s_pcSurface1->CopyAlpha( *pcGlyph );

        pcGlyph = s_pcSurface1;
      }

      if( i_bAlphaPreserve )
      {
        // Only blit the most opaque pixels
        i_rcGraphic.BlitAlphaPreserve( *pcGlyph, rumPoint( cDestPos.m_iX, iYOffset ) );
      }
      else
      {
        i_rcGraphic.Blit( *pcGlyph, cZero, rumPoint( cDestPos.m_iX, iYOffset ),
                          pcGlyph->GetWidth(), pcGlyph->GetHeight() );
      }

      cDestPos.m_iX += iter->second->m_iAdvance;
    }

    pcGlyph = nullptr;
  }

  return eResult;
}


// static
bool rumFont::Create( const std::string& i_strFile, const std::string& i_strKey,
                      const rumFontAttributes& i_rcFontAttributes )
{
  // Make sure a font with this key doesn't already exist
  if( s_hashFonts.find( i_strKey ) != s_hashFonts.end() )
  {
    return false;
  }

  // Create a new FontObject
  const rumConfig& rcConfig{ GetConfig() };
  std::filesystem::path fsPath( rcConfig.m_strUUID );
  fsPath /= i_strFile;

  return CreateApp( fsPath.string().c_str(), i_strKey, i_rcFontAttributes );
}


// static
rumFont* rumFont::Get( const std::string& i_strKey )
{
  const FontHash::iterator& iter( s_hashFonts.find( i_strKey ) );
  return iter != s_hashFonts.end() ? iter->second : nullptr;
}


// static
rumCodePoint rumFont::GetCodePointFromUTF8String( const std::string_view i_strUTF8 )
{
  // NOTE: For UTF-8 encoding, see https://en.wikipedia.org/wiki/UTF-8
  rumCodePoint cCodePoint;

  if( i_strUTF8.empty() )
  {
    return cCodePoint;
  }

  const unsigned char cChar{ (const unsigned char)i_strUTF8[0] };

  if( ( cChar & 0x80 ) == 0x00 )
  {
    // U+0000 to U+007F
    cCodePoint.m_uiCodePoint = static_cast<uint32_t>( cChar & 0x7F );
    cCodePoint.m_uiNumBytes = 1;
  }
  else if( ( cChar & 0xE0 ) == 0xC0 )
  {
    // U+0080 to U+07FF
    cCodePoint.m_uiCodePoint = static_cast<uint32_t>( cChar & 0x1F ) << 6;
    const unsigned char cChar2{ (const unsigned char)i_strUTF8[1] };
    cCodePoint.m_uiCodePoint += static_cast<uint32_t>( cChar2 & 0x3F );
    cCodePoint.m_uiNumBytes = 2;
  }
  else if( ( cChar & 0xF0 ) == 0xE0 )
  {
    // U+0800 to U+FFFF
    cCodePoint.m_uiCodePoint = static_cast<uint32_t>( cChar & 0x1F ) << 12;
    const unsigned char cChar2{ (const unsigned char)i_strUTF8[1] };
    cCodePoint.m_uiCodePoint += static_cast<uint32_t>( cChar2 & 0x3F ) << 6;
    const unsigned char cChar3{ (const unsigned char)i_strUTF8[2] };
    cCodePoint.m_uiCodePoint += static_cast<uint32_t>( cChar3 & 0x3F );
    cCodePoint.m_uiNumBytes = 3;
  }
  else if( ( cChar & 0xF8 ) == 0xF0 )
  {
    // U+10000 to U+10FFFF
    cCodePoint.m_uiCodePoint = static_cast<uint32_t>( cChar & 0x1F ) << 18;
    const unsigned char cChar2{ (const unsigned char)i_strUTF8[1] };
    cCodePoint.m_uiCodePoint += static_cast<uint32_t>( cChar2 & 0x3F ) << 12;
    const unsigned char cChar3{ (const unsigned char)i_strUTF8[2] };
    cCodePoint.m_uiCodePoint += static_cast<uint32_t>( cChar3 & 0x3F ) << 6;
    const unsigned char cChar4{ (const unsigned char)i_strUTF8[3] };
    cCodePoint.m_uiCodePoint += static_cast<uint32_t>( cChar4 & 0x3F );
    cCodePoint.m_uiNumBytes = 4;
  }

  rumAssert( cCodePoint.IsValid() );
  return cCodePoint;
}


uint32_t rumFont::GetTextPixelWidth( const std::string& i_strText )
{
  uint32_t uiWidth{ 0 };
  rumCodePoint cCodePoint;

  for( uint32_t i = 0; i < i_strText.length(); )
  {
    cCodePoint = GetCodePointFromUTF8String( &i_strText[i] );

    // Does the bitmap glyph exist?
    const GlyphHash::const_iterator& iter( m_hashGlyphs.find( cCodePoint.m_uiCodePoint ) );
    if( iter != m_hashGlyphs.end() )
    {
      uiWidth += iter->second->m_iAdvance;
      cCodePoint.IsValid() ? i += cCodePoint.m_uiNumBytes : ++i;
    }
    else
    {
      // Glyph has not been created
      if( CreateGlyph( cCodePoint.m_uiCodePoint ) != RESULT_SUCCESS )
      {
        // Give up, move to the next one
        ++i;
      }
    }
  }

  return uiWidth;
}


bool rumFont::GlyphExists( uint32_t i_uiCodePoint ) const
{
  const GlyphHash::const_iterator& iter( m_hashGlyphs.find( i_uiCodePoint ) );
  return m_hashGlyphs.end() != iter;
}


// static
int32_t rumFont::Init()
{
  s_pcSurface1 = rumGraphic::Create();
  s_pcSurface2 = rumGraphic::Create();

  return s_pcSurface1 && s_pcSurface2 && InitApp();
}


// static
bool rumFont::Manage( rumFont* i_pcFont, const std::string& i_strKey )
{
  bool bManaged{ false };

  // Add the font to the container
  const auto cPair{ s_hashFonts.insert( std::make_pair( i_strKey, i_pcFont ) ) };
  if( cPair.second )
  {
    bManaged = true;

    // The very first font managed becomes the default font
    if( s_hashFonts.size() == 1 )
    {
      SetDefault( i_strKey );
    }
  }

  return bManaged;
}


void rumFont::Release()
{
  // Remove all glyphs
  for( auto& iter : m_hashGlyphs )
  {
    rumGlyphStruct* pcGlyph{ iter.second };
    pcGlyph->m_pcGraphic->Free();
    delete pcGlyph;
  }

  m_hashGlyphs.clear();

  ReleaseApp();
}


// static
void rumFont::Release( const std::string& i_strKey )
{
  const FontHash::iterator& iter( s_hashFonts.find( i_strKey ) );
  if( iter != s_hashFonts.end() )
  {
    iter->second->Release();
    s_hashFonts.erase( iter );
  }
}


// static
void rumFont::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::Class<rumFontAttributes> cFont( pcVM, "rumFontProps" );
  cFont
    .Var( "Antialias", &rumFontAttributes::m_bAntialias )
    .Var( "BackgroundColor", &rumFontAttributes::m_cBackgroundColor )
    .Var( "BlendFace", &rumFontAttributes::m_bBlendFace )
    .Var( "FaceColor", &rumFontAttributes::m_cFaceColor )
    .Var( "FaceIndex", &rumFontAttributes::m_iFaceIndex )
    .Var( "OutlineColor", &rumFontAttributes::m_cOutlineColor )
    .Var( "Outlined", &rumFontAttributes::m_bOutlined )
    .Var( "OutlineWidth", &rumFontAttributes::m_iOutlinePixelWidth )
    .Var( "PixelHeight", &rumFontAttributes::m_uiPixelHeight );
  Sqrat::RootTable( pcVM ).Bind( "rumFontProps", cFont );

  Sqrat::RootTable( pcVM )
    .Func( "rumCreateFont", Create )
    .Func( "rumGetTextHeight", GetFontTextHeight )
    .Func( "rumGetTextWidth", GetFontTextWidth );
}


uint32_t rumFont::GetFontTextHeight( const std::string& i_strFont )
{
  const rumFont* pcFont{ Get( i_strFont ) };
  return pcFont ? pcFont->GetPixelHeight() : 0;
}


uint32_t rumFont::GetFontTextWidth( const std::string& i_strFont, const std::string& i_strText )
{
  rumFont* pcFont{ Get( i_strFont ) };
  return pcFont ? pcFont->GetTextPixelWidth( i_strText ) : 0;
}


// static
bool rumFont::SetDefault( const std::string& i_strKey )
{
  if( Exists( i_strKey ) )
  {
    s_strDefault = i_strKey;
    return true;
  }

  return false;
}


// static
void rumFont::Shutdown()
{
  if( s_pcSurface1 )
  {
    s_pcSurface1->Free();
    s_pcSurface1 = nullptr;
  }

  if( s_pcSurface2 )
  {
    s_pcSurface2->Free();
    s_pcSurface2 = nullptr;
  }

  // Free font information
  for( auto& iter : s_hashFonts )
  {
    iter.second->Release();
  }

  s_hashFonts.clear();

  ShutdownApp();
}
