function TransactOracleStart( i_ciPlayer )
{
  local iSocket = i_ciPlayer.GetSocket();

  local ciBroadcast = ::rumCreate( Merchant_Oracle_BroadcastID, MerchantOracleTransaction.Greet );
  ::rumSendPrivate( iSocket, ciBroadcast );

  // Start with the free hint
  local iHintIndex = i_ciPlayer.GetProperty( U3_Oracle_Index_PropertyID, 0 );
  local strDesc = ::rumGetString( u3_oracle_0_server_StringID, i_ciPlayer.m_iLanguageID );

  ciBroadcast = ::rumCreate( Merchant_Oracle_BroadcastID, MerchantOracleTransaction.Hint, strDesc, 0,
                             0 == iHintIndex );
  ::rumSendPrivate( iSocket, ciBroadcast );
}


function TransactOracleRecv( i_ciPlayer, i_eTransactionType, i_eNextHintIndex )
{
  if( MerchantOracleTransaction.RequestNext == i_eTransactionType )
  {
    if( i_eNextHintIndex >= Merchant_Oracle_Broadcast.s_iNumHints )
    {
      i_ciPlayer.IncrementHackAttempts();
      return;
    }

    local strDesc = ::rumGetStringByName( format( "u3_oracle_%d_server_StringID", i_eNextHintIndex ),
                                          i_ciPlayer.m_iLanguageID );

    local iHintIndex = i_ciPlayer.GetProperty( U3_Oracle_Index_PropertyID, 0 );
    if( iHintIndex >= i_eNextHintIndex )
    {
      local ciBroadcast = ::rumCreate( Merchant_Oracle_BroadcastID, MerchantOracleTransaction.Hint, strDesc,
                                       i_eNextHintIndex, iHintIndex <= i_eNextHintIndex );
      ::rumSendPrivate( i_ciPlayer.GetSocket(), ciBroadcast );
    }
    else
    {
      local iCost = i_eNextHintIndex * 100;

      // Does player have the gold?
      local iPlayerGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
      if( iPlayerGold >= iCost )
      {
        // Deduct gold
        i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iCost );

        // Send down the purchased hint
        local ciBroadcast = ::rumCreate( Merchant_Oracle_BroadcastID, MerchantOracleTransaction.Hint, strDesc,
                                         i_eNextHintIndex, true );
        ::rumSendPrivate( i_ciPlayer.GetSocket(), ciBroadcast );

        // Remember that the player unlocked this hint level
        i_ciPlayer.SetProperty( U3_Oracle_Index_PropertyID, i_eNextHintIndex );
      }
      else
      {
        // Player can't pay (should have been caught by the client)
        local ciBroadcast = ::rumCreate( Merchant_Tavern_BroadcastID, MerchantTavernTransaction.ServerTerminated,
                                         u3_merchant_oracle_cant_pay_client_StringID );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
    }
  }
}
