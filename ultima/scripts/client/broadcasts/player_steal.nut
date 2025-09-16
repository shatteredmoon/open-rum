// Received from client when player attempts to talk steal something
class Player_Steal_Broadcast extends rumBroadcast
{
  var1 = 0; // Direction or Class/Property
  var2 = 0; // Amount

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Direction or Class/Property
    }
    else if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Class or property
      var2 = vargv[1]; // Amount
    }
  }


  function OnRecv()
  {
    local strFind = ::rumGetString( msg_steal_client_StringID );

    if( var2 > 0 )
    {
      local strItem = format( "%d %s", var2, ::rumGetString( var1.s_strDescription ) );
      strFind = format( strFind, strItem );
    }
    else
    {
      local strItem = format( "%s", ::rumGetString( var1.s_strDescription ) );
      strFind = format( strFind, strItem );
    }

    ShowString( strFind, g_strColorTagArray.Magenta );
  }
}
