#include <controls/c_textview.h>

#include <c_graphics.h>
#include <c_font.h>

#include <u_assert.h>
#include <u_log.h>

#include <queue>
#include <regex>


struct rumViewportToken
{
  std::string m_strToken;
  uint32_t m_uiWidth{ 0 };
};


rumClientTextView::~rumClientTextView()
{
  Clear();
}


void rumClientTextView::BufferEnd()
{
  if( m_bNewestFirst )
  {
    super::BufferEnd();
  }
  else
  {
    const int32_t iMaxOffset{ static_cast<int32_t>( GetBufferPixelHeight() - GetHeight() ) };
    if( m_cBufferOffset.m_iY != iMaxOffset )
    {
      m_cBufferOffset.m_iY = iMaxOffset;
      UpdateDisplay();
    }
  }
}


void rumClientTextView::BufferHome()
{
  if( m_bNewestFirst )
  {
    super::BufferHome();
  }
  else
  {
    if( m_uiBufferPixelHeightUsed >= GetHeight() )
    {
      const int32_t iMaxOffset{ static_cast<int32_t>( GetBufferPixelHeight() - m_uiBufferPixelHeightUsed ) };
      if( m_cBufferOffset.m_iY != iMaxOffset )
      {
        m_cBufferOffset.m_iY = iMaxOffset;
        UpdateDisplay();
      }
    }
  }
}


void rumClientTextView::BufferPageDown()
{
  if( m_bNewestFirst )
  {
    super::BufferPageDown();
  }
  else
  {
    const int32_t iMaxOffset{ static_cast<int32_t>( GetBufferPixelHeight() - GetHeight() ) };
    if( m_cBufferOffset.m_iY != iMaxOffset )
    {
      m_cBufferOffset.m_iY += GetHeight();
      if( m_cBufferOffset.m_iY > iMaxOffset )
      {
        m_cBufferOffset.m_iY = iMaxOffset;
      }

      UpdateDisplay();
    }
  }
}


void rumClientTextView::BufferPageUp()
{
  if( m_bNewestFirst )
  {
    super::BufferPageUp();
  }
  else
  {
    if( m_uiBufferPixelHeightUsed >= GetHeight() )
    {
      const int32_t iMaxOffset{ static_cast<int32_t>( GetBufferPixelHeight() - m_uiBufferPixelHeightUsed ) };
      if( m_cBufferOffset.m_iY != iMaxOffset )
      {
        m_cBufferOffset.m_iY -= GetHeight();
        if( m_cBufferOffset.m_iY < iMaxOffset )
        {
          m_cBufferOffset.m_iY = iMaxOffset;
        }

        UpdateDisplay();
      }
    }
  }
}


uint32_t rumClientTextView::CalculateEntryHeight( const std::string& i_strText )
{
  return DrawEntry( i_strText, true /* i_bTestOnly */ );
}


