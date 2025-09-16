class Effect
{
  static s_fDuration = 10.0;
  static s_fInterval = 2.0;
  static s_ePropertyID = rumInvalidAssetID;

  m_uiTargetID = rumInvalidGameID;


  function Expire()
  {
    if( m_uiTargetID != rumInvalidGameID )
    {
      local ciTarget = ::rumFetchPawn( m_uiTargetID );
      if( ciTarget != null )
      {
        delete ciTarget.m_ciEffectsTable[this];
        if( s_ePropertyID != rumInvalidAssetID )
        {
          ciTarget.RemoveProperty( s_ePropertyID );
        }
      }
    }
  }

  function Remove()
  {
    if( m_uiTargetID != rumInvalidGameID )
    {
      local ciTarget = ::rumFetchPawn( m_uiTargetID );
      if( ciTarget != null )
      {
        delete ciTarget.m_ciEffectsTable[this];
        if( s_ePropertyID != rumInvalidAssetID )
        {
          ciTarget.RemoveProperty( s_ePropertyID );
        }
      }
    }
  }

  function Update()
  {
    ::rumSchedule( this, Update, s_fInterval );
  }
}
