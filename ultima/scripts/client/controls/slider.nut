class Slider extends rumSlider
{
  m_funcAccept = null;
  m_funcCancel = null;
  m_funcKeyPress = null;
  m_funcValueChanged = null;


  function ClearHandlers()
  {
    m_funcAccept = null;
    m_funcCancel = null;
    m_funcKeyPress = null;
    m_funcValueChanged = null;
  }


  function OnKeyPressed( i_ciKeyInput )
  {
    local eKey = i_ciKeyInput.GetKey();

    if( ( rumKeypress.KeyLeft() == eKey ) || ( rumKeypress.KeyDown() == eKey ) )
    {
      DecrementValue();
    }
    else if( ( rumKeypress.KeyRight() == eKey ) || ( rumKeypress.KeyUp() == eKey ) )
    {
      IncrementValue();
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
      SetValue( GetMinValue() );
    }
    else if( rumKeypress.KeyEnd() == eKey )
    {
      SetValue( GetMaxValue() );
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


  function OnMouseScrolled( i_ciDirection, i_ciPoint )
  {
    if( i_ciDirection.y < 0 )
    {
      DecrementValue();
    }
    else if( i_ciDirection.y > 0 )
    {
      IncrementValue();
    }
  }


  function OnValueChanged( i_iValue )
  {
    if( m_funcValueChanged != null )
    {
      m_funcValueChanged.call( this, i_iValue );
    }
  }
}
