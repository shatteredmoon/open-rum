#include <controls/c_listview.h>

#include <c_graphics.h>
#include <c_font.h>

#include <u_utility.h>

#include <queue>
#include <regex>

const rumColor rumClientListView::s_cDefaultSelectionActiveColor{ 128, 128, 128, 128 };
const rumColor rumClientListView::s_cDefaultSelectionInactiveColor{ 64, 64, 64, 128 };


struct rumViewportToken
{
  std::string m_strToken;
  uint32_t m_uiWidth{ 0 };
};


// override
void rumClientListView::Clear()
{
  m_vEntries.clear();
  super::Clear();
  m_iCurrentIndex = -1;
  m_uiMaxSelectableEntries = 1;
  m_uiMinSelectableEntries = 1;
  m_uiNumSelectedEntries = 0;
}


void rumClientListView::DisableMultiSelect()
{
  m_bMultiSelect = false;
  m_uiMinSelectableEntries = 1;
  m_uiMaxSelectableEntries = 1;
  SelectNone();
}


int32_t rumClientListView::DrawEntry( const int32_t i_iIndex, const ListItem& i_rcListItem )
{
  rumFont* pcFont{ GetFont() };
  if( nullptr == pcFont )
  {
    return RESULT_FAILED;
  }

  const rumColor cFontColor( pcFont->GetColor() );

  // Determine screen space needed
  const uint32_t uiFontHeight{ pcFont->GetPixelHeight() };

  // The width of a single space will likely be used many times over
  const uint32_t uiSpaceWidth{ pcFont->GetTextPixelWidth( " " ) };

  // Calculate where the overwrite should occur
  int32_t iColOffset{ 0 };
  int32_t iColEndOffset{ 0 };
  const int32_t iRowOffset{ static_cast<int32_t>( uiFontHeight * i_iIndex ) };

  // The current column
  uint32_t iColIndex{ 0 };

  // Selection color
  const bool bHighlight{ ( i_iIndex == m_iCurrentIndex ) && m_bHighlightCurrent };

  // Clean up the part we're about to overwrite with the requested color
  const rumColor& rcColor{ bHighlight ? ( HasInputFocus() ? m_cSelectionActiveColor : m_cSelectionInactiveColor )
                                      : rumColor::s_cBlackTransparent };
  m_pcDisplayBuffer->ClearRect( rumRectangle( rumPoint( 0, iRowOffset ), GetWidth(), uiFontHeight ), rcColor );

  // Tokens that will be displayed
  std::queue<std::string> qColumnStrings;

  // Parse the format string and build column offsets
  const char* strDelimiter{ m_strDelimiter.c_str() };
  char* strFormatCopy{ new char[i_rcListItem.m_strEntry.length() + 1] };
  strcpy( strFormatCopy, i_rcListItem.m_strEntry.c_str() );
  const char* strColumn{ strtok( strFormatCopy, strDelimiter ) };
  while( strColumn )
  {
    qColumnStrings.push( strColumn );

    // Get next token
    strColumn = strtok( nullptr, strDelimiter );
  }

  const rumPoint cZero;

  while( !qColumnStrings.empty() )
  {
    const std::string& strColumn{ qColumnStrings.front() };

    // Tokens that will be displayed
    std::queue<rumViewportToken> qTokens;

    static const std::regex reTag( "<.+?>" ); //non-greedy
    static const std::regex reToken( "\\s" );
    static const std::regex reGraphic( "<g#(([0-9]|[a-z]|[A-Z]|[_]){1,127})([:].*)*>", std::regex::icase );

    iColOffset = m_vColumnOffsets[iColIndex].m_uiOffset;
    if( m_vColumnOffsets.size() > ( iColIndex + 1 ) )
    {
      // End the column one pixel before the start of the next
      iColEndOffset = m_vColumnOffsets[iColIndex + 1].m_uiOffset - 1;
    }
    else
    {
      // End the column just before the scrollbar
      iColEndOffset = GetWidth() - m_uiScrollbarWidth - 1;
    }

    if( ( 0 == iColIndex ) && i_rcListItem.m_bSelected /* PromptVisible */ )
    {
      const rumGraphic* pcPrompt{ FetchPrompt() };
      if( pcPrompt )
      {
        int32_t iOffset{ iRowOffset };

        // A prompt graphic is used, so compute the vertical and horizontal offsets that it requires
        const uint32_t uiPromptHeight{ pcPrompt->GetHeight() };
        const uint32_t uiPromptWidth{ pcPrompt->GetWidth() };
        if( uiPromptHeight < uiFontHeight )
        {
          // The prompt needs to be centered vertically on this line
          iOffset += ( uiFontHeight - uiPromptHeight ) / 2;
        }

        // Show the prompt
        if( i_iIndex == m_iCurrentIndex )
        {
          m_pcDisplayBuffer->BlitAlphaPreserve( *pcPrompt, rumPoint( 0, iOffset ) );
        }
        else
        {
          m_pcDisplayBuffer->Blit( *pcPrompt, cZero, rumPoint( 0, iOffset ), uiPromptWidth, uiPromptHeight );
        }

        iColOffset += uiPromptWidth;
      }
    }

    uint32_t uiCurrentWidth{ 0 };

    // Default the current lineHeight to that of the font
    uint32_t uiLineHeight{ uiFontHeight };

    // Break text into tokens, space dilineated. The -1 in this call means:
    // return everything that doesn't match the expression, or in other words,
    // everything that isn't a space.
    std::sregex_token_iterator iter( strColumn.begin(), strColumn.end(), reToken, -1 );
    const std::sregex_token_iterator end;
    std::smatch match;
    for( ; iter != end; ++iter )
    {
      // Create the token
      std::string::const_iterator start( iter->first );
      const std::string innertoken( iter->first, iter->second );

      // See if the token contains any tags
      while( regex_search( start, iter->second, match, reTag ) )
      {
        // See if there was legitimate text in a group of tags
        if( match[0].first != start )
        {
          rumViewportToken cViewportToken;
          cViewportToken.m_strToken = std::string( start, match[0].first );
          cViewportToken.m_uiWidth = pcFont->GetTextPixelWidth( cViewportToken.m_strToken );

          qTokens.push( cViewportToken );
          uiCurrentWidth += cViewportToken.m_uiWidth + uiSpaceWidth;
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

          rumGraphic* pcGraphic{ rumGraphic::Fetch( strGraphic ) };
          if( pcGraphic )
          {
            uint32_t uiWidth{ pcGraphic->GetFrameWidth() };
            uint32_t uiHeight{ pcGraphic->GetFrameHeight() };

            // Tokenize and process any optional graphics modifiers (vcenter, h###, and w###)
            if( strOptionals.length() > 0 )
            {
              const char* strToken{ strtok( &strOptionals[0], ":" ) };
              while( strToken )
              {
                if( strlen( strToken ) > 1 )
                {
                  if( strToken[0] == L'h' )
                  {
                    char* strEnd{ nullptr };
                    uiHeight = strtol( &strToken[1], &strEnd, 10 );
                  }
                  else if( strToken[0] == L'w' )
                  {
                    char* strEnd{ nullptr };
                    uiWidth = strtol( &strToken[1], &strEnd, 10 );
                  }
                }

                // Get next token
                strToken = strtok( nullptr, ":" );
              }
            }

            uiLineHeight = std::max( uiLineHeight, uiHeight );
            uiCurrentWidth += uiWidth + uiSpaceWidth;
          }
        }

        rumViewportToken cViewportToken;
        cViewportToken.m_strToken = std::string( match[0].first, match[0].second );

        qTokens.push( cViewportToken );
        start = match[0].second;
      }

      // Grab anything not processed by the tag handler. If there were no tags in the token, then this should represent
      // the entire token
      if( start != iter->second )
      {
        rumViewportToken cViewportToken;
        cViewportToken.m_strToken = std::string( start, iter->second );

        // Otherwise, determine the string length and add it to the queue
        cViewportToken.m_uiWidth = pcFont->GetTextPixelWidth( cViewportToken.m_strToken );

        qTokens.push( cViewportToken );
        uiCurrentWidth += cViewportToken.m_uiWidth + ( qTokens.size() > 1 ? uiSpaceWidth : 0 );
      }
    }

    // Determine the justification offset
    switch( m_vColumnOffsets[iColIndex].m_eAlignment )
    {
      case ALIGN_RIGHT:
        iColOffset = iColEndOffset - uiCurrentWidth;
        break;

      case ALIGN_CENTER:
        iColOffset = ( GetWidth() / 2 ) - ( uiCurrentWidth / 2 );
        break;
    }

    int32_t hOffset = iColOffset;
    int32_t vOffset = iRowOffset;

    // Begin displaying the added text
    while( !qTokens.empty() )
    {
      const bool bHighlight{ ( i_iIndex == m_iCurrentIndex ) && m_bHighlightCurrent };

      const rumViewportToken& currentToken{ qTokens.front() };
      const std::string& strToken{ currentToken.m_strToken };
      if( 0 == currentToken.m_uiWidth )
      {
        if( regex_match( strToken, match, reGraphic ) )
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

          // TODO - Should be const, but has to be temporarily flipped to fix an upside-down graphics bug
          rumGraphic* pcGraphic{ rumGraphic::Fetch( strGraphic ) };
          if( pcGraphic )
          {
            // TODO - These come out upside down because of framebuffer shenanigans
            pcGraphic->Flip();

            int32_t iTempVertOffset{ iRowOffset };

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
              const char* strOptionaltoken{ strtok( &strOptionals[0], ":" ) };
              while( strOptionaltoken != nullptr )
              {
                if( strlen( strOptionaltoken ) > 1 )
                {
                  if( strOptionaltoken[0] == 'h' )
                  {
                    char* strEnd{ nullptr };
                    const uint32_t uiParsedHeight{ static_cast<uint32_t>( strtol( &strOptionaltoken[1],
                                                                                  &strEnd, 10 ) ) };
                    if( uiParsedHeight != uiHeight )
                    {
                      uiHeight = uiParsedHeight;
                      bStretch = true;
                    }
                  }
                  else if( strOptionaltoken[0] == 'w' )
                  {
                    char* strEnd{ nullptr };;
                    const uint32_t uiParsedWidth{ static_cast<uint32_t>( strtol( &strOptionaltoken[1],
                                                                                 &strEnd, 10 ) ) };
                    if( uiParsedWidth != uiWidth )
                    {
                      uiWidth = uiParsedWidth;
                      bStretch = true;
                    }
                  }
                  else if( strOptionaltoken[0] == 'f' )
                  {
                    char* strEnd{ nullptr };
                    uiFrameOffset = strtol( &strOptionaltoken[1], &strEnd, 10 );
                  }
                  else if( strOptionaltoken[0] == 's' )
                  {
                    char* strEnd{ nullptr };
                    uiStateOffset = strtol( &strOptionaltoken[1], &strEnd, 10 );
                  }
                  else if( strcmp( strOptionaltoken, "vcenter" ) == 0 )
                  {
                    bVerticalCenter = true;
                  }
                }

                // Get next token
                strOptionaltoken = strtok( nullptr, ":" );
              }

              if( bVerticalCenter )
              {
                iTempVertOffset += ( uiLineHeight - uiHeight ) / 2;
              }
            }

            if( bStretch )
            {
              // Specific width and/or height was specified
              if( bHighlight )
              {
                // Since the alpha has to be preserved, copy to a temporary space first and then blit alpha from there
                rumGraphic* pcTempBuffer{ rumGraphic::Create() };
                if( pcTempBuffer )
                {
                  pcTempBuffer->InitData( uiWidth, uiHeight );
                  pcTempBuffer->BlitStretch( *pcGraphic,
                                             rumPoint( uiFrameWidth * uiStateOffset, uiFrameHeight * uiFrameOffset ),
                                             cZero, uiWidth, uiHeight );

                  m_pcDisplayBuffer->BlitAlphaPreserve( *pcTempBuffer, rumPoint( hOffset, iTempVertOffset ) );

                  pcTempBuffer->Free();
                }
              }
              else
              {
                m_pcDisplayBuffer->BlitStretch( *pcGraphic,
                                                rumPoint( uiFrameWidth * uiStateOffset,
                                                          uiFrameHeight * uiFrameOffset ),
                                                rumPoint( hOffset, iTempVertOffset ),
                                                uiWidth, uiHeight );
              }
            }
            else
            {
              //if( bHighlight )
              {
                // Since the alpha has to be preserved, copy to a temporary space first and then blit alpha from there
                rumGraphic* pcTempBuffer{ rumGraphic::Create() };
                if( pcTempBuffer )
                {
                  pcTempBuffer->InitData( uiFrameWidth, uiFrameHeight );
                  pcTempBuffer->Blit( *pcGraphic,
                                      rumPoint( uiFrameWidth * uiStateOffset, uiFrameHeight * uiFrameOffset ),
                                      cZero, uiFrameWidth, uiFrameHeight );

                  m_pcDisplayBuffer->BlitAlphaPreserve( *pcTempBuffer, rumPoint( hOffset, iTempVertOffset ) );

                  pcTempBuffer->Free();
                }
              }
              //else
              //{
              //  // TODO - Rendering upside down???
              //  m_pcDisplayBuffer->BlitAlpha( *pcGraphic,
              //                                rumPoint( uiFrameWidth * uiStateOffset, uiFrameHeight * uiFrameOffset ),
              //                                rumPoint( hOffset, iTempVertOffset ), uiFrameWidth, uiFrameHeight,
              //                                true /* src color */ );
              //}
            }

            hOffset += uiWidth;

            // TODO - These come out upside down because of framebuffer shenanigans
            pcGraphic->Flip();
          }
        }
      }
      else
      {
        // Print the item to the buffer
        pcFont->BlitText( *m_pcDisplayBuffer, strToken, rumPoint( hOffset, iRowOffset ), bHighlight, &cFontColor );

        // Advance past the token
        hOffset += currentToken.m_uiWidth + uiSpaceWidth;
      }

      qTokens.pop();
    }

    qColumnStrings.pop();

    // Advance to the next column
    ++iColIndex;
  }

  delete[] strFormatCopy;

  UpdateDisplay();

  return RESULT_SUCCESS;
}


