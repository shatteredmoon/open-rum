// Sent to the client any time a creature is killed
class Creature_Killed_Broadcast extends rumBroadcast
{
  var1 = 0; // Creature type
  var2 = 0; // XP reward

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() == 2 )
    {
      // Creature type
      var1 = vargv[0];

      // XP reward
      var2 = vargv[1];
    }
  }
}
