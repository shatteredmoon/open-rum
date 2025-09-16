// Sent to client to apply attack updates
class Attack_Update_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;
  var3 = 0;
  var4 = 0;

  constructor( ... )
  {
    base.constructor();

    if( 4 == vargv.len() )
    {
      var1 = vargv[0]; // Attacker unique id
      var2 = vargv[1]; // Target unique id
      var3 = vargv[2]; // Weapon type
      var4 = vargv[3]; // Attack result
    }
  }
}
