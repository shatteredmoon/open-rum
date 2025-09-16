// Server
class Moon_Phases_Broadcast extends rumBroadcast
{
  var1 = 0; // Trammel phase
  var2 = 0; // Felucca phase

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() == 2 )
    {
      // Trammel phase
      var1 = vargv[0];

      // Felucca phase
      var2 = vargv[1];
    }
  }
}