void rumClientListView::EnableMultiSelect( uint32_t i_uiMin, uint32_t i_uiMax )
{
  if( i_uiMin > 1 || i_uiMax > 1 )
  {
    m_bMultiSelect = true;
    m_uiMinSelectableEntries = std::min( i_uiMin, i_uiMax );
    m_uiMaxSelectableEntries = std::max( i_uiMin, i_uiMax );
    SelectNone();
  }
}


// override
void rumClientListView::Focus()
{
  super::Focus();
  OnIndexChanged();
}


std::string rumClientListView::GetCurrentEntry()
{
  const ListItem* pcListItem{ ListItemFetchIndex( m_iCurrentIndex ) };
  return pcListItem ? pcListItem->m_strEntry.c_str() : "";
}


std::string rumClientListView::GetEntry( int64_t i_iKey )
{
  const auto* pcListItem{ ListItemFetchKey( i_iKey ) };
  return pcListItem != nullptr ? pcListItem->m_strEntry : "";
}


int32_t rumClientListView::GetIndexAtScreenPosition( const rumPoint& i_rcPoint )
{
  if( ContainsPoint( i_rcPoint ) )
  {
    // Offset from top of control + the scroll buffer offset
    const int32_t iEntryHeight{ static_cast<int32_t>( GetFontHeight() ) };
    return ( i_rcPoint.m_iY - GetPos().m_iY ) / iEntryHeight + m_cBufferOffset.m_iY / iEntryHeight;
  }

  // Invalid index
  return -1;
}


