// Received from client when player peers at a gem
// Sent from server to inform client to start peering
class Player_Peer_Broadcast extends rumBroadcast
{
  var = false;

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var = vargv[0]; // Client request to start or stop
    }
  }

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local bPeer = var;
    if( bPeer )
    {
      if( !( i_ciPlayer.IsIncapacitated() || i_ciPlayer.IsNegated() || i_ciPlayer.IsDead() ) )
      {
        local ePropertyID = U4_Gems_PropertyID;
        local eVersion = i_ciPlayer.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
        switch( eVersion )
        {
          case GameType.Ultima1: ePropertyID = null; break;
          case GameType.Ultima2: ePropertyID = U2_Helms_PropertyID; break;
          case GameType.Ultima3: ePropertyID = U3_Gems_PropertyID; break;
        }

        // Player wants to peer at a gem - make sure player meets all peering requirements
        local iGems = i_ciPlayer.GetProperty( ePropertyID, 0 );
        if( iGems > 0 )
        {
          // Consume a gem
          i_ciPlayer.SetProperty( ePropertyID, iGems - 1 );

          // Tell the client to start peering
          local ciBroadcast = ::rumCreate( Player_Peer_BroadcastID, true );
          ::rumSendPrivate( i_iSocket, ciBroadcast );

          // Create the peer effect
          local ciEffect = Peer_Effect();
          ciEffect.m_uiTargetID = i_ciPlayer.GetID();

          // Put the effect in the target's effect table
          i_ciPlayer.m_ciEffectsTable[ciEffect] <- ciEffect;

          // The effect expires after the specified duration
          ::rumSchedule( ciEffect, ciEffect.Expire, ciEffect.s_fGemDuration );
        }
        else
        {
          // Ultima IV says Peer at What? but None Owned! should suffice
          i_ciPlayer.ActionFailed( msg_none_owned_client_StringID );
        }
      }
      else
      {
        // Players can't peer at a gem when unconscious, frozen, or dead
        i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      }
    }
    else
    {
      // Player requests to stop peering before the effect expires
      i_ciPlayer.StopPeering();
    }

    local fDelay = i_ciPlayer.GetActionDelay( ActionDelay.Short );
    ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
  }
}
