// Sent from client when player peers at a gem
// Received from server to start peering
class Player_Peer_Broadcast extends rumBroadcast
{
  var = false;

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var = vargv[0]; // Peer start or stop
    }
  }

  function OnRecv()
  {
    if( g_ciCUO.m_bPeering != var )
    {
      // Peering status has changed
      local ciPlayer = ::rumGetMainPlayer();
      g_ciCUO.m_bPeering = var;
      local ciMap = ciPlayer.GetMap();
      ciMap.SetPeeringAttributes();
    }
  }
}
