#include <controls/c_textbox.h>

#include <c_graphics.h>
#include <c_font.h>

#include <u_utility.h>


rumClientTextBox::rumClientTextBox()
{
  m_strText.reserve( m_uiMaxInputSize );
}


rumClientTextBox::~rumClientTextBox()
{
  Clear();
}


bool rumClientTextBox::CharacterAdd( char i_strChar )
{
  std::string strTemp{ m_strText + i_strChar };
  std::smatch cMatch;
  if( m_bUseRegEx && !std::regex_match( strTemp, cMatch, m_reInput ) )
  {
    return false;
  }

  bool bUpdateDisplay{ false };

  if( m_bSelectionActive )
  {
    DeleteSelectedText();
    SetSelectionInactive();
    bUpdateDisplay = true;
  }

  const size_t iLength{ m_strText.length() };
  if( iLength <= m_uiMaxInputSize )
  {
    if( iLength == m_uiCursorIndex )
    {
      m_strText += i_strChar;
    }
    else
    {
      m_strText.insert( m_strText.begin() + m_uiCursorIndex, i_strChar );
    }

    ++m_uiCursorIndex;

    UpdateDisplay();
    return true;
  }

  if( bUpdateDisplay )
  {
    UpdateDisplay();
  }

  return false;
}


bool rumClientTextBox::CharacterBackspace()
{
  if( m_bSelectionActive )
  {
    DeleteSelectedText();
    SetSelectionInactive();
    UpdateDisplay();
    return true;
  }

  if( m_strText.length() > 0 && m_uiCursorIndex > 0 )
  {
    m_strText.erase( m_uiCursorIndex - 1, 1 );
    --m_uiCursorIndex;
    UpdateDisplay();
    return true;
  }

  return false;
}


bool rumClientTextBox::CharacterDelete()
{
  if( m_bSelectionActive )
  {
    DeleteSelectedText();
    SetSelectionInactive();
    UpdateDisplay();
    return true;
  }

  if( m_strText.length() > 0 && m_uiCursorIndex < m_strText.length() )
  {
    m_strText.erase( m_uiCursorIndex, 1 );
    UpdateDisplay();
    return true;
  }

  return false;
}


void rumClientTextBox::CopyTextToClipboard() const
{
  std::string strText{ "" };

  if( m_bSelectionActive )
  {
    // Copy only the selected text to the clipboard
    strText = GetSelectedText();
  }
  else
  {
    // Copy all text to the clipboard
    strText = GetText();
  }

  rumStringUtils::CopyTextToClipboard( strText );
}


void rumClientTextBox::CursorLeft( bool i_bSelect )
{
  if( !i_bSelect )
  {
    SetSelectionInactive();
  }

  if( HasCursor() )
  {
    if( m_uiCursorIndex > 0 )
    {
      const uint32_t uiPrevIndex{ m_uiCursorIndex };
      --m_uiCursorIndex;

      if( i_bSelect )
      {
        UpdateSelection( uiPrevIndex );
      }

      UpdateDisplay();
    }
  }
}


void rumClientTextBox::CursorRight( bool i_bSelect )
{
  if( !i_bSelect )
  {
    SetSelectionInactive();
  }

  if( HasCursor() )
  {
    if( m_uiCursorIndex < m_strText.length() )
    {
      const uint32_t uiPrevIndex{ m_uiCursorIndex };
      ++m_uiCursorIndex;

      if( i_bSelect )
      {
        UpdateSelection( uiPrevIndex );
      }

      UpdateDisplay();
    }
  }
}


void rumClientTextBox::CursorHome( bool i_bSelect )
{
  if( !i_bSelect )
  {
    SetSelectionInactive();
  }

  if( HasCursor() )
  {
    if( m_uiCursorIndex > 0 )
    {
      const uint32_t uiPrevIndex{ m_uiCursorIndex };
      m_uiCursorIndex = 0;

      if( i_bSelect )
      {
        UpdateSelection( uiPrevIndex );
      }

      UpdateDisplay();
    }
  }
}


