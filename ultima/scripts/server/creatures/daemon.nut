class U1_Daemon_Creature extends U1_NPC
{
  function OnDeath( i_ciSource, i_bKillCredit = true )
  {
    base.OnDeath( i_ciSource, i_bKillCredit );

    if( i_bKillCredit && ( i_ciSource instanceof Player ) )
    {
      // Credit for quest?
      local eQuestState = i_ciSource.GetProperty( U1_Quest_LostKing_PropertyID, U1_KingQuestState.Unbestowed1 );
      if( U1_KingQuestState.Bestowed2 == eQuestState )
      {
        i_ciSource.SetProperty( U1_Quest_LostKing_PropertyID, U1_KingQuestState.Completed2 );
        i_ciSource.ActionSuccess( u1_king_quest_completed_client_StringID );
      }
    }
  }
}


class U4_Daemon_Creature extends U4_NPC
{
  function AIAttack( i_ciAttackTarget )
  {
    local ciMap = GetMap();
    if( ciMap.GetAssetID() == U4_Shrine_Humility_MapID )
    {
      local ciTarget = ::rumFetchPawn( i_ciAttackTarget.m_uiTargetID );
      if( ciTarget != null && ciTarget.IsDaemonImmune() )
      {
        // Abandon the target
        AIForgetTarget();
        ForgetPath();

        return ActionDelay.Short;
      }
    }

    return base.AIAttack( i_ciAttackTarget );
  }


  function IsTargetValid( i_ciTarget )
  {
    local ciMap = GetMap();
    if( ciMap != null && ( ciMap.GetAssetID() == U4_Shrine_Humility_MapID ) )
    {
      local ciTarget = ::rumFetchPawn( m_uiTargetID );
      if( ciTarget != null )
      {
        return ciTarget.IsDaemonImmune();
      }
    }

    return base.IsTargetValid( i_ciTarget );
  }
}
