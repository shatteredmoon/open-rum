// Sent to client with initial merchant info
// Received from client when player transacts with a magic/spell merchant
class Merchant_Magic_Broadcast extends rumBroadcast
{
  var1 = 0; // Magic transaction type
  var2 = 0; // Spell or potion type to buy
  var3 = 0; // Amount

  constructor( ... )
  {
    base.constructor();

    if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Transport transaction type
      var2 = vargv[1]; // Response
    }
  }

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( i_ciPlayer instanceof Player )
    {
      local eTransactionType = var1;
      local eItemID = var2;
      local iAmount = var3;
      TransactMagicRecv( i_ciPlayer, eTransactionType, eItemID, iAmount );

      i_ciPlayer.PopPacket();
    }
  }
}
