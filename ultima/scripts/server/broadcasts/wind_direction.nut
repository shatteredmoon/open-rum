// Server
class Wind_Direction_Broadcast extends rumBroadcast
{
  var = 0; // Direction

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() == 1 )
    {
      // Direction
      var = vargv[0];
    }
  }
}
