class Region extends rumRegion
{
  m_funcKeyPress = null;
  m_funcMouseButtonPress = null;
  m_funcMouseMoved = null;


  function ClearHandlers()
  {
    m_funcKeyPress = null;
    m_funcMouseButtonPress = null;
    m_funcMouseMoved = null;
  }


  function OnKeyPressed( i_ciKeyInput )
  {
    if( null == i_ciKeyInput )
    {
      return;
    }

    local eKey = i_ciKeyInput.GetKey();
    if( rumKeypress.KeyF1() != eKey && m_funcKeyPress != null )
    {
      m_funcKeyPress.call( this, i_ciKeyInput );
    }
  }


  function OnMouseButtonPressed( i_eButton, i_ciPoint )
  {
    if( m_funcMouseButtonPress != null )
    {
      m_funcMouseButtonPress.call( this, i_eButton, i_ciPoint );
    }
  }


  function OnMouseMoved( i_ciPoint )
  {
    if( m_funcMouseMoved != null )
    {
      m_funcMouseMoved.call( this, i_ciPoint );
    }
  }
}