int64_t rumClientListView::GetSelectedKey() const
{
  return( m_iCurrentIndex < m_vEntries.size() ? m_vEntries.at( m_iCurrentIndex ).m_iKey : 0 );
}


rumPoint rumClientListView::GetSelectedPos() const
{
  rumPoint cPos{ GetPos() };
  cPos.m_iY += GetCurrentIndex() * GetFontHeight();

  return cPos;
}


rumClientListView::ListItem* rumClientListView::ListItemFetchIndex( uint32_t i_uiIndex )
{
  return i_uiIndex < m_vEntries.size() ? &m_vEntries.at( i_uiIndex ) : nullptr;
}


rumClientListView::ListItem* rumClientListView::ListItemFetchKey( int64_t i_iKey )
{
  for( auto& iter : m_vEntries )
  {
    if( iter.m_iKey == i_iKey )
    {
      return &iter;
    }
  }

  return nullptr;
}


void rumClientListView::MoveToIndex( int32_t i_iIndex )
{
  if( i_iIndex < 0 || i_iIndex >= m_vEntries.size() )
  {
    return;
  }

  const int32_t uiPrevItem{ m_iCurrentIndex };
  m_iCurrentIndex = i_iIndex;

  if( !IsInvalidated() )
  {
    // Redraw the previous item
    const ListItem* pcListItem{ ListItemFetchIndex( uiPrevItem ) };
    if( pcListItem )
    {
      DrawEntry( uiPrevItem, *pcListItem );
    }

    // Redraw the current item
    pcListItem = ListItemFetchIndex( m_iCurrentIndex );
    if( pcListItem )
    {
      DrawEntry( m_iCurrentIndex, *pcListItem );
    }
  }

  const uint32_t uiFontHeight{ GetFontHeight() };

  // Determine how far into our buffer the selection is
  const uint32_t uiYOffset{ m_iCurrentIndex * uiFontHeight };

  // If we're past the buffer, we should move the buffer down
  const rumPoint cPoint{ GetBufferVerticalExtents() };
  if( uiYOffset >= (uint32_t)cPoint.m_iY )
  {
    // todo - why not just set the buffer start pos?
    BufferDown( ( uiYOffset - cPoint.m_iY ) + uiFontHeight );
  }
  else if( uiYOffset < (uint32_t)cPoint.m_iX )
  {
    // todo - why not just set the buffer start pos?
    BufferUp( cPoint.m_iX - uiYOffset );
  }

  OnIndexChanged();
}


