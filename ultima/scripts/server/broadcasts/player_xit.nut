// Received from client when player wants to exit a transport
// Sent from server when a player leaves a transport
class Player_Xit_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;

  constructor( ... )
  {
    base.constructor();

    if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Transport ID
      var2 = vargv[1]; // Player ID
    }
  }

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local eDir = var1;

    if( i_ciPlayer instanceof Player )
    {
      local eDelay = ActionDelay.Short;

      local ciTransport = i_ciPlayer.GetTransport();
      if( ciTransport != null && ( ciTransport instanceof Transport_Widget ) )
      {
        local ciPos = i_ciPlayer.GetPosition() + GetDirectionVector( eDir );
        local bExited = ciTransport.Exit( i_ciPlayer, ciPos, i_ciPlayer.s_eDefaultMoveType );
        if( !bExited )
        {
          i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
        }
        else
        {
          eDelay = ActionDelay.Long;
        }
      }
      else
      {
        // Player is trying to exit a transport that doesn't exist
        i_ciPlayer.IncrementHackAttempts();
      }

      local fDelay = i_ciPlayer.GetActionDelay( eDelay );
      ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
    }
  }
}
