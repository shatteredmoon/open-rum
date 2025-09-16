class TextBox extends rumTextBox
{
  m_funcAccept = null;
  m_funcCancel = null;
  m_funcKeyPress = null;


  function ClearHandlers()
  {
    m_funcAccept = null;
    m_funcCancel = null;
    m_funcKeyPress = null;
  }


  function OnKeyPressed( i_ciKeyInput )
  {
    local eKey = i_ciKeyInput.GetKey();

    if( i_ciKeyInput.CtrlPressed() )
    {
      if( rumKeypress.KeyC() == eKey )
      {
        CopyTextToClipboard();
      }
      else if( rumKeypress.KeyV() == eKey )
      {
        PasteTextFromClipboard();
      }
      else if( rumKeypress.KeyA() == eKey )
      {
        SelectAll();
      }
    }
    else if( rumKeypress.KeyLeft() == eKey )
    {
      CursorLeft( i_ciKeyInput.ShiftPressed() );
    }
    else if( rumKeypress.KeyRight() == eKey )
    {
      CursorRight( i_ciKeyInput.ShiftPressed() );
    }
    else if( ( rumKeypress.KeyEnter() == eKey ) || ( rumKeypress.KeyPadEnter() == eKey ) )
    {
      if( m_funcAccept != null )
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
    else if( rumKeypress.KeyHome() == eKey )
    {
      CursorHome( i_ciKeyInput.ShiftPressed() );
    }
    else if( rumKeypress.KeyEnd() == eKey )
    {
      CursorEnd( i_ciKeyInput.ShiftPressed() );
    }
    else if( rumKeypress.KeyBackspace() == eKey )
    {
      CharacterBackspace();
    }
    else if( rumKeypress.KeyDelete() == eKey )
    {
      CharacterDelete();
    }
    else if( i_ciKeyInput.IsPrintable() )
    {
      CharacterAdd( i_ciKeyInput.GetAscii() );
    }
    else if( m_funcKeyPress != null )
    {
      m_funcKeyPress.call( this, i_ciKeyInput );
    }
  }


  function OnKeyRepeated( i_ciKeyInput )
  {
    OnKeyPressed( i_ciKeyInput );
  }
}
