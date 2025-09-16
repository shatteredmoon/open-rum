// Received from client when player searches
class Player_Search_Broadcast extends rumBroadcast
{
  var = 0;

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local eDelay = ActionDelay.Short;

    if( !( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() ) )
    {
      if( !i_ciPlayer.IsFlying() )
      {
        local ciMap = i_ciPlayer.GetMap();
        ciMap.Search( i_ciPlayer );
        eDelay = ActionDelay.Long;
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_must_land_client_StringID );
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
