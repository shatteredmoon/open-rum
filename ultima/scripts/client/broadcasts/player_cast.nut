// Sent from client
class Player_Cast_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;
  var3 = 0;
  var4 = 0;

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      // Spell type
      var1 = vargv[0];

      if( vargv.len() > 1 )
      {
        // Direction
        var2 = vargv[1];
      }

      if( vargv.len() > 2 )
      {
        // Special type
        var3 = vargv[2];
      }

      if( vargv.len() > 3 )
      {
        // Target ID
        var4 = vargv[3];
      }

      PlaySound( Player_Spell_SoundID );
    }
  }
}
