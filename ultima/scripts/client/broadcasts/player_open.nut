// Sent from client
class Player_Open_Broadcast extends rumBroadcast
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
