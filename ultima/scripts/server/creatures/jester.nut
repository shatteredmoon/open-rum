class U1_Jester_Creature extends U1_NPC
{
  function OnDeath( i_ciSource, i_bKillCredit = true )
  {
    base.OnDeath( i_ciSource, i_bKillCredit );

    if( i_bKillCredit && ( i_ciSource instanceof Player ) )
    {
      i_ciSource.AdjustVersionedProperty( g_eKeysPropertyVersionArray, 1 );
      i_ciSource.ActionSuccess( msg_found_jester_key_client_StringID );
    }
  }


  function OnSteal( i_ciPlayer )
  {
    i_ciPlayer.AdjustVersionedProperty( g_eKeysPropertyVersionArray, 1 );
    i_ciPlayer.ActionSuccess( msg_found_jester_key_client_StringID );
  }
}
