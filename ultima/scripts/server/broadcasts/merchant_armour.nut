// Sent to client with initial merchant info
// Received from client when player attempts to buy or sell armour
class Merchant_Armour_Broadcast extends rumBroadcast
{
  var1 = 0; // Armour transaction type
  var2 = 0; // Armour id to buy or sell

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Armour transaction type
    }
    else if( 2 == vargv.len() )
    {
      // Transaction received
      var1 = vargv[0]; // Armour transaction type
      var2 = vargv[1]; // Result
    }
  }

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local eTransaction = var1;
    local varArmour = var2;

    if( i_ciPlayer instanceof Player )
    {
      TransactArmourRecv( i_ciPlayer, eTransaction, varArmour );
      i_ciPlayer.PopPacket();
    }
  }
}
