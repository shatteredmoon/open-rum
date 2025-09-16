// Sent to client with initial merchant info
// Received from client when player transacts with a reagent merchant
class Merchant_Reagents_Broadcast extends rumBroadcast
{
  var1 = 0; // Reagent transaction type
  var2 = 0; // Reagent type
  var3 = 0; // Reagent amount
  var4 = 0; // Amount paid

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Reagent transaction type
    }
  }

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local eTransactionType = var1;
    local eReagent = var2;
    local iReagentAmount = var3;
    local iPaid = var4;

    if( i_ciPlayer instanceof Player )
    {
      local iVersion = i_ciPlayer.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
      switch( iVersion )
      {
        case GameType.Ultima1:
        case GameType.Ultima2:
        case GameType.Ultima3: i_ciPlayer.IncrementHackAttempts(); break;
        case GameType.Ultima4: TransactReagentsRecv( i_ciPlayer, eTransactionType, eReagent,
                                                     iReagentAmount, iPaid ); break;
      }

      i_ciPlayer.PopPacket();
    }
  }
}
