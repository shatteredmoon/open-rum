class Jinx_Effect extends Effect
{
  static s_fDuration = 30.0;
  static s_fInterval = 5.0;
  static s_ePropertyID = Jinxed_PropertyID;


  function Expire()
  {
    base.Expire();
    local ciTarget = ::rumFetchPawn( m_uiTargetID );
    if( ciTarget != null )
    {
      ciTarget.OnJinxEnd();
    }
  }


  function Update()
  {
    if( m_uiTargetID != rumInvalidGameID )
    {
      local ciTarget = ::rumFetchPawn( m_uiTargetID );
      if( ciTarget != null && ciTarget.IsJinxed() )
      {
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
  }
}
