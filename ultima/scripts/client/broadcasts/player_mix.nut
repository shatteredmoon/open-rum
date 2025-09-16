// Sent from client
class Player_Mix_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;
  var3 = 0;

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() == 3 )
    {
      // Spell type
      var1 = vargv[0];

      // Reagents bitflags
      var2 = vargv[1];

      // Amount to mix
      var3 = vargv[2];
    }
  }
}
