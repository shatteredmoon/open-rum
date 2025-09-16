class ListView extends rumListView
{
  m_funcAccept = null;
  m_funcCancel = null;
  m_funcIndexChanged = null;
  m_funcKeyPress = null;
  m_funcMouseMoved = null;


  function ClearHandlers()
  {
    m_funcAccept = null;
    m_funcCancel = null;
    m_funcIndexChanged = null;
    m_funcKeyPress = null;
    m_funcMouseMoved = null;
  }


  function OnIndexChanged( i_iIndex )
  {
    if( m_funcIndexChanged != null )
    {
      m_funcIndexChanged.call( this, i_iIndex );
    }
  }


  function OnKeyPressed( i_ciKeyInput )
  {
    local eKey = i_ciKeyInput.GetKey();

    if( ( rumKeypress.KeyUp() == eKey ) || ( rumKeypress.KeyPad8() == eKey ) )
    {
      MovePrev();
    }
    else if( ( rumKeypress.KeyDown() == eKey ) || ( rumKeypress.KeyPad2() == eKey ) )
    {
      MoveNext();
    }
    else if( ( rumKeypress.KeyEnter() == eKey ) || ( rumKeypress.KeyPadEnter() == eKey ) ||
             SelectItemByShortcut( eKey ) )
    {
      if( SupportsMultiSelect() )
      {
        if( m_funcAccept != null && ( ( rumKeypress.KeyEnter() == eKey ) || ( rumKeypress.KeyPadEnter() == eKey ) ) )
        {
          m_funcAccept.call( this );
        }
      }
      else if( m_funcAccept != null )
      {
        m_funcAccept.call( this );
      }
    }
    else if( rumKeypress.KeySpace() == eKey )
    {
      SelectCurrent();

      if( !SupportsMultiSelect() && m_funcAccept != null )
      {
        m_funcAccept.call( this );
      }
    }
    else if( rumKeypress.KeyEscape() == eKey )
    {
      if( m_funcCancel != null )
      {
        m_funcCancel.call( this );
      }
    }
    else if( rumKeypress.KeyPageUp() == eKey )
    {
      PageUp();
    }
    else if( rumKeypress.KeyPageDown() == eKey )
    {
      PageDown();
    }
    else if( rumKeypress.KeyEnd() == eKey )
    {
      End();
    }
    else if( rumKeypress.KeyHome() == eKey )
    {
      Home();
    }
    else if( rumKeypress.KeyTab() == eKey )
    {
      FocusNext();
    }
    else if( rumKeypress.KeyF1() != eKey && m_funcKeyPress != null )
    {
      m_funcKeyPress.call( this, i_ciKeyInput );
    }

    // TODO - selection by letter or number
    //if( SupportsMultiSelect() )
    //{
    //  // Multiple items can be selected
    //  if( SelectItemByShortcut( eKey ) )
    //  {
    //    ListSelectionShowCurrentText();
    //  }
    //}
    //else
    //{
    //  // Treat selection like the enter key when only one object is selectable
    //  if( SelectItemByShortcut( eKey ) )
    //  {
    //    ListSelectionShowCurrentText();
    //    g_ciUI.m_ciGameInputTextBox.ResetInputMode( false, GetSelectedKeys() );
    //  }
    //}
  }


  function OnKeyRepeated( i_ciKeyInput )
  {
    OnKeyPressed( i_ciKeyInput );
  }


  function OnMouseButtonPressed( i_eButton, i_ciPoint )
  {
    if( rumLeftMouseButton != i_eButton )
    {
      return;
    }

    Focus();

    if( m_funcAccept != null )
    {
      m_funcAccept.call( this );
    }
  }


  function OnMouseMoved( i_ciPoint )
  {
    local uiIndex = GetSelectedIndex();
    local iNewIndex = GetIndexAtScreenPosition( i_ciPoint );
    if( iNewIndex >= 0 && uiIndex != iNewIndex )
    {
      SetCurrentIndex( iNewIndex );

      if( m_funcMouseMoved != null )
      {
        m_funcMouseMoved.call( this, i_ciPoint );
      }
    }
  }


  function OnMouseScrolled( i_ciDirection, i_ciPoint )
  {
    if( i_ciDirection.y < 0 )
    {
      ScrollDown( g_ciUI.s_iDefaultLabelHeight );
    }
    else if( i_ciDirection.y > 0 )
    {
      ScrollUp( g_ciUI.s_iDefaultLabelHeight );
    }

    local uiIndex = GetSelectedIndex();
    local iNewIndex = GetIndexAtScreenPosition( i_ciPoint );
    if( iNewIndex >= 0 && uiIndex != iNewIndex )
    {
      SetCurrentIndex( iNewIndex );
    }
  }
}
