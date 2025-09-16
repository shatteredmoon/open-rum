class Sleep_Effect extends Effect
{
  static s_fDuration = 15.0;
  static s_fInterval = 3.0;
  static s_ePropertyID = Unconscious_PropertyID;


  function Update()
  {
    if( rumInvalidGameID == m_uiTargetID )
    {
      return;
    }

    local ciTarget = ::rumFetchPawn( m_uiTargetID );
    if( ( null == ciTarget ) || ciTarget.IsUnconscious() )
    {
      return;
    }

    if( ciTarget.SkillRoll( ciTarget.GetIntelligence() ) )
    {
      Expire();
    }
    else
    {
      ::rumSchedule( this, Update, s_fInterval );
    }
  }
}
