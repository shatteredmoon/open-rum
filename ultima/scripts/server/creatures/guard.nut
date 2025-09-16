class U2_Guard_Creature extends U2_NPC
{
  function OnDeath( i_ciSource, i_bKillCredit = true )
  {
    base.OnDeath( i_ciSource, i_bKillCredit );

    if( i_bKillCredit && ( i_ciSource instanceof Player ) )
    {
      local iNewAmount = i_ciSource.GetProperty( U2_Keys_PropertyID, 0 ) + 1;
      i_ciSource.SetProperty( U2_Keys_PropertyID, iNewAmount );
    }
  }


  function OnSteal( i_ciPlayer )
  {
    i_ciPlayer.AdjustVersionedProperty( g_eKeysPropertyVersionArray, 1 );
    i_ciPlayer.ActionSuccess( msg_found_jester_key_client_StringID );
  }
}


class U3_Guard_Creature extends U3_NPC
{
  static s_iBribeCost = 100;
  static s_fBribeDuration = 60.0;

  m_iBribeIndex = 0;


  function Bribe()
  {
    SetProperty( Creature_Bribed_PropertyID, true );
    ::rumSchedule( this, BribeDone, s_fBribeDuration, ++m_iBribeIndex );
  }


  function BribeDone( i_iBribeIndex )
  {
    if( ( i_iBribeIndex == m_iBribeIndex ) && GetProperty( Creature_Bribed_PropertyID, false ) )
    {
      // No longer bribed
      RemoveProperty( Creature_Bribed_PropertyID );
    }
  }
}
