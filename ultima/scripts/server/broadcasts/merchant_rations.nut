// Sent to client with initial merchant info
// Received from client when player transacts with a rations merchant
class Merchant_Rations_Broadcast extends rumBroadcast
{
  var1 = 0; // Ration transaction type
  var2 = 0; // Num packs


  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Ration transaction type
    }
    if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Ration transaction type
      var2 = vargv[1]; // Reason/Description
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local eTransactionType = var1;
    local iNumPacks = var2;

    if( i_ciPlayer instanceof Player )
    {
      TransactRationsRecv( i_ciPlayer, eTransactionType, iNumPacks );
      i_ciPlayer.PopPacket();
    }
  }
}
