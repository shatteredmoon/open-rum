function CastView( i_ciCaster )
{
  if( i_ciCaster instanceof Player )
  {
    // Tell the client to start peering
    local ciBroadcast = ::rumCreate( Player_Peer_BroadcastID, true );
    i_ciCaster.SendBroadcast( ciBroadcast );

    // Create the spell effect
    local ciEffect = Peer_Effect();
    ciEffect.m_uiTargetID = i_ciCaster.GetID();

    // Put the effect in the target's effect table
    i_ciCaster.m_ciEffectsTable[ciEffect] <- ciEffect;

    // The spell expires after the specified duration
    ::rumSchedule( ciEffect, ciEffect.Expire, ciEffect.s_fSpellDuration );
  }
}