uint32_t rumClientTextView::DrawEntry( const std::string& i_strText, bool i_bTestOnly )
{
  rumFont* pcFont{ GetFont() };
  if( !pcFont )
  {
    return 0;
  }

  rumColor cFontColor( pcFont->GetColor() );

  // Determine screen space needed
  uint32_t uiFontHeight{ pcFont->GetPixelHeight() };

  // The width of a single space will likely be used many times over
  uint32_t uiSpaceWidth{ pcFont->GetTextPixelWidth( " " ) };

  // Default the current lineHeight to that of the font
  uint32_t uiLineHeight{ uiFontHeight };

  // Running track of how much vertical space this entry will require
  uint32_t uiTotalEntryHeight{ 0 };

  // Running track of the width of the current line, used for detecting when word-wrap should occur
  uint32_t uiCurrentWidth{ 0 };

  const rumGraphic* pcCursor{ HasCursor() ? FetchCursor() : nullptr };

  // Tokens that will be displayed
  std::queue<rumViewportToken> qTokens;

  // The max height a pre-processed line achieved
  std::queue<uint32_t> qLineHeights;

  // Regular Expressions ---------------------------------------------------

  // Tag
  static const std::regex reTag( "<.+?>" ); //non-greedy

  //static const std::regex reToken( ".*([ \n\r\t]|<.*>)" );
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

  static const std::regex reBreak( "<\\s*B\\s*>", std::regex::icase );
  static const std::string fmtBreakDec( "<b>" );

  static const std::regex reGraphic( "<g#(([0-9]|[a-z]|[A-Z]|[_]){1,127})([:].*)*>", std::regex::icase );

  // Kill redundant spaces
  std::string cstr{ regex_replace( i_strText, reSpaces, fmtSpaces ) };

  // Kill spaces before certain punctuations
  cstr = regex_replace( cstr, rePunct, fmtPunct );

  // Does the string contain embedded tags?
  if( regex_search( cstr, reTag ) )
  {
    // Fix-up all color tags
    cstr = regex_replace( cstr, reColorHex, fmtColorHex );
    cstr = regex_replace( cstr, reColorDec, fmtColorDec );

    // Fix-up font tags
    cstr = regex_replace( cstr, reFont, fmtFontDec );

    // Fix up break tags
    cstr = regex_replace( cstr, reBreak, fmtBreakDec );
  }

  // Break text into tokens, space dilineated. The -1 in this call means:
  // return everything that doesn't match the expression, or in other words,
  // everything that isn't a space.
  std::sregex_token_iterator p( cstr.begin(), cstr.end(), reToken, -1 );
  std::sregex_token_iterator end;
  std::smatch match;
  for( ; p != end; ++p )
  {
    // Create the token
    std::string::const_iterator start( p->first );
    std::string token( p->first, p->second );

    // Debug
    //std::wcout << token << "*" << endl;

    // See if the token contains any tags
    while( regex_search( start, p->second, match, reTag ) )
    {
      // See if there was legitimate text in a group of tags
      if( match[0].first != start )
      {
        // Debug
        //std::cout << token << "**" << endl;

        rumViewportToken cViewportToken;
        cViewportToken.m_strToken = std::string( start, match[0].first );
        cViewportToken.m_uiWidth = pcFont->GetTextPixelWidth( cViewportToken.m_strToken );
        if( uiCurrentWidth + cViewportToken.m_uiWidth > m_uiScrollbarOffset )
        {
          // Wrap to the next line
          rumViewportToken cViewportToken2;
          cViewportToken2.m_strToken = "\n";
          qTokens.push( cViewportToken2 );
          uiCurrentWidth = 0;
          qLineHeights.push( uiLineHeight );
          uiTotalEntryHeight += uiLineHeight;
        }

        qTokens.push( cViewportToken );
        uiCurrentWidth += cViewportToken.m_uiWidth + uiSpaceWidth;
      }

      // Take action on certain tags
      if( regex_match( match[0].first, match[0].second, reFont ) )
      {
        // Parse a font tag
        const std::string strTag( match[0].first + 3, match[0].second - 1 );
        pcFont = rumFont::Get( strTag );
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

        uiLineHeight = std::max( uiLineHeight, uiFontHeight );
      }
      else if( regex_match( match[0].first, match[0].second, reBreak ) )
      {
        // Wrap to the next line
        rumViewportToken cViewportToken2;
        cViewportToken2.m_strToken = "\n";
        qTokens.push( cViewportToken2 );
        uiCurrentWidth = 0;
        qLineHeights.push( uiLineHeight );
        uiTotalEntryHeight += uiLineHeight;
      }
      else if( regex_match( match[0].first, match[0].second, reGraphic ) )
      {
        // Parse a graphic tag
        const std::string strTag( match[0].first + 3, match[0].second - 1 );
        const std::string::size_type iOptionals{ strTag.find_first_of( ":" ) };
        const std::string strGraphic{ strTag.substr( strTag.find_first_of( "#" ) + 1, iOptionals ) };
        std::string strOptionals{ "" };
        if( iOptionals != std::string::npos )
        {
          strOptionals = strTag.substr( iOptionals );
        }

        const rumGraphic* pcGraphic{ rumGraphic::Fetch( strGraphic ) };
        if( pcGraphic )
        {
          uint32_t uiWidth{ pcGraphic->GetFrameWidth() };
          uint32_t uiHeight{ pcGraphic->GetFrameHeight() };

          // Tokenize and process any optional graphics modifiers (vcenter, h###, and w###)
          if( strOptionals.length() > 0 )
          {
            char* strContext{ nullptr };
            const char* strToken{ strtok( &strOptionals[0], ":" ) };
            while( strToken )
            {
              if( strlen( strToken ) > 1 )
              {
                if( strToken[0] == 'h' )
                {
                  char* endptr{ nullptr };
                  uiHeight = strtol( &token[1], &endptr, 10 );
                }
                else if( strToken[0] == 'w' )
                {
                  char* endptr{ nullptr };
                  uiWidth = strtol( &token[1], &endptr, 10 );
                }
              }

              // Get next token
              strToken = strtok( nullptr, ":" );
            }
          }

          uiLineHeight = std::max( uiLineHeight, uiHeight );
          if( uiCurrentWidth + uiWidth > m_uiScrollbarOffset )
          {
            // Wrap to the next line
            rumViewportToken cViewportToken2;
            cViewportToken2.m_strToken = "\n";
            qTokens.push( cViewportToken2 );
            uiCurrentWidth = 0;
            qLineHeights.push( uiLineHeight );
            uiTotalEntryHeight += uiLineHeight;
          }

          uiCurrentWidth += uiWidth + uiSpaceWidth;
        }
      }

      rumViewportToken cViewportToken;
      cViewportToken.m_strToken = std::string( match[0].first, match[0].second );

      // Debug
      //std::wcout << vt.m_strToken << "**" << endl;

      qTokens.push( cViewportToken );
      start = match[0].second;
    }

    // Grab anything not processed by the tag handler. If there were no
    // tags in the token, then this should represent the entire token
    if( start != p->second )
    {
      rumViewportToken cViewportToken;
      cViewportToken.m_strToken = std::string( start, p->second );

      // Debug
      //std::wcout << vt.m_strToken << "**" << endl;

      // Otherwise, determine the string length and add it to the queue
      cViewportToken.m_uiWidth = pcFont->GetTextPixelWidth( cViewportToken.m_strToken );
      if( uiCurrentWidth + cViewportToken.m_uiWidth > m_uiScrollbarOffset )
      {
        // Wrap to the next line
        rumViewportToken cViewportToken2;
        cViewportToken2.m_strToken = "\n";
        qTokens.push( cViewportToken2 );
        uiCurrentWidth = 0;
        qLineHeights.push( uiLineHeight );
        uiTotalEntryHeight += uiLineHeight;
      }

      qTokens.push( cViewportToken );
      uiCurrentWidth += cViewportToken.m_uiWidth + uiSpaceWidth;
    }
  }

  if( m_bNewestFirst && !i_bTestOnly )
  {
    // Save the cursor position now since we know where it's going to print to the screen
    m_cCursorOffset.m_iY = uiTotalEntryHeight;
  }

  uiTotalEntryHeight += uiLineHeight;

  if( i_bTestOnly )
  {
    return uiTotalEntryHeight;
  }

  // Always push a final lineHeight
  qLineHeights.push( uiLineHeight );

  // Reset the font
  pcFont = GetFont();
  rumAssert( pcFont );
  if( pcFont )
  {
    uiFontHeight = pcFont->GetPixelHeight();
    uiSpaceWidth = pcFont->GetTextPixelWidth( " " );
  }

  // Determine how much of the last current buffer to save
  int32_t hOffset{ 0 };
  int32_t vOffset{ 0 };
  if( !m_bNewestFirst )
  {
    vOffset = GetBufferPixelHeight() - uiTotalEntryHeight;
    m_cBufferOffset.m_iY = GetBufferPixelHeight() - GetHeight();
  }

  // Calculate how much of the buffer space we have used
  m_uiBufferPixelHeightUsed += uiTotalEntryHeight;
  if( m_uiBufferPixelHeightUsed > GetBufferPixelHeight() )
  {
    m_uiBufferPixelHeightUsed = GetBufferPixelHeight();
  }

  // Shift the buffer
  if( m_bNewestFirst )
  {
    m_pcDisplayBuffer->Blit( *m_pcDisplayBuffer, rumPoint( 0, 0 ), rumPoint( 0, uiTotalEntryHeight ), GetWidth(),
                             GetBufferPixelHeight() - uiTotalEntryHeight );
  }
  else
  {
    m_pcDisplayBuffer->Blit( *m_pcDisplayBuffer, rumPoint( 0, uiTotalEntryHeight ), rumPoint(), GetWidth(), vOffset );
  }

  // Clean up the part we're about to overwrite with the requested color
  m_pcDisplayBuffer->ClearRect( rumRectangle( rumPoint( 0, vOffset ), GetWidth(), uiTotalEntryHeight ),
                                rumColor::s_cBlackTransparent );

  // Determine the output height
  uiLineHeight = qLineHeights.front();
  qLineHeights.pop();

  // Begin displaying the added text
  while( !qTokens.empty() )
  {
    const rumViewportToken& rcCurrentToken{ qTokens.front() };
    const std::string& strToken{ rcCurrentToken.m_strToken };
    if( rcCurrentToken.m_uiWidth == 0 )
    {
      // Check for a new-line indicator
      if( strToken.compare( "\n" ) == 0 )
      {
        uiLineHeight = qLineHeights.front();
        qLineHeights.pop();

        // Word-wrap
        vOffset += uiLineHeight;
        hOffset = 0;
      }
      // Check regular expressions
      else if( regex_match( strToken, match, reColorHex ) )
      {
        // Parse a color tag, expressed in hex
        char* pstrEnd{ nullptr };
        std::string strTag( match[0].first + 3, match[0].first + 5 );
        const uint32_t r{ strtoul( strTag.c_str(), &pstrEnd, 16 ) };
        strTag = std::string( match[0].first + 5, match[0].first + 7 );
        const uint32_t g{ strtoul( strTag.c_str(), &pstrEnd, 16 ) };
        strTag = std::string( match[0].first + 7, match[0].first + 9 );
        const uint32_t b{ strtoul( strTag.c_str(), &pstrEnd, 16 ) };
        cFontColor = rumColor( r, g, b );
      }
      else if( regex_match( strToken, match, reColorDec ) )
      {
        // Parse a color tag, expressed in decimal
        char* pstrEnd{ nullptr };
        const size_t uiOffset{ strToken.find_first_of( "#" ) + 1 };
        std::string strTag( strToken.substr( uiOffset ) );
        const uint32_t r{ strtoul( strTag.c_str(), &pstrEnd, 10 ) };
        strTag.erase( 0, strTag.find_first_of( "," ) + 1 );
        const uint32_t g{ strtoul( strTag.c_str(), &pstrEnd, 10 ) };
        strTag.erase( 0, strTag.find_first_of( "," ) + 1 );
        const uint32_t b{ strtoul( strTag.c_str(), &pstrEnd, 10 ) };
        cFontColor = rumColor( r, g, b );
      }
      else if( regex_match( strToken, match, reFont ) )
      {
        // Parse a font tag
        const std::string strTag( match[0].first + 3, match[0].second - 1 );
        pcFont = rumFont::Get( strTag );
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
      else if( regex_match( strToken, match, reGraphic ) )
      {
        // Parse a graphic tag
        const std::string strTag( match[0].first + 3, match[0].second - 1 );
        const size_t uiOffset{ strTag.find_first_of( ":" ) };
        const std::string strGraphic{ strTag.substr( strTag.find_first_of( "#" ) + 1, uiOffset ) };
        std::string strOptionals{ "" };
        if( uiOffset != std::string::npos )
        {
          strOptionals = strTag.substr( uiOffset );
        }

        rumGraphic* pcGraphic{ rumGraphic::Fetch( strGraphic ) };
        if( pcGraphic )
        {
          int32_t iTempVertOffset{ vOffset };

          uint32_t uiFrameOffset{ 0 };
          uint32_t uiStateOffset{ 0 };

          uint32_t uiWidth{ pcGraphic->GetFrameWidth() };
          uint32_t uiHeight{ pcGraphic->GetFrameHeight() };

          uint32_t uiFrameWidth{ uiWidth };
          uint32_t uiFrameHeight{ uiHeight };

          bool bStretch{ false };
          bool bVerticalCenter{ false };

          // Tokenize and process any optional graphics modifiers (vcenter, h###, and w###)
          if( strOptionals.length() > 0 )
          {
            char* strContext{ nullptr };
            const char* strOptionalToken{ strtok( &strOptionals[0], ":" ) };
            while( strOptionalToken )
            {
              if( strlen( strOptionalToken ) > 1 )
              {
                if( strOptionalToken[0] == 'h' )
                {
                  char* endptr{ nullptr };
                  const uint32_t uiParsedHeight{ strtoul( &strOptionalToken[1], &endptr, 10 ) };
                  if( uiParsedHeight != uiHeight )
                  {
                    uiHeight = uiParsedHeight;
                    bStretch = true;
                  }
                }
                else if( strOptionalToken[0] == 'w' )
                {
                  char* endptr{ nullptr };
                  const uint32_t uiParsedWidth{ strtoul( &strOptionalToken[1], &endptr, 10 ) };
                  if( uiParsedWidth != uiWidth )
                  {
                    uiWidth = uiParsedWidth;
                    bStretch = true;
                  }
                }
                else if( strOptionalToken[0] == 'f' )
                {
                  char* endptr{ nullptr };
                  uiFrameOffset = strtol( &strOptionalToken[1], &endptr, 10 );
                }
                else if( strOptionalToken[0] == 's' )
                {
                  char* endptr{ nullptr };
                  uiStateOffset = strtol( &strOptionalToken[1], &endptr, 10 );
                }
                else if( strcmp( strOptionalToken, "vcenter" ) == 0 )
                {
                  bVerticalCenter = true;
                }
              }

              // Get next token
              strOptionalToken = strtok( nullptr, ":" );
            }

            if( bVerticalCenter )
            {
              iTempVertOffset += ( uiLineHeight - uiHeight ) / 2;
            }
          }

          if( bStretch )
          {
            // Specific width and/or height was specified
            m_pcDisplayBuffer->BlitStretch( *pcGraphic,
                                            rumPoint( uiFrameWidth * uiStateOffset, uiFrameHeight * uiFrameOffset ),
                                            rumPoint( hOffset, iTempVertOffset ),
                                            uiWidth, uiHeight );
          }
          else
          {
            m_pcDisplayBuffer->Blit( *pcGraphic,
                                     rumPoint( uiFrameWidth * uiStateOffset, uiFrameHeight * uiFrameOffset ),
                                     rumPoint( hOffset, iTempVertOffset ), uiFrameWidth, uiFrameHeight );
          }

          hOffset += uiWidth;
        }
      }
      //else assert(false); // Not sure what we just found
    }
    else
    {
      uint32_t tvOffset{ static_cast<uint32_t>( vOffset ) };
      if( uiFontHeight < uiLineHeight )
      {
        // The current font is smaller that the largest font that will occur on this same line, so offset a little more
        tvOffset += uiLineHeight - uiFontHeight;
      }

      // Print this token to the viewport buffer
      if( pcFont )
      {
        pcFont->BlitText( *m_pcDisplayBuffer, strToken, rumPoint( hOffset, tvOffset ), false, &cFontColor );
      }

      // Advance past the token
      hOffset += rcCurrentToken.m_uiWidth + uiSpaceWidth;
    }

    qTokens.pop();
  }

  // Determine the cursor screen pos
  if( pcCursor )
  {
    if( !m_bNewestFirst )
    {
      if( pcCursor->GetFrameHeight() < uiLineHeight )
      {
        // The cursor needs to be centered vertically
        m_cCursorOffset.m_iY = GetHeight() - uiLineHeight + ( uiLineHeight - pcCursor->GetFrameHeight() ) / 2;
      }
      else
      {
        m_cCursorOffset.m_iY = GetHeight() - uiLineHeight;
      }
    }
    else
    {
      if( pcCursor->GetFrameHeight() < uiLineHeight )
      {
        m_cCursorOffset.m_iY += ( uiLineHeight - pcCursor->GetFrameHeight() ) / 2;
      }
    }

    // Set the cursor horizontal screen pos
    m_cCursorOffset.m_iX = hOffset;
  }

  UpdateDisplay();

  return uiTotalEntryHeight;
}


void rumClientTextView::DrawScrollbar()
{
  if( m_bNewestFirst )
  {
    super::DrawScrollbar();
  }
  else
  {
    // Draw the scrollbar
    if( m_bScrollbarVisible && m_uiBufferPixelHeightUsed > GetHeight() )
    {
      uint32_t iVertOffset{ GetBufferPixelHeight() - m_cBufferOffset.m_iY };
      const float fRatio{ iVertOffset / static_cast<float>( m_uiBufferPixelHeightUsed ) };
      iVertOffset = GetHeight() - static_cast<uint32_t>( fRatio * GetHeight() );

      const uint32_t uiSize
      {
        static_cast<uint32_t>( GetHeight() / static_cast<float>( m_uiBufferPixelHeightUsed ) * GetHeight() )
      };

      // Display the scrollbar background
      m_pcDisplay->ClearRect( rumRectangle( rumPoint( m_uiScrollbarOffset, 0 ), m_uiScrollbarWidth, GetHeight() ),
                              m_cScrollbarBackground );

      // Display the scrollbar
      m_pcDisplay->ClearRect( rumRectangle( rumPoint( m_uiScrollbarOffset, iVertOffset ), m_uiScrollbarWidth, uiSize ),
                              m_cScrollbarColor );
    }
  }
}


void rumClientTextView::PopText()
{
  m_listText.pop_back();
  Invalidate();
}


uint32_t rumClientTextView::PushText( const std::string& i_strText )
{
  m_listText.push_back( i_strText );
  if( m_listText.size() > m_uiMaxEntries )
  {
    m_listText.pop_front();
  }

  return DrawEntry( i_strText );
}


void rumClientTextView::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumClientTextView, rumClientScrollBuffer> cClientTextView( pcVM, "rumClientTextView" );
  cClientTextView
    .Func( "CalculateEntryHeight", &CalculateEntryHeight )
    .Func( "PopText", &PopText )
    .Func( "PushText", &PushText )
    .Func( "SetMaxEntries", &SetMaxEntries )
    .Func( "SetNewestFirst", &SetNewestFirst );
  Sqrat::RootTable( pcVM ).Bind( "rumTextView", cClientTextView );

  rumScript::CreateClassScript( "TextView", "rumTextView" );
}


void rumClientTextView::SetNewestFirst( bool i_bNewestFirst )
{
  if( m_bNewestFirst != i_bNewestFirst )
  {
    m_bNewestFirst = i_bNewestFirst;
    Invalidate();
  }
}


bool rumClientTextView::Validate()
{
  bool bResult{ super::Validate() };

  if( !m_bNewestFirst )
  {
    // Newest items get pushed onto the bottom of the buffer
    m_cBufferOffset.m_iY = GetBufferPixelHeight() - GetHeight();
  }

  // Add all current text in the queue
  for( const auto& iter : m_listText )
  {
    DrawEntry( iter );
  }

  return bResult;
}
