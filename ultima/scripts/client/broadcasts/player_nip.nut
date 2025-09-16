// Sent from client
class Player_Nip_Broadcast extends rumBroadcast
{
  var1 = 0; // On Self or on another creature
  var2 = 0; // PropertyID
  var3 = 0; // Direction

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      // Self targeted (true) or on another creature (false)?
      var1 = vargv[0];
      var2 = vargv[1];

      if( vargv.len() > 2 )
      {
        // Direction
        var3 = vargv[2];
      }
    }
  }
}
