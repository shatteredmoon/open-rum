class Trigger_Widget extends Widget
{
  static s_bServerOnly = true;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );
  }


  function AffectCreature( i_ciCreature )
  {
    if( !IsVisible() || ( i_ciCreature.GetMoveType() != MoveType.Terrestrial ) )
    {
      return;
    }

    if( ( i_ciCreature instanceof Player ) && !i_ciCreature.IsDead() )
    {
      local ciMap = GetMap();
      local ciPawnArray = ciMap.GetAllPawns();
      foreach( ciPawn in ciPawnArray )
      {
        if( ( ciPawn instanceof Widget ) && !( ciPawn instanceof Trigger_Widget ) )
        {
          if( ciPawn.m_iGroupID == m_iGroupID )
          {
            local eTriggerType = GetProperty( Widget_Trigger_Type_PropertyID, FloorTriggerType.ShowGroupWidgets );
            if( FloorTriggerType.ShowGroupWidgets == eTriggerType )
            {
              ciPawn.SetVisibility( true );
            }
            else if( FloorTriggerType.HideGroupWidgets == eTriggerType )
            {
              ciPawn.SetVisibility( false );
            }
          }
        }
      }

      // Hide/disable the trigger itself for a short while
      SetVisibility( false );

      ciMap.GroupTriggered( m_iGroupID );

      // The trigger won't work again until the respawn interval has passed
      ScheduleRespawn();

      // Schedule a reset of default visibility for all objects in this trigger group
      ::rumSchedule( this, ResetTriggerGroup, ciMap.s_fResetGroupTime );
    }
  }


  function ResetTriggerGroup()
  {
    local ciMap = GetMap();
    ciMap.ResetExpiredTriggerGroups();
  }
}