void rumClientListView::MovePageUp()
{
  rumAssert( m_iCurrentIndex >= 0 );

  const uint32_t uiFontHeight{ GetFontHeight() };

  if( 0 == uiFontHeight )
  {
    return;
  }

  const uint32_t uiNumPageItems{ GetHeight() / uiFontHeight };
  int32_t iIndex{ m_iCurrentIndex - static_cast<int32_t>( uiNumPageItems ) };

  // Stay within our index range
  if( iIndex >= m_vEntries.size() )
  {
    iIndex = 0;
  }

  MoveToIndex( iIndex );
}


void rumClientListView::MovePageDown()
{
  rumAssert( m_iCurrentIndex >= 0 );

  const uint32_t uiFontHeight{ GetFontHeight() };
  if( 0 == uiFontHeight )
  {
    return;
  }

  const uint32_t uiNumPageItems{ GetHeight() / uiFontHeight };
  int32_t iIndex{ m_iCurrentIndex + static_cast<int32_t>( uiNumPageItems ) };
  iIndex += uiNumPageItems;

  // Stay within our index range
  if( iIndex >= m_vEntries.size() )
  {
    iIndex = static_cast<uint32_t>( m_vEntries.size() - 1 );
  }

  MoveToIndex( iIndex );
}


void rumClientListView::OnIndexChanged()
{
  Sqrat::Object sqInstance{ GetScriptInstance() };
  if( sqInstance.GetType() != OT_NULL )
  {
    rumScript::ExecOptionalFunc( sqInstance, "OnIndexChanged", m_iCurrentIndex );
  }
}


