// Sent from client when player moves
class Player_Move_Broadcast extends rumBroadcast
{
  var = 0;

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      // Direction
      var = vargv[0];
    }
  }
}
