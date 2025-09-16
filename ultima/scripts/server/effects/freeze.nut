class Freeze_Effect extends Effect
{
  static s_fDuration = 15.0;
  static s_fInterval = 3.0;
  static s_ePropertyID = Frozen_PropertyID;


  function Update()
  {
    if( m_uiTargetID != rumInvalidGameID )
    {
      local ciTarget = ::rumFetchPawn( m_uiTargetID );
      if( ciTarget != null && ciTarget.IsFrozen() )
      {
        if( ciTarget.SkillRoll( ciTarget.GetStrength() ) )
        {
          Expire();
        }
        else
        {
          ::rumSchedule( this, Update, s_fInterval );
        }
      }
    }
  }
}
