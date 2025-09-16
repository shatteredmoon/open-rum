function TransactStableRecv( i_ciPlayer, i_eTransactionType )
{
  // Does the player already own a horse?
  local bOwnsHorse = i_ciPlayer.GetProperty( U4_Horse_PropertyID, false );
  if( bOwnsHorse )
  {
    // Client should prevent an attempt to buy multiple horses
    i_ciPlayer.IncrementHackAttempts();

    local ciBroadcast = ::rumCreate( Merchant_Stable_BroadcastID, MerchantStableTransaction.ServerTerminated );
    i_ciPlayer.SendBroadcast( ciBroadcast );
    return;
  }

  if( MerchantStableTransaction.Purchase == i_eTransactionType )
  {
    // Can player afford the horse?
    local ciMap = i_ciPlayer.GetMap();
    local iPrice = ciMap.GetProperty( Merchant_Stable_Price_PropertyID, 0 );
    local iPlayerGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
    if( iPlayerGold >= iPrice )
    {
      // Charge the player and give player a horse
      i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iPrice );
      i_ciPlayer.SetProperty( U4_Horse_PropertyID, true );

      local ciBroadcast = ::rumCreate( Merchant_Stable_BroadcastID, MerchantStableTransaction.Purchase,
                                       u4_merchant_stable_buy_ok_client_StringID );
      i_ciPlayer.SendBroadcast( ciBroadcast );

      local ciTransport = CreateTransport( U4_Horse_WidgetID, 0, ciMap, i_ciPlayer.GetPosition() );
      if( ciTransport != null )
      {
        ciTransport.Board( i_ciPlayer );
      }
    }
    else
    {
      local ciBroadcast = ::rumCreate( Merchant_Stable_BroadcastID, MerchantStableTransaction.ServerTerminated,
                                       u4_merchant_stable_cant_pay_client_StringID );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
  }
  else
  {
    // Hack attempt - invalid transaction type
    i_ciPlayer.IncrementHackAttempts();

    local ciBroadcast = ::rumCreate( Merchant_Stable_BroadcastID, MerchantStableTransaction.ServerTerminated );
    i_ciPlayer.SendBroadcast( ciBroadcast );
  }
}


function TransactStableStart( i_ciPlayer )
{
  local ciBroadcast = ::rumCreate( Merchant_Stable_BroadcastID, MerchantStableTransaction.Greet );
  ::rumSendPrivate( i_ciPlayer.GetSocket(), ciBroadcast );
}
