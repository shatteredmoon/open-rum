// Sent to client with initial merchant info
// Received from client when player attempts to buy or sell a weapon
class Merchant_Weapons_Broadcast extends rumBroadcast
{
  var1 = 0; // Weapon transaction type
  var2 = 0; // Weapon id to buy or sell
  var3 = 0; // Quantity


  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Weapon transaction type
    }
    else if( 2 == vargv.len() )
    {
      // Transaction received
      var1 = vargv[0]; // Weapon transaction type
      var2 = vargv[1]; // Result
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local eTransaction = var1;
    local varWeapon = var2;
    local iQuantity = var3;

    if( i_ciPlayer instanceof Player )
    {
      TransactWeaponRecv( i_ciPlayer, eTransaction, varWeapon, iQuantity );
      i_ciPlayer.PopPacket();
    }
  }
}
