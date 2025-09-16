// Sent from client when player uses an item
class Player_Use_Broadcast extends rumBroadcast
{
  var1 = 0; // Property ID
  var2 = 0; // Direction

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      // Property ID
      var1 = vargv[0];

      if( vargv.len() > 1 )
      {
        var2 = vargv[1];
      }
    }
  }
}