void rumClientListView::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::DerivedClass<rumClientListView, rumClientScrollBuffer> cClientListView( pcVM, "rumClientListView" );
  cClientListView
    .Func( "CalcHeight", &CalcHeight )
    .Func( "Clear", &Clear )
    .Func( "DisableMultiSelect", &DisableMultiSelect )
    .Func( "EnableMultiSelect", &EnableMultiSelect )
    .Func( "HighlightCurrentIndex", &HighlightCurrentIndex )
    .Func( "MoveHome", &MoveToFirst )
    .Func( "MoveEnd", &MoveToLast )
    .Func( "MoveNext", &MoveToNext )
    .Func( "MovePrev", &MoveToPrev )
    .Func( "MovePageUp", &MovePageUp )
    .Func( "MovePageDown", &MovePageDown )
    .Func( "GetIndexAtScreenPosition", &GetIndexAtScreenPosition )
    .Func( "GetSelectedIndex", &GetCurrentIndex )
    .Func( "GetSelectedKey", &GetSelectedKey )
    .Func( "GetSelectedKeys", &ScriptGetSelectedKeys )
    .Func( "GetSelectedPos", &GetSelectedPos )
    .Func( "GetEntry", &GetEntry )
    .Func( "GetCurrentEntry", &GetCurrentEntry )
    .Func( "GetNumEntries", &GetNumEntries )
    .Func( "GetNumSelected", &GetNumSelectedEntries )
    .Func( "GetDelimiter", &GetDelimiter )
    .Func( "RemoveEntry", &RemoveEntry )
    .Func( "SetFormat", &SetFormat )
    .Func( "SelectAll", &SelectAll )
    .Func( "SelectCurrent", &SelectCurrent )
    .Func( "ResetSelection", &SelectionReset )
    .Func( "SelectItem", &SelectItem )
    .Func( "SelectItemByShortcut", &SelectItemByShortcut )
    .Func( "SelectNone", &SelectNone )
    .Func( "SetCurrentIndex", &SetCurrentIndex )
    .Func( "SetMaxEntries", &SetMaxEntries )
    .Func( "GetMaxSelectable", &GetMaxSelectableEntries )
    .Func( "SetMaxSelectable", &SetMaxSelectableEntries )
    .Func( "GetMinSelectable", &GetMinSelectableEntries )
    .Func( "SetMinSelectable", &SetMinSelectableEntries )
    .Func( "SupportsMultiSelect", &SupportsMultiSelect )
    .Overload<bool( rumClientListView::* )( int64_t, const std::string& )>( "SetEntry", &SetEntry )
    .Overload<bool( rumClientListView::* )( int64_t, const std::string&, NativeHandle )>( "SetEntry", &SetEntry )
    .Overload<void( rumClientListView::* )( const std::string& )> ( "SetDelimiter", &SetDelimiter );
  Sqrat::RootTable( pcVM ).Bind( "rumListView", cClientListView );

  rumScript::CreateClassScript( "ListView", "rumListView" );
}


