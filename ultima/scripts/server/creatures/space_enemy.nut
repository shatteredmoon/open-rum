class U1_Space_Enemy_Creature extends U1_NPC
{
  static s_iSpaceAceKillRequirement = 20;


  function OnDeath( i_ciSource, i_bKillCredit = true )
  {
    base.OnDeath( i_ciSource, i_bKillCredit );

    if( i_ciSource instanceof Player )
    {
      local iNumKilled = i_ciSource.GetProperty( U1_Space_Enemies_Killed_PropertyID, 0 ) + 1;

      // Credit towards Space Ace
      i_ciSource.SetProperty( U1_Space_Enemies_Killed_PropertyID, iNumKilled );

      if( s_iSpaceAceKillRequirement == iNumKilled )
      {
        // Tell the player they're a space ace
        i_ciSource.ActionInfo( msg_space_ace_awarded_client_StringID );
      }
    }
  }
}
