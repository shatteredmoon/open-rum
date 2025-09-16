// Sent to client with initial merchant info
// Received from client when player transacts with a tavern merchant
class Merchant_Tavern_Broadcast extends rumBroadcast
{
  var1 = 0; // Tavern transaction type
  var2 = 0; // Payment amount
  var3 = 0;

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Tavern transaction type
    }
    else if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Tavern transaction type
      var2 = vargv[1]; // Reason/result
    }
    else if( 3 == vargv.len() )
    {
      var1 = vargv[0]; // Tavern transaction type
      var2 = vargv[1]; // Reason/result
      var3 = vargv[2]; // Response
    }
  }

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local eTransactionType = var1;

    if( i_ciPlayer instanceof Player )
    {
      TransactTavernRecv( i_ciPlayer, eTransactionType, var2 );
      i_ciPlayer.PopPacket();
    }
  }
}
