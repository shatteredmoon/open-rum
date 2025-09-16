// Sent from client when player wants to exit a transport
// Received from server when a player leaves a transport
class Player_Xit_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Direction type
    }
  }

  function OnRecv()
  {
    local ciTransport = ::rumFetchPawn( var1 );
    if( ciTransport != null && ( ciTransport instanceof Transport_Widget ) )
    {
      ciTransport.RemovePassenger( var2 );

      local ciPlayer = ::rumGetMainPlayer();
      ciPlayer.UpdateGraphic();
    }
  }
}
