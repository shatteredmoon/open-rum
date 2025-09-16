// Received from client
class Player_Jimmy_Broadcast extends rumBroadcast
{
  var = 0;

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( i_ciPlayer instanceof Player )
    {
      local eDir = var;
      local ciDir = i_ciPlayer.GetPosition() + GetDirectionVector( eDir );

      local eDelay = ActionDelay.Short;

      local ciMap = i_ciPlayer.GetMap();
      local bFound = false;
      local ciPosData = ciMap.GetPositionData( ciDir );
      local ciDoor;
      while( ciDoor = ciPosData.GetNext( rumWidgetPawnType ) )
      {
        if( ciDoor.IsVisible() &&
          ( ciDoor instanceof U4_Door_Locked_Widget || ciDoor instanceof U3_Door_Locked_Widget ||
            ciDoor instanceof U2_Door_Locked_Widget || ciDoor instanceof U1_Door_Locked_Widget ||
            ciDoor instanceof U1_Door_Princess_Widget ) )
        {
          bFound = true;
          ciDoor.Unlock( i_ciPlayer );
          ciPosData.Stop();

          eDelay = ActionDelay.Short;
        }
      }

      if( !bFound )
      {
        i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
      }

      local fDelay = i_ciPlayer.GetActionDelay( eDelay );
      ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
    }
  }
}
