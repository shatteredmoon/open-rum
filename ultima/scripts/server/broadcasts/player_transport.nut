// Received from client when player klimbs or descends on various transports (such as the balloon)
class Player_Transport_Broadcast extends rumBroadcast
{
  var1 = 0; // Command Type
  var2 = 0; // Direction or coordinates

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() >= 1 )
    {
      var1 = vargv[0];
      if( vargv.len() >= 2 )
      {
        var2 = vargv[1];
      }
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local eCommandType = var1;

    if( i_ciPlayer instanceof Player )
    {
      if( TransportCommandType.Movement == eCommandType )
      {
        local eVerticalDir = var2;

        local ciTransport = i_ciPlayer.GetTransport();
        if( ciTransport instanceof U4_Air_Balloon_Widget ||
            ciTransport instanceof U2_Plane_Widget ||
            ciTransport instanceof U2_Rocket_Widget ||
            ciTransport instanceof U1_Shuttle_Widget )
        {
          if( VerticalDirectionType.Up == eVerticalDir )
          {
            ciTransport.Ascend( i_ciPlayer );
          }
          else if( VerticalDirectionType.Down == eVerticalDir )
          {
            ciTransport.Descend( i_ciPlayer );
          }
          else
          {
            // Not an ascend or descend request
            i_ciPlayer.IncrementHackAttempts();
          }
        }
        else
        {
          // Player is not on a relevant transport
          i_ciPlayer.ActionFailed( msg_failed_client_StringID );
          i_ciPlayer.IncrementHackAttempts();
        }
      }
      else if( TransportCommandType.TimeMachineLaunchMondain == eCommandType )
      {
        // Make sure player is in the time machine and on the proper map
        local ciTransport = i_ciPlayer.GetTransport();
        if( ciTransport != null )
        {
          local ciMap = ciTransport.GetMap();
          if( ciMap.GetAssetID() == U1_Time_Machine_MapID )
          {
            ciTransport.Exit( i_ciPlayer, ciTransport.GetPosition(), MoveType.Terrestrial );

            // Transport the player to Mondain's Lair
            local ciDestMap = GetOrCreateMap( i_ciPlayer, U1_Lair_Mondain_MapID );
            if( ciDestMap != null )
            {
              ciMap.TransferPawn( i_ciPlayer, ciDestMap, rumPos( 2, 4 ) );
              local strDesc = ::rumGetString( msg_mondain_creating_gem_server_StringID, i_ciPlayer.m_iLanguageID );
              i_ciPlayer.ActionInfo( format("<b>%s" , strDesc ), false );
            }
          }
        }
      }
      else if( TransportCommandType.TimeMachineLaunchEnd == eCommandType )
      {
        // Make sure player is in the time machine and on the proper map
        local ciTransport = i_ciPlayer.GetTransport();
        if( ciTransport != null )
        {
          local ciMap = ciTransport.GetMap();
          if( ciMap.GetAssetID() == U1_Lair_Mondain_MapID )
          {
            ciTransport.Exit( i_ciPlayer, ciTransport.GetPosition(), MoveType.Terrestrial );

            // Transport the player to Lord British
            local ciDestMap = GetOrCreateMap( i_ciPlayer, U1_Castle_Lord_British_MapID );
            if( ciDestMap != null )
            {
              ciMap.TransferPawn( i_ciPlayer, ciDestMap, rumPos( 33, 5 ) );
            }
          }
        }
      }
      else if( TransportCommandType.HyperJump == eCommandType )
      {
        local ciTransport = i_ciPlayer.GetTransport();
        if( ciTransport != null )
        {
          ciTransport.HyperJump( i_ciPlayer, var2 );
        }
      }
      else
      {
        // Not an understood transport command type
        i_ciPlayer.IncrementHackAttempts();
      }

      local fDelay = i_ciPlayer.GetActionDelay( ActionDelay.Short );
      ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
    }
  }
}
