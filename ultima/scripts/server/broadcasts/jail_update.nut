// Sent to client to update player on sentence length
class Jail_Update_Broadcast extends rumBroadcast
{
  var = 0; // Notoriety

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      // Notoriety
      var = max( 0, vargv[0] );
    }
  }
}
