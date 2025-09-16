// Sent from server to print a client-side database string
class Command_Result_Broadcast extends rumBroadcast
{
  var1 = null; // String id
  var2 = null; // Optional color tag
  var3 = true; // Whether received string should be fetch from db on client

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      var1 = vargv[0]; // String id

      if( vargv.len() > 1 )
      {
        var2 = vargv[1]; // Optional color tag

        if( vargv.len() > 2 )
        {
          var3 = vargv[2];
        }
      }
    }
  }
}
