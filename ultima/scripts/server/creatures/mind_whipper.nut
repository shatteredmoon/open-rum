class U1_Mind_Whipper_Creature extends U1_NPC
{
  function OnDeath( i_ciSource, i_bKillCredit = true )
  {
    base.OnDeath( i_ciSource, i_bKillCredit );

    if( i_bKillCredit && ( i_ciSource instanceof Player ) )
    {
      // Credit for quest?
      local eQuestState = i_ciSource.GetProperty( U1_Quest_Shamino_PropertyID, U1_KingQuestState.Unbestowed1 );
      if( U1_KingQuestState.Bestowed2 == eQuestState )
      {
        i_ciSource.SetProperty( U1_Quest_Shamino_PropertyID, U1_KingQuestState.Completed2 );
        i_ciSource.ActionSuccess( u1_king_quest_completed_client_StringID );
      }
    }
  }
}
