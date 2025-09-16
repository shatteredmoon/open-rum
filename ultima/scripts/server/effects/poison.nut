class Poison_Effect extends Effect
{
  static s_fInterval = 3.0;
  static s_iDamage = 5;


  // TODO - put effect indices on the effect itself?

  function Remove()
  {
    if( m_uiTargetID != rumInvalidGameID )
    {
      local ciTarget = ::rumFetchPawn( m_uiTargetID );
      if( ciTarget != null )
      {
        // Note: poison is persistent on players, so don't modify the property here!
        delete ciTarget.m_ciEffectsTable[this];
      }
    }
  }


  function Update( i_iPoisonIndex )
  {
    if( m_uiTargetID != rumInvalidGameID )
    {
      local ciTarget = ::rumFetchPawn( m_uiTargetID );
      if( ciTarget != null && ciTarget.IsPoisoned() && ( i_iPoisonIndex == ciTarget.m_iPoisonIndex ) )
      {
        // Players are not affected by poison if dead or frozen
        if( !( ciTarget.IsDead() || ciTarget.IsFrozen() ) )
        {
          ciTarget.Damage( rand() % s_iDamage + 1, this );
          ::rumSchedule( this, Update, s_fInterval, i_iPoisonIndex );
        }
      }
    }
  }
}
