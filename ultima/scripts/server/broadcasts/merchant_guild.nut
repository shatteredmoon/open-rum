// Sent to client with initial merchant info
// Received from client when player transacts with a guild merchant
class Merchant_Guild_Broadcast extends rumBroadcast
{
  var1 = 0; // Guild Transaction Type
  var2 = 0; // Purchase type

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Guild transaction type
    }
    else if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Guild transaction type
      var2 = vargv[1]; // Reason/result
    }
  }

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( i_ciPlayer instanceof Player )
    {
      local eTransactionType = var1;
      local ePurchaseType = var2;
      TransactGuildRecv( i_ciPlayer, eTransactionType, ePurchaseType );

      i_ciPlayer.PopPacket();
    }
  }
}
