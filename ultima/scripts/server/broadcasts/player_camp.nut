// Received from client when player creates a campfire
// Send from server with an error message
class Player_Camp_Broadcast extends rumBroadcast
{
  var = 0;

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var = vargv[0]; // Error string
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local eDelay = ActionDelay.Short;

    if( !( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() ) )
    {
      local ciMap = i_ciPlayer.GetMap();
      local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
      if( ( MapType.World == eMapType ) || ( MapType.Dungeon == eMapType ) || ( MapType.Abyss == eMapType ) ||
          ( MapType.Altar == eMapType ) || ( MapType.Cave == eMapType ) )
      {
        if( i_ciPlayer.GetTransportID() == rumInvalidGameID )
        {
          // The player can't be on the same space as another pawn
          local ciPosData = ciMap.GetPositionData( i_ciPlayer.GetPosition() );
          if( ciPosData.GetNumObjects() <= 1 )
          {
            local bSuccess = false;
            local ciCampfire = null;

            local ciPos = i_ciPlayer.GetPosition() + GetDirectionVector( var );
            ciPosData = ciMap.GetPositionData( ciPos );
            while( ciCampfire = ciPosData.GetNext( rumWidgetPawnType, U4_Spit_WidgetID ) )
            {
              if( ciCampfire.IsVisible() )
              {
                bSuccess = true;
                ciPosData.Stop();
              }
            }

            if( !bSuccess )
            {
              // Verify that nothing is blocking the new campfire
              if( ( ciPosData.GetNumObjects() == 0 ) &&
                  ( ciMap.MovePawn( i_ciPlayer, ciPos, rumTestMoveFlag ) == rumSuccessMoveResultType ) )
              {
                local iNumTorches = i_ciPlayer.GetProperty( U4_Torches_PropertyID, 0 );

                // If the player isn't on the world map, there's a cost to building a fire
                if( ( MapType.World == eMapType ) || iNumTorches >= g_iTorchesForCampfire )
                {
                  ciCampfire = ::rumCreate( U4_Spit_WidgetID );
                  if( ciCampfire != null && ciMap.AddPawn( ciCampfire, ciPos ) )
                  {
                    if( MapType.World != eMapType )
                    {
                      // Consume the player's torches and schedule the campfire for expiration
                      i_ciPlayer.SetProperty( U4_Torches_PropertyID, iNumTorches - g_iTorchesForCampfire );
                    }

                    local fExpireInterval = ciCampfire.GetProperty( Expiration_Interval_PropertyID, 0.0 );
                    ::rumSchedule( ciCampfire, ciCampfire.Expire, fExpireInterval );
                    bSuccess = true;
                  }
                  else
                  {
                    i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
                  }
                }
                else
                {
                  // A simple ActionFailed won't work here, because the string has a specifier
                  local ciBroadcast = ::rumCreate( Player_Camp_BroadcastID, "msg_camp_req_torches" );
                  i_ciPlayer.SendBroadcast( ciBroadcast );
                }
              }
              else
              {
                i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
              }
            }

            if( bSuccess )
            {
              i_ciPlayer.Camp();
              eDelay = ActionDelay.Long;
            }
          }
          else
          {
            i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
          }
        }
        else
        {
          ciPlayerActionFailed( msg_only_on_foot_client_StringID );
        }
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_outdoors_only_client_StringID );
        i_ciPlayer.IncrementHackAttempts();
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
    }

    local fDelay = i_ciPlayer.GetActionDelay( eDelay );
    ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
  }
}