void rumClientListView::RemoveEntry( int64_t i_iKey )
{
  uint32_t uiIndex{ 0 };

  for( auto iter : m_vEntries )
  {
    if( iter.m_iKey = i_iKey )
    {
      break;
    }

    ++uiIndex;
  }

  m_vEntries.erase(m_vEntries.begin() + uiIndex );
  Invalidate();
}


Sqrat::Array rumClientListView::ScriptGetSelectedKeys()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  // It's okay to have zero length squirrel arrays
  Sqrat::Array sqArray( pcVM, m_uiNumSelectedEntries );

  if( m_uiNumSelectedEntries > 0 )
  {
    uint32_t uiIndex{ 0 };
    for( const auto& iter : m_vEntries )
    {
      if( iter.m_bSelected )
      {
        // If this assert fails, we're not tracking NumSelectedEntries correctly!
        rumAssert( uiIndex < m_uiNumSelectedEntries );

        // Add the key
        sqArray.SetValue( uiIndex++, iter.m_iKey );
      }
    }
  }

  return sqArray;
}


void rumClientListView::SelectAll( bool i_bSelected )
{
  if( i_bSelected && ( m_uiNumSelectedEntries >= m_uiMaxSelectableEntries ) )
  {
    // Early out since nothing can be selected
    return;
  }

  int32_t iIndex{ 0 };
  for( auto& iter : m_vEntries )
  {
    if( m_uiNumSelectedEntries < m_uiMaxSelectableEntries && iter.m_bSelected != i_bSelected )
    {
      iter.m_bSelected = i_bSelected;

      if( i_bSelected )
      {
        ++m_uiNumSelectedEntries;
      }
      else
      {
        --m_uiNumSelectedEntries;
      }

      rumAssert( m_uiNumSelectedEntries <= m_uiMaxSelectableEntries );

      // Redraw this item
      if( !IsInvalidated() )
      {
        DrawEntry( iIndex, iter );
      }
    }

    ++iIndex;
  }
}