void rumClientTextBox::CursorEnd( bool i_bSelect )
{
  if( !i_bSelect )
  {
    SetSelectionInactive();
  }

  if( HasCursor() )
  {
    if( m_uiCursorIndex < m_strText.length() )
    {
      const uint32_t uiPrevIndex{ m_uiCursorIndex };
      m_uiCursorIndex = (uint32_t)m_strText.length();

      if( i_bSelect )
      {
        UpdateSelection( uiPrevIndex );
      }

      UpdateDisplay();
    }
  }
}


void rumClientTextBox::DeleteSelectedText()
{
  uint32_t uiSelectionStartIndex{ m_uiSelectionStartIndex };
  uint32_t uiSelectionEndIndex{ m_uiCursorIndex };

  if( uiSelectionEndIndex < uiSelectionStartIndex )
  {
    std::swap( uiSelectionStartIndex, uiSelectionEndIndex );
  }

  m_strText.erase( uiSelectionStartIndex, uiSelectionEndIndex - uiSelectionStartIndex );

  // Update the cursor index, since its current index may no longer even exist in the string
  m_uiCursorIndex = uiSelectionStartIndex;
}


std::string rumClientTextBox::GetSelectedText() const
{
  std::string strSelectedText{ "" };

  if( m_bSelectionActive )
  {
    // Make sure the Start/End indices are in the right order
    uint32_t uiSelectionStartIndex{ m_uiSelectionStartIndex };
    uint32_t uiSelectionEndIndex{ m_uiCursorIndex };
    if( uiSelectionEndIndex < uiSelectionStartIndex )
    {
      std::swap( uiSelectionStartIndex, uiSelectionEndIndex );
    }

    strSelectedText = m_strText.substr( uiSelectionStartIndex, uiSelectionEndIndex - uiSelectionStartIndex );
  }

  return strSelectedText;
}


void rumClientTextBox::ObscureText( bool i_bObscure )
{
  if( m_bObscure != i_bObscure )
  {
    m_bObscure = i_bObscure;
    UpdateDisplay();
  }
}


bool rumClientTextBox::PasteTextFromClipboard()
{
  const std::string strCopied( rumStringUtils::CopyTextFromClipboard() );
  const size_t uiLength{ strCopied.length() };

  // Make sure the Start/End indices are in the right order
  uint32_t uiSelectionStartIndex{ m_uiSelectionStartIndex };
  uint32_t uiSelectionEndIndex{ m_uiCursorIndex };
  if( uiSelectionEndIndex < uiSelectionStartIndex )
  {
    std::swap( uiSelectionStartIndex, uiSelectionEndIndex );
  }

  if( m_bSelectionActive )
  {
    // Is there enough room to hold the pasted text?
    if( uiLength > m_uiMaxInputSize )
    {
      return false;
    }

    const size_t uiNewSize{ m_strText.length() - ( uiSelectionEndIndex - uiSelectionStartIndex ) + uiLength };
    if( uiNewSize > m_uiMaxInputSize )
    {
      return false;
    }

    // Paste over the selected text
    m_strText.replace( uiSelectionStartIndex, uiSelectionEndIndex - uiSelectionStartIndex, strCopied );
    m_uiCursorIndex = static_cast<uint32_t>( uiSelectionStartIndex + uiLength );
    SetSelectionInactive();
  }
  else
  {
    // Is there enough room to hold the pasted text?
    const size_t uiNewSize{ m_strText.length() + uiLength };
    if( uiNewSize > m_uiMaxInputSize )
    {
      return false;
    }

    // Append/insert
    m_strText.insert( m_uiCursorIndex, strCopied );
    m_uiCursorIndex += static_cast<uint32_t>( uiLength );
  }

  UpdateDisplay();

  return true;
}


