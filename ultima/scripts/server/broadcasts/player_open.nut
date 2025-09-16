// Received from client
class Player_Open_Broadcast extends rumBroadcast
{
  var = 0;

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( i_ciPlayer instanceof Player )
    {
      local eDir = var;
      local ciDir = i_ciPlayer.GetPosition() + GetDirectionVector( eDir );

      local eDelay = ActionDelay.Short;

      local cDoor = U4_Door_Widget;
      local eVersion = i_ciPlayer.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
      {
        switch( eVersion )
        {
          case GameType.Ultima1: cDoor = U1_Door_Widget; break;
          case GameType.Ultima2: cDoor = U2_Door_Widget; break;
          case GameType.Ultima3: cDoor = U3_Door_Widget; break;
        }
      }

      local ciMap = i_ciPlayer.GetMap();
      local bFound = false;
      local ciPosData = ciMap.GetPositionData( ciDir );
      local ciDoor;


      while( ciDoor = ciPosData.GetNext( rumWidgetPawnType ) )
      {

        if( ciDoor.IsVisible() && ciDoor instanceof cDoor )
        {

          bFound = true;
          ciDoor.Open( i_ciPlayer );
          ciPosData.Stop();

          eDelay = ActionDelay.Long;
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
