class Peer_Effect extends Effect
{
  static s_fGemDuration = 10.0;
  static s_fSpellDuration = 15.0;


  function Expire()
  {
    if( m_uiTargetID != rumInvalidGameID )
    {
      local ciTarget = ::rumFetchPawn( m_uiTargetID );
      if( ciTarget != null )
      {
        delete ciTarget.m_ciEffectsTable[this];

        // The target might have other peering effects active, so only stop peering if no other peer effect is found
        if( !ciTarget.HasEffect( Peer_Effect ) )
        {
          local ciBroadcast = ::rumCreate( Player_Peer_BroadcastID, false );
          ciTarget.SendBroadcast( ciBroadcast );
        }
      }
    }
  }


  function ExpireAll()
  {
    if( m_uiTargetID != rumInvalidGameID )
    {
      local ciTarget = ::rumFetchPawn( m_uiTargetID );
      if( ciTarget != null )
      {
        delete ciTarget.m_ciEffectsTable[this];

        local ciEffect;
        while( ciEffect = ciTarget.GetEffect( Peer_Effect ) )
        {
          delete ciTarget.m_ciEffectsTable[ciEffect];
        }

        local ciBroadcast = ::rumCreate( Player_Peer_BroadcastID, false );
        ciTarget.SendBroadcast( ciBroadcast );
      }
    }
  }
}