void rumClientTextBox::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumClientTextBox, rumClientControl> cClientTextBox( pcVM, "rumClientTextBox" );
  cClientTextBox
    .Func( "CharacterBackspace", &CharacterBackspace )
    .Func( "CharacterDelete", &CharacterDelete )
    .Func( "CursorLeft", &CursorLeft )
    .Func( "CursorRight", &CursorRight )
    .Func( "CursorHome", &CursorHome )
    .Func( "CursorEnd", &CursorEnd )
    .Func( "CopyTextToClipboard", &CopyTextToClipboard )
    .Func( "PasteTextFromClipboard", &PasteTextFromClipboard )
    .Func( "SelectAll", &SelectAll )
    .Func( "ObscureText", &ObscureText )
    .Func( "CharacterAdd", &ScriptCharacterAdd )
    .Func( "GetText", &GetText )
    .Func( "GetSelectedText", &GetSelectedText )
    .Func( "HasSelectedText", &HasSelectedText )
    .Func( "SetInputRegEx", &SetInputRegEx )
    .Func( "SetText", &SetText )
    .Func( "SetMaxInputSize", &SetMaxInputSize );
  Sqrat::RootTable( pcVM ).Bind( "rumTextBox", cClientTextBox );

  rumScript::CreateClassScript( "TextBox", "rumTextBox" );
}


void rumClientTextBox::SelectAll()
{
  if( HasCursor() )
  {
    m_uiCursorIndex = (uint32_t)m_strText.length();
    UpdateSelection( 0 );
    UpdateDisplay();
  }
}


void rumClientTextBox::SetInputRegEx( const std::string& i_strRegEx )
{
  if( i_strRegEx.empty() )
  {
    m_reInput = {};
    m_bUseRegEx = false;
  }
  else
  {
    m_reInput = std::regex( i_strRegEx, std::regex::icase );
    m_bUseRegEx = true;
  }
}


void rumClientTextBox::SetMaxInputSize( uint32_t i_uiMaxInputSize )
{
  m_uiMaxInputSize = i_uiMaxInputSize;

  if( m_strText.length() > i_uiMaxInputSize )
  {
    // The current text must be truncated
    SetSelectionInactive();
    m_strText.erase( m_strText.begin() + m_uiMaxInputSize, m_strText.end() );
    UpdateDisplay();
  }

  m_strText.reserve( i_uiMaxInputSize );
}


void rumClientTextBox::SetSelectionInactive()
{
  m_bSelectionActive = false;
  m_uiSelectionStartIndex = 0;
}


bool rumClientTextBox::SetText( const std::string& i_strText )
{
  bool bResult{ false };

  if( i_strText.length() <= m_uiMaxInputSize )
  {
    m_strText = i_strText;
    m_uiCursorIndex = (uint32_t)i_strText.length();
    UpdateDisplay();
    bResult = true;
  }

  return bResult;
}


