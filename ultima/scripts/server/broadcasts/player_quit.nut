// Received from client
class Player_Quit_Broadcast extends rumBroadcast
{
  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( i_ciPlayer != null && ( i_ciPlayer instanceof Player ) )
    {
      i_ciPlayer.Quit();
    }
  }
}
