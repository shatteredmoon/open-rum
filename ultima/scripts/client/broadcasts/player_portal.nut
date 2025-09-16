// Sent from client
class Player_Portal_Broadcast extends rumBroadcast
{
  var = 0;

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      // Method type
      var = vargv[0];
    }
  }
}
