// Sent from client when player creates a campfire
// Received from server on error
class Player_Camp_Broadcast extends rumBroadcast
{
  var = 0;

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var = vargv[0]; // Direction enum
    }
  }

  function OnRecv()
  {
    // An error message was received that needs formatting
    local strError = ::rumGetString( var );
    strError = format( strError, g_iTorchesForCampfire );
    ShowString( strError, g_strColorTagArray.Red );
  }
}
