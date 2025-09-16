// Sent to client with initial merchant info
// Received from client when player transacts with a transport merchant
class Merchant_Transport_Broadcast extends rumBroadcast
{
  var1 = 0; // Transport transaction type
  var2 = 0; // Transport ID


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      var1 = vargv[0]; // Transport transaction type
      if( vargv.len() > 1 )
      {
        var2 = vargv[1]; // Response
      }
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local eTransactionType = var1;
    local eTransportID = var2;

    if( i_ciPlayer instanceof Player )
    {
      TransactTransportRecv( i_ciPlayer, eTransactionType, eTransportID );
      i_ciPlayer.PopPacket();
    }
  }
}
