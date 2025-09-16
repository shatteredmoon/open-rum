function TransactMagicStart( i_ciPlayer )
{
  local ciBroadcast = ::rumCreate( Merchant_Magic_BroadcastID, MerchantMagicTransaction.Greet );
  ::rumSendPrivate( i_ciPlayer.GetSocket(), ciBroadcast );
}


function TransactMagicRecv( i_ciPlayer, i_eTransactionType, i_eItemID, i_iAmount )
{
  if( MerchantMagicTransaction.Purchase != i_eTransactionType || ( 0 == i_iAmount ) )
  {
    // Why was the message even sent?
    i_ciPlayer.IncrementHackAttempts();
    return;
  }

  local eInventoryArray = null;

  // Determine the data table row
  local eMapArray = ::rumGetDataTableColumn( merchant_magic_DataTableID, 0 );
  local ciMap = i_ciPlayer.GetMap();
  local iRow = eMapArray.find( ciMap.GetAssetID() );
  if( iRow != null )
  {
    // Fetch the row and slice out the spell data
    eInventoryArray = ::rumGetDataTableRow( merchant_magic_DataTableID, iRow ).slice( 3, 9 );
  }

  if( ( U1_Potions_PropertyID == i_eItemID ) || ( U2_Potions_PropertyID == i_eItemID ) )
  {
    // Handle Potions
    local fDiscount = i_ciPlayer.GetDiscountPercent();
    local iPrice = ::rumGetDataTableRow( merchant_magic_DataTableID, iRow )[2] * i_iAmount;

    // Verify gold
    local iPlayerGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
    if( iPlayerGold < iPrice )
    {
      local ciBroadcast = ::rumCreate( Merchant_Magic_BroadcastID, MerchantMagicTransaction.Purchase,
                                       merchant_magic_cant_pay_client_StringID );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
    else
    {
      // Give the potion amount to player
      i_ciPlayer.AdjustVersionedProperty( g_ePotionsPropertyVersionArray, i_iAmount );

      // Deduct gold from the player
      i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iPrice );

      // Notify client
      local ciBroadcast = ::rumCreate( Merchant_Magic_BroadcastID, MerchantMagicTransaction.Purchase,
                                       merchant_magic_buy_yes_client_StringID );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
  }
  else
  {
    // Handle spells
    local iMaxItems = eInventoryArray.len();
    local iIndex = eInventoryArray.find( rumInvalidAssetID );
    local iNumAvailable = ( null == iIndex ) ? iMaxItems : iIndex;

    // Does the merchant sell this spell?
    if( ValueInContainer( i_eItemID, eInventoryArray ) )
    {
      local fDiscount = i_ciPlayer.GetDiscountPercent();
      local iPrice = 0;

      local ciSpell = ::rumGetCustomAsset( i_eItemID );
      if( ciSpell != null )
      {
        // What's the going rate for the spell?
        iPrice = ciSpell.GetProperty( Merchant_Price_PropertyID, 0 ) * i_iAmount;
        iPrice = iPrice - ( iPrice * fDiscount ).tointeger();
        iPrice = max( iPrice, 1 );
      }

      if( iPrice <= 0 )
      {
        // Spell price not found
        i_ciPlayer.IncrementHackAttempts();

        local ciBroadcast = ::rumCreate( Merchant_Magic_BroadcastID, MerchantMagicTransaction.ServerTerminated );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else
      {
        // Verify gold
        local iPlayerGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
        if( iPlayerGold < iPrice )
        {
          local ciBroadcast = ::rumCreate( Merchant_Magic_BroadcastID, MerchantMagicTransaction.Purchase,
                                           merchant_magic_cant_pay_client_StringID );
          i_ciPlayer.SendBroadcast( ciBroadcast );
        }
        else
        {
          // Give spell amount to player
          local ePropertyID = ciSpell.GetProperty( Spell_ID_PropertyID, rumInvalidAssetID );
          if( ePropertyID != rumInvalidAssetID )
          {
            local iCurrentAmount = i_ciPlayer.GetProperty( ePropertyID, 0 );
            i_ciPlayer.SetProperty( ePropertyID, iCurrentAmount + i_iAmount );

            // Deduct gold from the player
            i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iPrice );

            // Notify client
            local ciBroadcast = ::rumCreate( Merchant_Magic_BroadcastID, MerchantMagicTransaction.Purchase,
                                             merchant_magic_buy_yes_client_StringID );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
        }
      }
    }
    else
    {
      // Hack attempt - item not sold by this merchant
      i_ciPlayer.IncrementHackAttempts();

      local ciBroadcast = ::rumCreate( Merchant_Magic_BroadcastID, MerchantMagicTransaction.ServerTerminated );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
  }
}


function TransactMagicSteal( i_ciCreature, i_ciPlayer )
{
  local ciMap = i_ciPlayer.GetMap();

  local bSuccess = i_ciPlayer.AttemptSteal( i_ciCreature, ciMap.s_cMagicStealModifier );
  if( bSuccess )
  {
    local eInventoryArray = null;

    // TODO - handle potion stealing

    // Determine the data table row
    local eMapArray = ::rumGetDataTableColumn( merchant_magic_DataTableID, 0 );
    local iRow = eMapArray.find( ciMap.GetAssetID() );
    if( iRow != null )
    {
      // Fetch the row and slice out the spell data
      eInventoryArray = ::rumGetDataTableRow( merchant_magic_DataTableID, iRow ).slice( 3, 9 );
    }

    if( null == eInventoryArray )
    {
      return;
    }

    // Merchants can sell up to 6 items
    local iMaxItems = 6;
    local iIndex = eInventoryArray.find( rumInvalidAssetID );
    local iNumAvailable = ( null == iIndex ) ? iMaxItems : iIndex;

    // Random spell index
    local eItemID = eInventoryArray[rand%iNumAvailable];
    local ciSpell = ::rumGetCustomAsset( eItemID );
    if( ciSpell != null )
    {
      // Add spell to player
      local ePropertyID = ciSpell.GetProperty( Spell_ID_PropertyID, rumInvalidAssetID );
      if( ePropertyID != rumInvalidAssetID )
      {
        local iCurrentAmount = i_ciPlayer.GetProperty( ePropertyID, 0 );
        i_ciPlayer.SetProperty( ePropertyID, iCurrentAmount + 1 );

        local ciBroadcast = ::rumCreate( Player_Steal_BroadcastID, ePropertyID );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
    }
  }
}
