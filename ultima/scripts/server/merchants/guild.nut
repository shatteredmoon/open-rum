function TransactGuildStart( i_ciPlayer )
{
  local ciBroadcast = ::rumCreate( Merchant_Guild_BroadcastID, MerchantGuildTransaction.Greet );
  ::rumSendPrivate( i_ciPlayer.GetSocket(), ciBroadcast );
}


function TransactGuildRecv( i_ciPlayer, i_eTransactionType, i_ePurchaseType )
{
  // TODO - verify the player is within range of the merchant

  if( i_ePurchaseType < MerchantGuildProducts.Torches || i_ePurchaseType > MerchantGuildProducts.Sextant )
  {
    i_ciPlayer.IncrementHackAttempts();

    // Terminate the transaction
    local ciBroadcast = ::rumCreate( Merchant_Guild_BroadcastID, MerchantGuildTransaction.ServerTerminated );
    i_ciPlayer.SendBroadcast( ciBroadcast );

    return;
  }

  if( MerchantGuildTransaction.Purchase == i_eTransactionType )
  {
    local iPlayerGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
    local eVersion = i_ciPlayer.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );

    local vItemArray = null;
    if( GameType.Ultima4 == eVersion )
    {
      vItemArray = ::rumGetDataTableRow( merchant_guild_u4_DataTableID, i_ePurchaseType );
    }
    else if( GameType.Ultima3 == eVersion )
    {
      vItemArray = ::rumGetDataTableRow( merchant_guild_u3_DataTableID, i_ePurchaseType );
    }

    if( vItemArray != null )
    {
      local iPrice = vItemArray[1];
      local iAmount = vItemArray[2];
      local ePropertyID = vItemArray[5];

      if( iPrice > 0 && iAmount > 0 && iPlayerGold >= iPrice )
      {
        // Give the items to the player and deduct the cost
        i_ciPlayer.SetProperty( ePropertyID, i_ciPlayer.GetProperty( ePropertyID, 0 ) + iAmount );
        i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iPrice );

        local ciBroadcast = ::rumCreate( Merchant_Guild_BroadcastID, MerchantGuildTransaction.Purchase );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else
      {
        // Terminate the transaction
        local ciBroadcast = ::rumCreate( Merchant_Guild_BroadcastID, MerchantGuildTransaction.ServerTerminated,
                                         u4_merchant_guild_cant_pay_client_StringID );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
    }
    else
    {
      // Terminate the transaction
      local ciBroadcast = ::rumCreate( Merchant_Guild_BroadcastID, MerchantGuildTransaction.ServerTerminated );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
  }
  else
  {
    // Terminate the transaction
    local ciBroadcast = ::rumCreate( Merchant_Guild_BroadcastID, MerchantGuildTransaction.ServerTerminated );
    i_ciPlayer.SendBroadcast( ciBroadcast );
  }
}
