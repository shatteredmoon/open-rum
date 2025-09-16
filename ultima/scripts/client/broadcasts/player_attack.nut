// Sent from client
class Player_Attack_Broadcast extends rumBroadcast
{
  var1 = 0; // Attack type
  var2 = 0; // Direction or target attack type

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() >= 2 )
    {
      var1 = vargv[0]; // Attack Type
      var2 = vargv[1]; // Direction or targeted attack type
    }
  }
}
