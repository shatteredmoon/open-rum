// Sent from client when player wants to board a transport
// Received from server when a transport is boarded
class Player_Board_Broadcast extends rumBroadcast
{
  var = 0; // Transport ID

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var = vargv[0]; // Transport ID
    }
  }

  function OnRecv()
  {
    local ciTransport = ::rumFetchPawn( var );
    if( ciTransport != null && ( ciTransport instanceof Transport_Widget ) )
    {
      local ciPlayer = ::rumGetMainPlayer();
      ciTransport.AddPassenger( ciPlayer.GetID() );

      ciPlayer.UpdateGraphic();
    }
  }
}
