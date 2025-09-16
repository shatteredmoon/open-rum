#ifndef _C_FONT_H_
#define _C_FONT_H_

#include <u_structs.h>
#include <c_graphics.h>

#define USE_FONT_SYSTEM_FREETYPE

/*
Notes on font properties
1. blendFace is only applicable with antialias=true
2. Use blendFace=false for a pseudo-outline (without the outline)
3. blendFace=false with a backgroundColor set to the faceColor is identical to
   setting antialias=false
*/
struct rumFontAttributes
{
  uint32_t m_uiPixelHeight{ 16 };
  uint32_t m_uiWidestPixelWidth{ 0 };
  int32_t m_iFaceIndex{ 0 };
  int32_t m_iOutlinePixelWidth{ 1 };
  int32_t m_iAscent{ 0 };
  int32_t m_iDescent{ 0 };
  bool m_bOutlined{ false };
  bool m_bAntialias{ true };
  bool m_bBlendFace{ true };
  rumColor m_cFaceColor{ rumColor::s_cWhite };
  rumColor m_cOutlineColor{ rumColor::s_cBlack };
  rumColor m_cBackgroundColor{ rumColor::s_cBlack };
};


struct rumCodePoint
{
  uint32_t m_uiCodePoint{ 0 };
  uint32_t m_uiNumBytes{ 0 };

  bool IsValid() const
  {
    return m_uiNumBytes > 0 && m_uiNumBytes <= 4 &&
      m_uiNumBytes == 1 ? m_uiCodePoint >= 0x00000 && m_uiCodePoint <= 0x00007F :
      m_uiNumBytes == 2 ? m_uiCodePoint >= 0x00080 && m_uiCodePoint <= 0x0007FF :
      m_uiNumBytes == 3 ? m_uiCodePoint >= 0x00800 && m_uiCodePoint <= 0x00FFFF :
      m_uiNumBytes == 4 ? m_uiCodePoint >= 0x10000 && m_uiCodePoint <= 0x10FFFF :
      false;
  }
};


struct rumGlyphStruct;


class rumFont
{
public:

  int32_t CreateGlyph( uint32_t i_uiCodePoint );

  const rumColor& GetColor() const
  {
    return m_cAttributes.m_cFaceColor;
  }

  uint32_t GetPixelHeight() const
  {
    return m_cAttributes.m_uiPixelHeight;
  }

  // TODO - would be nice if this was const, but it can't because it may need to create any missing glyphs
  uint32_t GetTextPixelWidth( const std::string& i_strText );

  bool GlyphExists( uint32_t i_uiCodePoint ) const;

  int32_t BlitText( rumGraphic& i_rcGraphic, const std::string& i_strText, const rumPoint& i_rcDestPos,
                    bool i_bAlphaPreserve = false, const rumColor* i_pcColor = nullptr );

  void Release();

  static bool Create( const std::string& i_strFile, const std::string& i_strKey,
                      const rumFontAttributes& i_rcFontAttributes );
  static bool Exists( const std::string& i_strKey )
  {
    return ( s_hashFonts.find( i_strKey ) != s_hashFonts.end() );
  }
  static rumFont* GetDefault()
  {
    return Get( GetDefaultFontName() );
  }
  static rumFont* Get( const std::string& i_strKey );
  static const std::string& GetDefaultFontName()
  {
    return s_strDefault;
  }

  static int32_t Init();

  static void Release( const std::string& i_strKey );
  static void ScriptBind();
  static bool SetDefault( const std::string& i_strKey );
  static void Shutdown();

  static uint32_t GetFontTextHeight( const std::string& i_strFont );
  static uint32_t GetFontTextWidth( const std::string& i_strFont, const std::string& i_strText );

  typedef std::unordered_map<std::string, rumFont*> FontHash;

  // A glyphContainer is a set of glyphs with the unicode code point represented by each used as a key
  typedef std::unordered_map<uint32_t, rumGlyphStruct*> GlyphHash;

private:

  static bool CreateApp( const std::string& i_strFile, const std::string& i_strKey,
                         const rumFontAttributes& i_rcFontAttributes );

  static rumCodePoint GetCodePointFromUTF8String( const std::string_view i_strUTF8 );

  static int32_t InitApp();
  static bool Manage( rumFont* i_pcFont, const std::string& i_strKey );
  static void ShutdownApp();

  static FontHash s_hashFonts;
  static std::string s_strDefault;

  // Scratch surfaces for graphic operations
  static rumGraphic* s_pcSurface1;
  static rumGraphic* s_pcSurface2;

  int32_t AddGlyph( uint32_t i_uiCodePoint, const rumColor* i_pcColor, uint32_t i_uiWidth, uint32_t i_uiHeight,
                    int32_t i_uiAdvance, int32_t i_uiAscent );

  void ReleaseApp();

  void* m_pcFaceData{ nullptr };

  rumFontAttributes m_cAttributes;
  GlyphHash m_hashGlyphs;
};

#endif // _C_FONT_H_
