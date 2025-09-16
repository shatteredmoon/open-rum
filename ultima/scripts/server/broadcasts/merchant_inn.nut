// Sent to client with initial merchant info
// Received from client when player transacts with an inn merchant
class Merchant_Inn_Broadcast extends rumBroadcast
{
  var1 = 0; // Inn transaction type
  var2 = 0; // Room selection

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Inn Transaction type
    }
  }

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( i_ciPlayer instanceof Player )
    {
      local eTransactionType = var1;
      local iSelection = var2;
      TransactInnRecv( i_ciPlayer, eTransactionType, iSelection );

      i_ciPlayer.PopPacket();
    }
  }
}
