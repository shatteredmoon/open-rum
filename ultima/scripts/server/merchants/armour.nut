function TransactArmourRecv( i_ciPlayer, i_eTransactionType, i_vValue )
{
  local fDiscount = 0.0;
  local eVersion = i_ciPlayer.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
  switch( eVersion )
  {
    case GameType.Ultima1:
    case GameType.Ultima2:
      fDiscount = i_ciPlayer.GetDiscountPercent();
      break;
  }

  local ciMap = i_ciPlayer.GetMap();

  if( MerchantArmouryTransaction.Purchase == i_eTransactionType )
  {
    local eArmourArray = null;

    // Determine the data table row
    local eMapArray = ::rumGetDataTableColumn( merchant_armour_DataTableID, 0 );
    local iRow = eMapArray.find( ciMap.GetAssetID() );
    if( iRow != null )
    {
      // Fetch the row and slice off the map column
      eArmourArray = ::rumGetDataTableRow( merchant_armour_DataTableID, iRow ).slice( 1 );
    }

    local eArmourType = i_vValue;

    // Does the merchant sell this armour?
    if( ValueInContainer( eArmourType, eArmourArray ) )
    {
      local ciAsset = ::rumGetInventoryAsset( eArmourType );
      if( null == ciAsset )
      {
        // Armour not found
        i_ciPlayer.IncrementHackAttempts();

        local ciBroadcast = ::rumCreate( Merchant_Armour_BroadcastID,
                                         MerchantArmouryTransaction.ServerTerminated );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else
      {
        // What's the going rate for the armour?
        local iPrice = ciAsset.GetProperty( Inventory_Price_PropertyID, 0 );
        if( iPrice <= 0 )
        {
          // Something wrong with price
          i_ciPlayer.IncrementHackAttempts();

          local ciBroadcast = ::rumCreate( Merchant_Armour_BroadcastID,
                                           MerchantArmouryTransaction.ServerTerminated );
          i_ciPlayer.SendBroadcast( ciBroadcast );
        }
        else
        {
          // Apply discount
          iPrice -= ( iPrice * fDiscount ).tointeger();

          // Verify gold
          local iPlayerGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
          if( iPlayerGold < iPrice )
          {
            local ciBroadcast = ::rumCreate( Merchant_Armour_BroadcastID, MerchantArmouryTransaction.Purchase,
                                             u4_merchant_armour_cant_pay_client_StringID );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
          else
          {
            // Add armour to player
            if( i_ciPlayer.AddOrCreateItem( eArmourType ) )
            {
              // Deduct gold from the player
              i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iPrice );

              // Notify client
              local ciBroadcast = ::rumCreate( Merchant_Armour_BroadcastID, MerchantArmouryTransaction.Purchase,
                                               u4_merchant_armour_buy_yes_client_StringID );
              i_ciPlayer.SendBroadcast( ciBroadcast );
            }
            else
            {
              // Failed to create armour
              local ciBroadcast = ::rumCreate( Merchant_Armour_BroadcastID,
                                               MerchantArmouryTransaction.ServerTerminated );
              i_ciPlayer.SendBroadcast( ciBroadcast );
            }
          }
        }
      }
    }
    else
    {
      // Hack attempt - armour not sold by this merchant
      i_ciPlayer.IncrementHackAttempts();

      local ciBroadcast = ::rumCreate( Merchant_Armour_BroadcastID, MerchantArmouryTransaction.ServerTerminated );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
  }
  else if( MerchantArmouryTransaction.Sell == i_eTransactionType )
  {
    local iArmourID = i_vValue;
    local ciArmour = i_ciPlayer.GetInventory( iArmourID );
    if( null == ciArmour )
    {
      // Hack attempt - attempt to sell armour that isn't owned by the player
      i_ciPlayer.IncrementHackAttempts();

      local ciBroadcast = ::rumCreate( Merchant_Armour_BroadcastID, MerchantArmouryTransaction.ServerTerminated );
      i_ciPlayer.SendBroadcast( ciBroadcast );
      return;
    }

    local eInventoryType = ciArmour.GetProperty( Inventory_Type_PropertyID, InventoryType.Standard );
    if( ( InventoryType.Armour == eInventoryType ) && ( ciArmour instanceof rumInventory ) )
    {
      local iPrice = ciArmour.GetProperty( Inventory_Price_PropertyID, 0 );

      // Remove the reference to the armour instance and delete it from the player inventory
      if( i_ciPlayer.DeleteInventory( ciArmour ) )
      {
        // Pay the player for the armour
        local iValue = ( iPrice / 2.0 - ( 2.0 * fDiscount ) ).tointeger();
        iValue = max( iValue, 0 );
        i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, iValue );
      }

      // Notify client
      local ciBroadcast = ::rumCreate( Merchant_Armour_BroadcastID, MerchantArmouryTransaction.Sell );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
    else
    {
      // Hack attempt - attempt to sell non-armour
      i_ciPlayer.IncrementHackAttempts();

      local ciBroadcast = ::rumCreate( Merchant_Armour_BroadcastID, MerchantArmouryTransaction.ServerTerminated );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
  }
  else
  {
    // Hack attempt - invalid transaction type
    i_ciPlayer.IncrementHackAttempts();

    local ciBroadcast = ::rumCreate( Merchant_Armour_BroadcastID, MerchantArmouryTransaction.ServerTerminated );
    i_ciPlayer.SendBroadcast( ciBroadcast );
  }
}


function TransactArmourStart( i_ciPlayer )
{
  local ciBroadcast = ::rumCreate( Merchant_Armour_BroadcastID, MerchantArmouryTransaction.Greet );
  ::rumSendPrivate( i_ciPlayer.GetSocket(), ciBroadcast );
}


function TransactArmourSteal( i_ciCreature, i_ciPlayer )
{
  local iModifier = 52;
  local bSuccess = i_ciPlayer.AttemptSteal( i_ciCreature, iModifier );
  if( bSuccess )
  {
    local eArmourArray = null;

    // Determine the data table row
    local eMapArray = ::rumGetDataTableColumn( merchant_armour_DataTableID, 0 );
    local iRow = eMapArray.find( ciMap.GetAssetID() );
    if( iRow != null )
    {
      // Fetch the row and slice off the map column
      eArmourArray = ::rumGetDataTableRow( merchant_armour_DataTableID, iRow ).slice( 1 );
    }

    local iIndex = eArmourArray.find( rumInvalidAssetID );
    local iNumAvailable = ( null == iIndex ) ? iMaxItems : iIndex;

    local eArmourType = rumInvalidAssetID;
    local bDone = false;

    // Basically, the player takes a skill roll for each element in the array, with armour value increasing the
    // further into the array the player gets. Any time a skill roll fails, walking the array is done and the
    // player is given the armour at the achieved index.
    for( local i = 0; i < iNumAvailable && !bDone; ++i )
    {
      eArmourType = eArmourArray[i];
      if( !i_ciPlayer.SkillRoll( i_ciPlayer.GetDexterity() ) )
      {
        bDone = true;
      }
    }

    // Add armour to player
    if( i_ciPlayer.AddOrCreateItem( eArmourType ) )
    {
      local ciBroadcast = ::rumCreate( Player_Steal_BroadcastID, eArmourType );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
    else
    {
      // Failed to create armour
      local ciBroadcast = ::rumCreate( Merchant_Armour_BroadcastID, MerchantArmouryTransaction.ServerTerminated );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
  }
}
