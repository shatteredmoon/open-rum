// Received from client when a new character has finished the gypsy introduction
// Sent from server with character creation results
class Player_Initialize_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;
  var3 = 0;


  constructor( ... )
  {
    base.constructor();

    if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Initialization result
      var2 = vargv[1]; // Character name
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local strPlayerName = var1;
    local iGender = var2;
    local uiCardArray = var3;

    if( null == i_ciPlayer )
    {
      // Login the player, but don't attempt a database restore because the player hasn't been created yet!
      i_ciPlayer = ::rumPlayerLogin( i_iSocket, strPlayerName, /* i_bRestoreDB */ false );
    }

    if( i_ciPlayer != null && ( i_ciPlayer instanceof Player ) )
    {
      if( !i_ciPlayer.Initialize( iGender, uiCardArray ) )
      {
        local ciBroadcast = ::rumCreate( Player_Initialize_BroadcastID, false, null );
        i_ciPlayer.SendBroadcast( ciBroadcast );

        // Always free the reference to the player before logging the player out!
        i_ciPlayer = null;

        ::rumPlayerLogout( i_iSocket );
      }

      i_ciPlayer.PopPacket();
    }
    else
    {
      i_ciPlayer = null;
      ::rumPlayerLogout( i_iSocket );
    }
  }
}