void rumClientTextBox::UpdateDisplay()
{
  m_pcDisplay->Clear( rumColor::s_cBlackTransparent );

  rumFont* pcFont{ GetFont() };
  if( !pcFont )
  {
    return;
  }

  const rumGraphic* pcCursor{ HasCursor() ? FetchCursor() : nullptr };
  const rumGraphic* pcPrompt{ nullptr };

  uint32_t uiPromptHeight{ 0 };
  uint32_t uiPromptWidth{ 0 };

  if( HasPrompt() )
  {
    pcPrompt = FetchPrompt();
    if( pcPrompt )
    {
      uiPromptHeight = pcPrompt->GetHeight();
      uiPromptWidth = pcPrompt->GetWidth();
    }
  }

  std::string strTempText;
  if( m_bObscure )
  {
    strTempText.assign( m_strText.length(), '*' );
  }
  else
  {
    strTempText = m_strText;
  }

  const uint32_t uiTextWidth{ pcFont->GetTextPixelWidth( strTempText ) };
  uint32_t uiTotalTextWidth{ uiTextWidth };

  uint32_t uiCursorOffset{ 0 };
  if( m_uiCursorIndex < strTempText.length() )
  {
    uiCursorOffset = pcFont->GetTextPixelWidth( strTempText.substr( 0, m_uiCursorIndex ) );
  }
  else
  {
    uiCursorOffset = uiTextWidth;
  }

  const uint32_t uiDisplayHeight{ m_pcDisplay->GetHeight() };
  const uint32_t uiDisplayWidth{ m_pcDisplay->GetWidth() };

  if( HasPrompt() )
  {
    uiTotalTextWidth += uiPromptWidth;
    uiCursorOffset += uiPromptWidth;
  }

  if( HasCursor() )
  {
    if( strTempText.length() == m_uiCursorIndex )
    {
      uiTotalTextWidth += pcCursor->GetWidth();
    }
  }

  int32_t hOffset{ 0 };
  int32_t vOffset{ 0 };

  switch( m_eAlignment )
  {
    case ALIGN_CENTER:
      hOffset = ( GetWidth() / 2 ) - ( uiTotalTextWidth / 2 );
      break;

    case ALIGN_RIGHT:
      hOffset = GetWidth() - uiTotalTextWidth;
      break;
  }

  if( uiTotalTextWidth > uiDisplayWidth )
  {
    hOffset = uiDisplayWidth;

    if( pcCursor && pcCursor->GetFrameHeight() < uiDisplayHeight )
    {
      // The cursor needs to be centered vertically on this line
      vOffset = ( uiDisplayHeight - pcCursor->GetFrameHeight() ) / 2;
    }

    // If the cursor is at the end of the input, display the cursor to the far right with as much of the visible
    // text as possible displayed before it
    if( strTempText.length() == m_uiCursorIndex )
    {
      m_uiCursorAnchorOffset = uiTotalTextWidth - uiDisplayWidth;

      // The cursor will be displayed at the far right
      if( pcCursor )
      {
        hOffset -= pcCursor->GetFrameWidth();
      }

      // Set the cursor screen pos
      m_cCursorOffset.m_iX = hOffset;
      m_cCursorOffset.m_iY = vOffset;

      hOffset -= uiTextWidth;

      // Print the text to the viewport buffer
      pcFont->BlitText( *m_pcDisplay, strTempText, rumPoint( hOffset, 0 ), false, &( pcFont->GetColor() ) );

      // no need to draw the prompt, it's fully off-screen
    }
    else
    {
      // See if the cursor anchor needs to be adjusted as a result of cursor movement
      if( uiCursorOffset < m_uiCursorAnchorOffset )
      {
        // The cursor moved to the left of the anchor
        m_uiCursorAnchorOffset = uiCursorOffset;
      }
      else if( ( uiCursorOffset - m_uiCursorAnchorOffset ) >= ( uiDisplayWidth - pcCursor->GetFrameWidth() ) )
      {
        // The cursor moved too far away from the anchor to show all text between the two in the given space
        m_uiCursorAnchorOffset = uiCursorOffset - uiDisplayWidth + ( pcPrompt ? uiPromptWidth : 0 );
      }

      // Text is drawn based on cursor and cursor anchor settings. Basically, the cursor should never be drawn
      // outside of the visible display. Text is always drawn with the anchor offset starting at the beginning of
      // the display area. The cursor can fall anywhere in the visible display.
      m_cCursorOffset.m_iX = uiCursorOffset - m_uiCursorAnchorOffset;
      m_cCursorOffset.m_iY = vOffset;

      if( pcPrompt && m_uiCursorAnchorOffset < uiPromptWidth )
      {
        if( uiPromptHeight < uiDisplayHeight )
        {
          // The prompt needs to be centered vertically on this line
          vOffset = ( uiDisplayHeight - uiPromptHeight ) / 2;
        }

        // Show the prompt
        m_pcDisplay->Blit( *pcPrompt, rumPoint(), rumPoint( m_uiCursorAnchorOffset, vOffset ),
                           uiPromptWidth, uiPromptHeight );
      }

      hOffset = 0 - m_uiCursorAnchorOffset + ( pcPrompt ? uiPromptWidth : 0 );
      pcFont->BlitText( *m_pcDisplay, strTempText, rumPoint( hOffset, 0 ), false, &( pcFont->GetColor() ) );
    }
  }
  else
  {
    // A prompt graphic is used, so compute the vertical and horizontal offsets that it requires
    if( pcPrompt )
    {
      if( uiPromptHeight < uiDisplayHeight )
      {
        // The prompt needs to be centered vertically on this line
        vOffset = ( uiDisplayHeight - uiPromptHeight ) / 2;
      }

      // Show the prompt
      m_pcDisplay->Blit( *pcPrompt, rumPoint(), rumPoint( hOffset, vOffset ), uiPromptWidth, uiPromptHeight );

      switch( m_eAlignment )
      {
        case ALIGN_LEFT:
        case ALIGN_RIGHT:
          hOffset += uiPromptWidth;
          break;

        case ALIGN_CENTER:
          hOffset += ( uiPromptWidth / 2 );
          break;
      }

      vOffset = 0;
    }

    // Print the text to the viewport buffer
    pcFont->BlitText( *m_pcDisplay, strTempText, rumPoint( hOffset, 0 ), false, &( pcFont->GetColor() ) );

    uint32_t uiCursorWidth{ 0 };

    // Determine the cursor screen pos
    if( pcCursor )
    {
      uiCursorWidth = pcCursor->GetWidth();

      if( pcCursor->GetFrameHeight() < uiDisplayHeight )
      {
        // The prompt needs to be centered vertically on this line
        vOffset = ( uiDisplayHeight - pcCursor->GetFrameHeight() ) / 2;
      }
    }

    // Set the cursor screen pos
    switch( m_eAlignment )
    {
      case ALIGN_LEFT:
        m_cCursorOffset.m_iX = uiCursorOffset;
        break;

      case ALIGN_RIGHT:
        m_cCursorOffset.m_iX = uiDisplayWidth - uiCursorWidth;
        break;

      case ALIGN_CENTER:
        m_cCursorOffset.m_iX = hOffset + uiTotalTextWidth - uiCursorWidth;
        break;
    }

    m_cCursorOffset.m_iY = vOffset;
  }

  // Highlight the selected part of the text
  if( m_bSelectionActive )
  {
    uint32_t uiSelectionStartIndex{ m_uiSelectionStartIndex };
    uint32_t uiSelectionEndIndex{ m_uiCursorIndex };
    if( uiSelectionEndIndex < uiSelectionStartIndex )
    {
      std::swap( uiSelectionStartIndex, uiSelectionEndIndex );
    }

    // Get the substr
    const std::string strSelectedText{ strTempText.substr( uiSelectionStartIndex,
                                                           uiSelectionEndIndex - uiSelectionStartIndex ) };

    const uint32_t uiTextWidth{ pcFont->GetTextPixelWidth( strTempText.substr( 0, uiSelectionStartIndex ) ) };
    pcFont->BlitText( *m_pcDisplay, strSelectedText, rumPoint( hOffset + uiTextWidth, 0 ), false,
                      &rumColor::s_cYellow );
  }
}


void rumClientTextBox::UpdateSelection( uint32_t i_uiPrevIndex )
{
  if( !m_bSelectionActive )
  {
    m_bSelectionActive = true;
    m_uiSelectionStartIndex = i_uiPrevIndex;
  }
}


// override
bool rumClientTextBox::Validate()
{
  bool bResult{ super::Validate() };
  UpdateDisplay();
  return bResult;
}
