class Light_Effect extends Effect
{
  static s_fDuration = 300.0;
  static s_iRange = 5;

  m_bMagical = false;


  constructor( i_bMagical )
  {
    m_bMagical = i_bMagical;
  }


  function Expire()
  {
    base.Expire();

    if( m_uiTargetID != rumInvalidGameID )
    {
      local ciTarget = ::rumFetchPawn( m_uiTargetID );
      if( ciTarget != null )
      {
        ciTarget.SetLightRange( 0 );
      }
    }
  }
}
