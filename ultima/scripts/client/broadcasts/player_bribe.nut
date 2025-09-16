// Sent from client when player bribes a guard
class Player_Bribe_Broadcast extends rumBroadcast
{
  var = 0; // Direction

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