bool rumClientListView::SelectCurrent()
{
  bool bSelected{ false };

  ListItem* pcListItem{ ListItemFetchIndex( m_iCurrentIndex ) };
  if( pcListItem )
  {
    if( pcListItem->m_bSelected )
    {
      // Deselect since it's already selected
      pcListItem->m_bSelected = false;
      --m_uiNumSelectedEntries;
    }
    else
    {
      // Okay to select?
      if( m_uiNumSelectedEntries < m_uiMaxSelectableEntries )
      {
        // Select
        pcListItem->m_bSelected = true;
        ++m_uiNumSelectedEntries;
      }
    }

    bSelected = pcListItem->m_bSelected;
    rumAssert( m_uiNumSelectedEntries <= m_uiMaxSelectableEntries );

    // Redraw this item
    if( !IsInvalidated() )
    {
      DrawEntry( m_iCurrentIndex, *pcListItem );
    }
  }

  return bSelected;
}


bool rumClientListView::SelectItem( int32_t i_iKey )
{
  bool bSelected{ false };

  // Iterate to the item (so that we can determine the index)
  int32_t iIndex{ 0 };
  for( auto& iter : m_vEntries )
  {
    if( iter.m_iKey == i_iKey )
    {
      if( iter.m_bSelected )
      {
        // Deselect since it's already selected
        iter.m_bSelected = false;
        --m_uiNumSelectedEntries;
      }
      else if( m_uiNumSelectedEntries < m_uiMaxSelectableEntries )
      {
        // Select
        iter.m_bSelected = true;
        ++m_uiNumSelectedEntries;
      }

      bSelected = iter.m_bSelected;

      // Redraw this item
      if( !IsInvalidated() )
      {
        DrawEntry( iIndex, iter );
      }

      break;
    }

    ++iIndex;
  }

  return bSelected;
}


bool rumClientListView::SelectItemByShortcut( int32_t i_iKeyShortcut )
{
  bool bSelected{ false };

  int32_t iIndex{ 0 };
  for( auto& iter : m_vEntries )
  {
    if( iter.m_keyShortcut == i_iKeyShortcut )
    {
      if( iter.m_bSelected )
      {
        iter.m_bSelected = false;
        --m_uiNumSelectedEntries;
      }
      else if( m_uiNumSelectedEntries < m_uiMaxSelectableEntries )
      {
        iter.m_bSelected = true;
        ++m_uiNumSelectedEntries;
      }

      MoveToIndex( iIndex );

      bSelected = iter.m_bSelected;
      rumAssert( m_uiNumSelectedEntries <= m_uiMaxSelectableEntries );

      // Redraw this item
      if( !IsInvalidated() )
      {
        DrawEntry( iIndex, iter );
      }

      break;
    }

    ++iIndex;
  }

  return bSelected;
}


void rumClientListView::SetCurrentIndex( int32_t i_iIndex )
{
  if( i_iIndex >= 0 && i_iIndex < m_vEntries.size() )
  {
    MoveToIndex( i_iIndex );
  }
}


