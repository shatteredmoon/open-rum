// Sent to client with initial merchant info
// Received from client when player transacts with a healer
class Merchant_Healer_Broadcast extends rumBroadcast
{
  var1 = 0; // Healer Transaction type
  var2 = 0; // Service (Cure, Heal, or Resurrect)

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Healer transaction type
    }
    else if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Healer transaction type
      var2 = vargv[1]; // Transaction result
    }
  }

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local eTransactionType = var1;
    local eService = var2;

    if( i_ciPlayer instanceof Player )
    {
      TransactHealerRecv( i_ciPlayer, eTransactionType, eService );
      i_ciPlayer.PopPacket();
    }
  }
}
