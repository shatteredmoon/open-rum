// Server
class TimeGate_Period_Broadcast extends rumBroadcast
{
  var = 0;

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      // Period type
      var = vargv[0];
      print( "Sending timegate period " + var + "\n" );
    }
  }
}