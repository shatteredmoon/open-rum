// Sent from server to client when a property modification should be displayed
class Change_PropertyID_Broadcast extends rumBroadcast
{
  var1 = null; // Property class
  var2 = null; // Integer change
  var3 = g_strColorTagArray.Green; // Optional color tag

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      var1 = vargv[0]; // Property class

      if( vargv.len() > 1 )
      {
        var2 = vargv[1]; // Integer change

        if( vargv.len() > 2 )
        {
          var3 = vargv[2]; // Optional color tag
        }
      }
    }
  }
}
