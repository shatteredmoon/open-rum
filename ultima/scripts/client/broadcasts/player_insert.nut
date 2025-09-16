// Sent from client when player inserts a card into Exodus
// Received from server with results
class Player_Insert_Broadcast extends rumBroadcast
{
  var1 = 0; // Card type
  var2 = 0; // Direction

  constructor( ... )
  {
    base.constructor();

    if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Card Type
      var2 = vargv[1]; // Direction
    }
  }

  function OnRecv()
  {
    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();

    ciMap.DestroyStart();
  }
}
