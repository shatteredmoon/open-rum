#include <controls/c_label.h>

#include <c_font.h>
#include <c_graphics.h>

#include <u_assert.h>

#include <queue>
#include <regex>


struct rumLabelToken
{
  std::string m_strToken;
  uint32_t m_uiWidth{ 0 };
};


rumClientLabel::~rumClientLabel()
{
  Clear();
}


void rumClientLabel::Clear()
{
  m_strText.clear();
  super::Clear();
}


void rumClientLabel::SetText( const std::string& i_strText )
{
  m_strText = i_strText;
  UpdateDisplay();
}


void rumClientLabel::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumClientLabel, rumClientControl> cClientLabel( pcVM, "rumClientLabel" );
  cClientLabel
    .Func( "GetText", &GetText )
    .Func( "SetText", &SetText )
    .Func( "IsEmpty", &IsEmpty );
  Sqrat::RootTable( pcVM ).Bind( "rumLabel", cClientLabel );

  rumScript::CreateClassScript( "Label", "rumLabel" );
}


void rumClientLabel::UpdateDisplay()
{
  m_pcDisplay->Clear( rumColor::s_cBlackTransparent );

  rumGraphic* pcGraphic{ rumGraphic::Create() };
  rumAssert( pcGraphic );

  pcGraphic->InitData( *m_pcDisplay );

  rumFont* pcFont{ GetFont() };
  if( !pcFont )
  {
    return;
  }

  rumColor cFontColor( pcFont->GetColor() );

  // Determine screen space needed
  uint32_t uiFontHeight{ pcFont->GetPixelHeight() };

  // The width of a single space will likely be used many times over
  uint32_t uiSpaceWidth{ pcFont->GetTextPixelWidth( " " ) };

  const uint32_t uiLineHeight{ this->GetHeight() };

  const rumGraphic* pcCursor{ nullptr };
  const rumGraphic* pcPrompt{ nullptr };

  if( HasCursor() )
  {
    pcCursor = FetchCursor();
  }

  if( HasPrompt() )
  {
    pcPrompt = FetchPrompt();
  }

  // Tokens that will be displayed
  std::queue<rumLabelToken> qTokens;

  // Regular Expressions ---------------------------------------------------

  // Tag
  static const std::regex reTag( "<.+?>" ); //non-greedy

  //static const std::wregex reToken(L".*([ \n\r\t]|<.*>)");
  static const std::regex reToken( "\\s" );

  // Repeating spaces
  static const std::regex reSpaces( "\\s{2,}" );
  static const std::string fmtSpaces( " " );

  // Spaces before punctuation
  static const std::regex rePunct( "\\s+([.,?:;!])" );
  static const std::string fmtPunct( "$1" );

  // Before parsing, make sure that tags are well-formed
  static const std::regex reColorHex( "<\\s*C\\s*#\\s*(([0-9]|[a-f]){6})\\s*>", std::regex::icase );
  static const std::string fmtColorHex( "<c#$1>" );

  static const std::regex reColorDec( "<\\s*C\\s*#\\s*(\\d{1,3})[,]\\s*(\\d{1,3})[,]\\s*(\\d{1,3})\\s*>",
                                       std::regex::icase );
  static const std::string fmtColorDec( "<c#$1,$2,$3>" );

  //static const std::regex reFontHex( "<\\s*F\\s*#\\s*([0-9]|[a-f])([0-9]|[a-f])?\\s*>", std::regex::icase );
  //static const std::string fmtFontHex( "<f#$1$2>" );

  static const std::regex reFont( "<\\s*F\\s*#\\s*(([0-9]|[a-z]){1,32})\\s*>", std::regex::icase );
  static const std::string fmtFontDec( "<f#$1>" );

  // Kill redundant spaces
  std::string strModifiedText{ regex_replace( m_strText, reSpaces, fmtSpaces ) };

  // Kill spaces before certain punctuations
  strModifiedText = regex_replace( strModifiedText, rePunct, fmtPunct );

  // Does the string contain embedded tags?
  if( regex_search( strModifiedText, reTag ) == true )
  {
    // Fix-up all color tags
    strModifiedText = regex_replace( strModifiedText, reColorHex, fmtColorHex );
    strModifiedText = regex_replace( strModifiedText, reColorDec, fmtColorDec );

    // Fix-up font tags
    strModifiedText = regex_replace( strModifiedText, reFont, fmtFontDec );
  }

  // Break text into tokens, space dilineated. The -1 in this call means:
  // return everything that doesn't match the expression, or in other words,
  // everything that isn't a space.
  std::sregex_token_iterator iter( strModifiedText.begin(), strModifiedText.end(), reToken, -1 );
  std::sregex_token_iterator end;
  std::smatch match;
  for( ; iter != end; ++iter )
  {
    // Create the token
    std::string::const_iterator strIter( iter->first );
    std::string strToken( iter->first, iter->second );

    // See if the token contains any tags
    while( regex_search( strIter, iter->second, match, reTag ) == true )
    {
      // See if there was legitimate text in a group of tags
      if( match[0].first != strIter )
      {
        rumLabelToken cLabelToken;
        cLabelToken.m_strToken = std::string( strIter, match[0].first );
        cLabelToken.m_uiWidth = pcFont->GetTextPixelWidth( cLabelToken.m_strToken );

        qTokens.push( cLabelToken );
      }

      // Take action on certain tags
      if( regex_match( match[0].first, match[0].second, reFont ) )
      {
        // Parse a font tag
        const std::string strFont( match[0].first + 3, match[0].second - 1 );
        pcFont = rumFont::Get( strFont );
        if( !pcFont )
        {
          pcFont = GetFont();
        }

        rumAssert( pcFont );
        if( pcFont )
        {
          uiFontHeight = pcFont->GetPixelHeight();
          uiSpaceWidth = pcFont->GetTextPixelWidth( " " );
        }
      }

      rumLabelToken cLabelToken;
      cLabelToken.m_strToken = std::string( match[0].first, match[0].second );

      qTokens.push( cLabelToken );
      strIter = match[0].second;
    }

    // Grab anything not processed by the tag handler. If there were no
    // tags in the token, then this should represent the entire token
    if( strIter != iter->second )
    {
      rumLabelToken cLabelToken;
      cLabelToken.m_strToken = std::string( strIter, iter->second );

      // Otherwise, determine the string length and add it to the queue
      cLabelToken.m_uiWidth = pcFont->GetTextPixelWidth( cLabelToken.m_strToken );

      qTokens.push( cLabelToken );
    }
  }

  // Reset the font
  pcFont = GetFont();
  rumAssert( pcFont );
  if( pcFont )
  {
    uiFontHeight = pcFont->GetPixelHeight();
    uiSpaceWidth = pcFont->GetTextPixelWidth( " " );
  }

  int32_t hOffset{ 0 };
  int32_t vOffset{ 0 };

  const rumPoint cZero;

  if( pcPrompt )
  {
    // A prompt graphic is used, so compute the vertical and horizontal offsets that it requires
    int32_t bvOffset{ vOffset };
    const uint32_t uiPromptHeight{ pcPrompt->GetHeight() };
    const uint32_t uiPromptWidth{ pcPrompt->GetWidth() };
    if( uiPromptHeight < uiLineHeight )
    {
      // The prompt needs to be centered vertically on this line
      bvOffset += ( uiLineHeight - uiPromptHeight ) / 2;
    }

    // Show the prompt
    pcGraphic->Blit( *pcPrompt, cZero, rumPoint( hOffset, bvOffset ), uiPromptWidth, uiPromptHeight );
    hOffset += uiPromptWidth;
  }

  // Begin displaying the added text
  while( !qTokens.empty() )
  {
    const rumLabelToken& rcCurrentToken{ qTokens.front() };
    const std::string& strToken{ rcCurrentToken.m_strToken };
    if( rcCurrentToken.m_uiWidth == 0 )
    {
      // Check regular expressions
      if( regex_match( strToken, match, reColorHex ) )
      {
        // Parse a color tag, expressed in hex
        char* strEnd{ nullptr };
        std::string strTag( match[0].first + 3, match[0].first + 5 );
        const uint32_t r{ strtoul( strTag.c_str(), &strEnd, 16 ) };
        strTag = std::string( match[0].first + 5, match[0].first + 7 );
        const uint32_t g{ strtoul( strTag.c_str(), &strEnd, 16 ) };
        strTag = std::string( match[0].first + 7, match[0].first + 9 );
        const uint32_t b{ strtoul( strTag.c_str(), &strEnd, 16 ) };
        cFontColor = rumColor( r, g, b );
      }
      else if( regex_match( strToken, match, reColorDec ) )
      {
        // Parse a color tag, expressed in decimal
        char* strEnd{ nullptr };
        const size_t uiOffset{ strToken.find_first_of( "#" ) + 1 };
        std::string strTag( strToken.substr( uiOffset ) );
        const uint32_t r{ strtoul( strTag.c_str(), &strEnd, 10 ) };
        strTag.erase( 0, strTag.find_first_of( "," ) + 1 );
        const uint32_t g{ strtoul( strTag.c_str(), &strEnd, 10 ) };
        strTag.erase( 0, strTag.find_first_of( "," ) + 1 );
        const uint32_t b{ strtoul( strTag.c_str(), &strEnd, 10 ) };
        cFontColor = rumColor( r, g, b );
      }
      else if( regex_match( strToken, match, reFont ) )
      {
        // Parse a font tag
        const std::string strFont( match[0].first + 3, match[0].second - 1 );
        pcFont = rumFont::Get( strFont );
        if( !pcFont )
        {
          pcFont = GetFont();
        }

        rumAssert( pcFont );
        if( pcFont )
        {
          uiFontHeight = pcFont->GetPixelHeight();
          uiSpaceWidth = pcFont->GetTextPixelWidth( " " );
        }
      }
    }
    else
    {
      uint32_t tvOffset{ static_cast<uint32_t>( vOffset ) };
      if( uiFontHeight < uiLineHeight )
      {
        // The current font is smaller that the largest font that will occur on this same line, so we need to offset a
        // little more
        tvOffset += uiLineHeight - uiFontHeight;
      }

      // Print this token
      if( pcFont )
      {
        pcFont->BlitText( *pcGraphic, strToken, rumPoint( hOffset, tvOffset ), false, &cFontColor );
      }

      // Advance past the token
      hOffset += rcCurrentToken.m_uiWidth + uiSpaceWidth;
    }

    qTokens.pop();
  }

  hOffset -= uiSpaceWidth;

  if( pcCursor )
  {
    m_cCursorOffset.m_iY = vOffset;

    if( pcCursor->GetFrameHeight() < GetHeight() )
    {
      // The cursor needs to be centered vertically on this line
      m_cCursorOffset.m_iY = ( GetHeight() - pcCursor->GetFrameHeight() ) / 2;
    }

    m_cCursorOffset.m_iX = hOffset;
    hOffset += pcCursor->GetWidth();
  }

  // Blit the final label
  switch( m_eAlignment )
  {
    case ALIGN_LEFT:
      m_pcDisplay->Blit( *pcGraphic, cZero, cZero, hOffset, pcGraphic->GetHeight() );
      break;

    case ALIGN_RIGHT:
      m_pcDisplay->Blit( *pcGraphic, cZero, rumPoint( pcGraphic->GetWidth() - hOffset, 0 ),
                         hOffset, pcGraphic->GetHeight() );
      if( pcCursor )
      {
        m_cCursorOffset.m_iX = pcGraphic->GetWidth() - pcCursor->GetWidth();
      }
      break;

    case ALIGN_CENTER:
      m_pcDisplay->Blit( *pcGraphic, cZero, rumPoint( ( pcGraphic->GetWidth() / 2 - hOffset / 2 ), 0 ),
                         hOffset, pcGraphic->GetHeight() );
      if( pcCursor )
      {
        m_cCursorOffset.m_iX = ( pcGraphic->GetWidth() / 2 - hOffset / 2 ) + hOffset - pcCursor->GetWidth();
      }
      break;
  }

  pcGraphic->Free();
}


bool rumClientLabel::Validate()
{
  bool bResult{ super::Validate() };
  UpdateDisplay();
  return bResult;
}
