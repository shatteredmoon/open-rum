class U3_Exodus_Widget extends U3_Widget
{
  function Animate()
  {
    local eState = GetProperty( State_PropertyID, 0 );
    if( U3_ExodusComponentState.Functional == eState )
    {
      base.Animate();
    }
  }
}
