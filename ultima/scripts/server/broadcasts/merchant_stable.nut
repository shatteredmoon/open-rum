// Sent to client with initial merchant info
// Received from client when player transacts with a stable merchant
class Merchant_Stable_Broadcast extends rumBroadcast
{
  var1 = 0; // Stable transaction type
  var2 = 0;

  constructor( ... )
  {
    base.constructor();

    if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Stable transaction type
      var2 = vargv[1]; // Response
    }
  }

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local eTransactionType = var1;

    if( i_ciPlayer instanceof Player )
    {
      TransactStableRecv( i_ciPlayer, eTransactionType );
      i_ciPlayer.PopPacket();
    }
  }
}
