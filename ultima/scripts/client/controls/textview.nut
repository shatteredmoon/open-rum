class TextView extends rumTextView
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

    if( ( rumKeypress.KeyEnter() == eKey ) || ( rumKeypress.KeyPadEnter() == eKey ) )
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
    else if( rumKeypress.KeyF1() != eKey && m_funcKeyPress != null )
    {
      m_funcKeyPress.call( this, i_ciKeyInput );
    }
  }


  function OnKeyRepeated( i_ciKeyInput )
  {
    OnKeyPressed( i_ciKeyInput );
  }
}
