// Received from client
class Player_Move_Broadcast extends rumBroadcast
{
  var = 0;

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local eDir = var;
    local ciMap = i_ciPlayer.GetMap();
    local ciPos = i_ciPlayer.GetPosition() + GetDirectionVector( eDir );

    local eResult;

    local eDelay = ActionDelay.Short;

    // If player is on a transport, move the actual transport
    local ciTransport = i_ciPlayer.GetTransport();
    if( ciTransport != null )
    {
      eResult = ciTransport.TryMove( i_ciPlayer, ciPos, eDir );
    }
    else
    {
      eResult = ciMap.MovePawn( i_ciPlayer, ciPos );
    }

    if( rumSuccessMoveResultType == eResult )
    {
      if( ciTransport != null && ciTransport.GetMoveType() == MoveType.Sails )
      {
        // Determine movement delay based on wind direction
        local iMinDistance = 0;

        local eWindDirection = Direction.None;

        local eVersion = ciMap.GetProperty( Ultima_Version_PropertyID, 0 );
        if( GameType.Ultima4 == eVersion )
        {
          // This is a U4 map, so there could be a Wind spell in effect on the player
          eWindDirection = i_ciPlayer.GetProperty( U4_Wind_Direction_Override_PropertyID, Direction.None );
        }

        if( Direction.None == eWindDirection )
        {
          // Current wind direction on all worlds
          eDir = g_ciServer.m_ciUltima4World.m_eWindDirection;
        }

        if( eDir != eWindDirection )
        {
          local iClockwiseTurns = 0;
          local iCounterClockwiseTurns = 0;
          if( eDir > eWindDirection )
          {
            iClockwiseTurns = ( eWindDirection + 8 ) - eDir;
            iCounterClockwiseTurns = eDir - eWindDirection;
          }
          else
          {
            iClockwiseTurns = eWindDirection - eDir;
            iCounterClockwiseTurns = eDir - ( eWindDirection - 8 );
          }

          iMinDistance = min( iClockwiseTurns, iCounterClockwiseTurns );
        }

        if( iMinDistance > 2 )
        {
          // Sailing against the wind
          eDelay = ActionDelay.Long;
        }
        else if( iMinDistance > 0 )
        {
          // Sailing somewhat with or perpendicular to the wind
          eDelay = ActionDelay.Medium;
        }
      }
      else
      {
        // Determine movement delay based on the tile traversed
        local ciPosData = ciMap.GetPositionData( ciPos );
        local ciTile = ::rumGetTileAsset( ciPosData.GetTileID() );
        local fWeight = ciTile.GetWeight();
        if( fWeight >= 2.0 )
        {
          eDelay = ActionDelay.Long;
        }
        else if( fWeight >= 1.0 )
        {
          eDelay = ActionDelay.Medium;
        }
        else
        {
          eDelay = ActionDelay.Short;
        }
      }
    }
    else
    {
      switch( eResult )
      {
        case rumTileCollisionMoveResultType:
          i_ciPlayer.ActionFailed( msg_blocked_client_StringID );
          break;

        case rumPawnCollisionMoveResultType:
          i_ciPlayer.ActionFailed( msg_blocked_client_StringID );

          // See if the collision was with a wandering NPC
          local ciPosData = ciMap.GetPositionData( ciPos );
          local ciCreature = null;
          while( ciCreature = ciPosData.GetNext( rumCreaturePawnType ) )
          {
            if( ciCreature instanceof NPC )
            {
              ciCreature.AINudge();
              break;
            }
          }
          break;

        case rumOffMapMoveResultType:
          local eExitMapID = ciMap.GetExitMapID();
          local ciDestMap = GetOrCreateMap( i_ciPlayer, eExitMapID );

          // Get the destination map and location
          if( ciTransport )
          {
            ciMap.Exit( ciTransport, ciDestMap );
          }

          ciMap.Exit( i_ciPlayer, ciDestMap );

          if( ciTransport )
          {
            // Re-attach the player to the transport client-side
            local ciBroadcast = ::rumCreate( Player_Board_BroadcastID, ciTransport.GetID() );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
          break;

        case rumFailMoveResultType:
        case rumErrorMoveResultType:
          i_ciPlayer.ActionFailed( msg_failed_client_StringID );
          break;
      }
    }

    local fDelay = i_ciPlayer.GetActionDelay( ActionDelay.Short );
    ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
  }
}
