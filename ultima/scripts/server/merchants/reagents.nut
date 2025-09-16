function TransactReagentsRecv( i_ciPlayer, i_eTransactionType, i_eReagent, iReagentAmount, i_iPaid )
{
  if( MerchantReagentTransaction.Purchase != i_eTransactionType )
  {
    // Why was the message even sent?
    i_ciPlayer.IncrementHackAttempts();
    return;
  }

  local bSuccess = false;

  if( i_eReagent >= Reagents.Sulphurous_Ash && i_eReagent < Reagents.NumReagents )
  {
    local ciMap = i_ciPlayer.GetMap();

    // Determine cost
    local iPrice = 0;
    local iMapArray = ::rumGetDataTableColumn( merchant_reagents_DataTableID, 0 );
    local iRow = iMapArray.find( ciMap.GetAssetID() );
    if( iRow != null )
    {
      // Fetch the row and slice off the map column
      local uiPriceArray = ::rumGetDataTableRow( merchant_reagents_DataTableID, iRow ).slice( 1 );
      iPrice = uiPriceArray[i_eReagent] * iReagentAmount;
    }

    if( iPrice > 0 && i_iPaid > 0 )
    {
      // Does the player have enough gold?
      local iPlayerGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
      if( iPlayerGold < i_iPaid )
      {
        i_iPaid = iPlayerGold;
      }

      if( i_iPaid < iPrice )
      {
        // Deduct virtue for cheating the blind woman
        i_ciPlayer.AffectVirtue( VirtueType.Honesty, -1, true, true );
        i_ciPlayer.AffectVirtue( VirtueType.Justice, -1, true, false );
        i_ciPlayer.AffectVirtue( VirtueType.Honor, -1, true, false );
      }
      else
      {
        // Add virtue for not cheating the blind woman
        i_ciPlayer.AffectVirtue( VirtueType.Honesty, 1, false, true );
      }

      // Give the player the reagents and deduct gold
      local ePropertyID = g_eU4ReagentPropertyArray[i_eReagent];
      i_ciPlayer.SetProperty( ePropertyID, i_ciPlayer.GetProperty( ePropertyID, 0 ) + iReagentAmount );
      i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -i_iPaid );

      local ciBroadcast = ::rumCreate( Merchant_Reagents_BroadcastID, MerchantReagentTransaction.Purchase );
      i_ciPlayer.SendBroadcast( ciBroadcast );

      bSuccess = true;
    }
  }

  if( !bSuccess )
  {
    iPlayer.IncrementHackAttempts();

    // Terminate the transaction
    local ciBroadcast = ::rumCreate( Merchant_Reagents_BroadcastID, MerchantReagentTransaction.ServerTerminated );
    i_ciPlayer.SendBroadcast( ciBroadcast );
  }
}


function TransactReagentsStart( i_ciPlayer )
{
  local ciBroadcast = ::rumCreate( Merchant_Reagents_BroadcastID, MerchantReagentTransaction.Greet );
  ::rumSendPrivate( i_ciPlayer.GetSocket(), ciBroadcast );
}
