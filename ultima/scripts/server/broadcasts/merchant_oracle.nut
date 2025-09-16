// Sent to client with initial merchant info
// Received from client when player transacts with an oracle
class Merchant_Oracle_Broadcast extends rumBroadcast
{
  var1 = 0; // Transaction type
  var2 = 0; // Next hint index
  var3 = 0;
  var4 = 0;

  static s_iNumHints = 10;

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Transaction type
    }
    else if( 4 == vargv.len() )
    {
      var1 = vargv[0]; // Transaction type
      var2 = vargv[1]; // Message
      var3 = vargv[2]; // Message index
      var4 = vargv[3]; // Charge
    }
  }

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local eTransactionType = var1;
    local eNextHintIndex = var2;

    if( i_ciPlayer instanceof Player )
    {
      TransactOracleRecv( i_ciPlayer, eTransactionType, eNextHintIndex );
      i_ciPlayer.PopPacket();
    }
  }
}
