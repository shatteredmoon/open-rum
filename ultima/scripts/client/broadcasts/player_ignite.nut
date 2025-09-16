// Sent from client
class Player_Ignite_Broadcast extends rumBroadcast
{
  var = 0; // Ignite or extinguish

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      // Ignite or extinguish
      var = vargv[0];
    }
  }
}