bool rumClientListView::SetEntry( int64_t i_iKey, const std::string& i_strEntry, int32_t i_iKeyShortcut )
{
  bool bSet{ false };

  auto* pcListItem{ ListItemFetchKey( i_iKey ) };
  if( pcListItem != nullptr )
  {
    // Update the item
    pcListItem->m_keyShortcut = i_iKeyShortcut;
    pcListItem->m_strEntry = i_strEntry;

    uint32_t iIndex{ 0 };
    for( const auto& iter : m_vEntries )
    {
      if( iter.m_iKey == i_iKey )
      {
        DrawEntry( iIndex, iter );
      }

      ++iIndex;
    }

    bSet = true;
  }
  else if( m_vEntries.size() < m_uiMaxEntries )
  {
    // Add the new item
    ListItem cListItem;
    cListItem.m_iKey = i_iKey;
    cListItem.m_strEntry = i_strEntry;
    cListItem.m_keyShortcut = i_iKeyShortcut;

    m_vEntries.push_back( cListItem );

    Invalidate();

    bSet = true;
  }

  if( bSet && m_iCurrentIndex < 0 )
  {
    MoveToFirst();
  }

  return bSet;
}


void rumClientListView::SetMaxSelectableEntries( uint32_t i_uiMax )
{
  if( i_uiMax < m_uiNumSelectedEntries )
  {
    // We currently have more entries selected than is about to be allowed, so deselect all entries
    SelectAll( false );
  }

  m_uiMaxSelectableEntries = i_uiMax;
}


bool rumClientListView::Validate()
{
  // Determine buffer height based on the number of current entries
  const uint32_t uiRequiredHeight{ GetFontHeight() * (uint32_t)m_vEntries.size() };
  SetBufferPixelHeight( uiRequiredHeight );

  const bool bResult{ super::Validate() };

  // For list views, we always use the entire buffer
  m_uiBufferPixelHeightUsed = uiRequiredHeight;

  // Build the offset table from the format string
  m_vColumnOffsets.clear();
  int32_t iLastOffset{ -1 };

  // Parse the format string and build column offsets
  const char* strDelimiter{ m_strDelimiter.c_str() };
  char* strFormatCopy{ new char[m_strFormat.length() + 1] };
  strcpy( strFormatCopy, m_strFormat.c_str() );
  const char* strToken{ strtok( strFormatCopy, strDelimiter ) };
  while( strToken )
  {
    // Convert to double
    char* strEnd{ nullptr };
    double fVal{ strtod( strToken, &strEnd ) };
    fVal = abs( fVal );

    // How should the column be justified
    AlignType eAlignment{ ALIGN_LEFT };
    if( fVal >= static_cast<double>( ALIGN_RIGHT ) )
    {
      eAlignment = ALIGN_RIGHT;
      fVal -= ALIGN_RIGHT;
    }
    else if( fVal >= static_cast<double>( ALIGN_CENTER ) )
    {
      eAlignment = ALIGN_CENTER;
      fVal -= ALIGN_CENTER;
    }

    // Valid range is 0.0 to 1.0
    rumNumberUtils::Clamp<double>( fVal, 0.0, 1.0 );

    const uint32_t uiOffset{ static_cast<uint32_t>( GetWidth() * fVal ) };

    // If the new column offset differs from previous, store the value
    if( static_cast<int32_t>( uiOffset ) > iLastOffset )
    {
      OffsetEntry cOffsetEntry;
      cOffsetEntry.m_uiOffset = uiOffset;
      cOffsetEntry.m_eAlignment = eAlignment;

      m_vColumnOffsets.push_back( cOffsetEntry );
      iLastOffset = static_cast<int32_t>( uiOffset );
    }

    // Get next token
    strToken = strtok( nullptr, strDelimiter );
  }

  delete[] strFormatCopy;

  uint32_t uiIndex{ 0 };
  for( const auto& iter : m_vEntries )
  {
    DrawEntry( uiIndex++, iter );
  }

  return bResult;
}
