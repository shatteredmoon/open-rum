class Negate_Effect extends Effect
{
  static s_fDuration = 20.0;
  static s_fInterval = 5.0;


  function Expire()
  {
    if( m_uiTargetID != rumInvalidGameID )
    {
      local ciTarget = ::rumFetchPawn( m_uiTargetID );
      if( ciTarget != null )
      {
        if( ciTarget.IsNegated() )
        {
          delete ciTarget.m_ciEffectsTable[this];
          ciTarget.RemoveProperty( Negated_PropertyID );
          ciTarget.RemoveProperty( Last_Negate_Time_PropertyID );
        }

        if( ciTarget.IsJailed() )
        {
          ciTarget.Negate();
        }
      }
    }
  }


  function Update()
  {
    if( m_uiTargetID != rumInvalidGameID )
    {
      local ciTarget = ::rumFetchPawn( m_uiTargetID );
      if( ciTarget != null && ciTarget.IsNegated() )
      {
        if( !ciTarget.IsJailed() && ciTarget.SkillRoll( ciTarget.GetIntelligence() ) )
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
