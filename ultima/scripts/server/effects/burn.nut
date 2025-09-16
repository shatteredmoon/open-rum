class Burn_Effect extends Effect
{
  static s_iDamage = 10;
  static s_ePropertyID = Burning_PropertyID;


  function Update()
  {
    if( m_uiTargetID != rumInvalidGameID )
    {
      local ciTarget = ::rumFetchPawn( m_uiTargetID );
      if( ciTarget != null )
      {
        if( ciTarget.SkillRoll( ciTarget.GetDexterity() ) )
        {
          Expire();
        }
        else if( !ciTarget.IsDead() )
        {
          ciTarget.Damage( rand() % s_iDamage + 1, this );
          ::rumSchedule( this, Update, s_fInterval );
        }
      }
    }
  }
}
