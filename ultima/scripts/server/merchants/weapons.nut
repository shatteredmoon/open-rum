function TransactWeaponRecv( i_ciPlayer, i_eTransactionType, i_vValue, i_iQuantity )
{
  local bError = false;

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

  if( MerchantWeaponryTransaction.Purchase == i_eTransactionType )
  {
    local eWeaponArray = null;

    // Determine the data table row
    local eMapArray = ::rumGetDataTableColumn( merchant_weapons_DataTableID, 0 );
    local iRow = eMapArray.find( ciMap.GetAssetID() );
    if( iRow != null )
    {
      // Fetch the row and slice off the map column
      eWeaponArray = ::rumGetDataTableRow( merchant_weapons_DataTableID, iRow ).slice( 1 );
    }

    local eWeaponType = i_vValue;

    // Does the merchant sell this weapon?
    if( ValueInContainer( eWeaponType, eWeaponArray ) )
    {
      local ciAsset = ::rumGetInventoryAsset( eWeaponType );
      if( ciAsset != null )
      {
        if( ciAsset.GetProperty( Inventory_Stacks_PropertyID, false ) )
        {
          // The item stacks, so determine how many the player can actually carry
          local ciItem = i_ciPlayer.GetInventoryByType( eWeaponType );
          if( ciItem != null )
          {
            local iNumOwned = ciItem.GetProperty( Inventory_Quantity_PropertyID, 1 );
            local ciProperty = ::rumGetPropertyAsset( Inventory_Quantity_PropertyID );
            i_iQuantity = min( i_iQuantity, ciProperty.GetMaxValue() - iNumOwned );
          }
        }

        // What's the going rate for the weapon?
        local iPrice = ciAsset.GetProperty( Inventory_Price_PropertyID, 0 ) * i_iQuantity;
        if( iPrice > 0 )
        {
          // Apply discount
          iPrice -= ( iPrice * fDiscount ).tointeger();

          // Verify gold
          local iPlayerGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
          if( iPlayerGold < iPrice )
          {
            local ciBroadcast = ::rumCreate( Merchant_Weapons_BroadcastID, MerchantWeaponryTransaction.Purchase,
                                             u4_merchant_weapon_cant_pay_client_StringID );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
          else
          {
            local bAdded = i_ciPlayer.AddOrCreateItem( eWeaponType, i_iQuantity );
            if( bAdded )
            {
              // Deduct gold from the player
              i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iPrice );

              // Notify client
              local ciBroadcast = ::rumCreate( Merchant_Weapons_BroadcastID, MerchantWeaponryTransaction.Purchase,
                                               u4_merchant_weapon_buy_yes_client_StringID );
              i_ciPlayer.SendBroadcast( ciBroadcast );
            }
            else
            {
              // Failed to add/create weapon(s)
              bError = true;
            }
          }
        }
        else
        {
          // Invalid price or quantity
          i_ciPlayer.IncrementHackAttempts();
          bError = true;
        }
      }
      else
      {
        // Weapon doesn't exist
        i_ciPlayer.IncrementHackAttempts();
        bError = true;
      }
    }
    else
    {
      // Weapon not sold by this merchant
      i_ciPlayer.IncrementHackAttempts();
      bError = true;
    }
  }
  else if( MerchantWeaponryTransaction.Sell == i_eTransactionType )
  {
    local iWeaponID = i_vValue;
    local ciWeapon = i_ciPlayer.GetInventory( iWeaponID );
    if( ciWeapon != null )
    {
      local iNewAmount = 0;
      if( ciWeapon.GetProperty( Inventory_Stacks_PropertyID, false ) )
      {
        // The item stacks, does the player have the quantity provided?
        local iNumOwned = ciWeapon.GetProperty( Inventory_Quantity_PropertyID, 0 );
        i_iQuantity = min( iNumOwned, i_iQuantity );
        iNewAmount = iNumOwned - i_iQuantity;
      }

      local eInventoryType = ciWeapon.GetProperty( Inventory_Type_PropertyID, InventoryType.Standard );
      if( ( InventoryType.Weapon == eInventoryType ) && ( ciWeapon instanceof rumInventory ) )
      {
        local iPrice = ciWeapon.GetProperty( Inventory_Price_PropertyID, 0 ) * i_iQuantity;
        if( iPrice > 0 )
        {
          local bSold = false;

          // Remove the reference to the weapon instance and delete it from the player inventory
          if( iNewAmount > 0 )
          {
            ciWeapon.SetProperty( Inventory_Quantity_PropertyID, iNewAmount );
            bSold = true;
          }
          else if( i_ciPlayer.DeleteInventory( ciWeapon ) )
          {
            bSold = true;
          }

          if( bSold )
          {
            // Pay the player for the weapon
            local iValue = ( iPrice / 2.0 - ( 2.0 * fDiscount ) ).tointeger();
            iValue = max( iValue, 0 );
            i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, iValue );

            // Notify client
            local ciBroadcast = ::rumCreate( Merchant_Weapons_BroadcastID, MerchantWeaponryTransaction.Sell );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
          else
          {
            bError = true;
          }
        }
        else
        {
          // Invalid price or quantity
          i_ciPlayer.IncrementHackAttempts();
          bError = true;
        }
      }
      else
      {
        // Attempt to sell non-weapon
        i_ciPlayer.IncrementHackAttempts();
        bError = true;
      }
    }
    else
    {
      // Attempt to sell a weapon that isn't owned by the player
      i_ciPlayer.IncrementHackAttempts();
      bError = true;
    }
  }
  else
  {
    // Invalid transaction type
    i_ciPlayer.IncrementHackAttempts();
    bError = true;
  }

  if( bError )
  {
    local ciBroadcast = ::rumCreate( Merchant_Weapons_BroadcastID, MerchantWeaponryTransaction.ServerTerminated );
    i_ciPlayer.SendBroadcast( ciBroadcast );
  }
}


function TransactWeaponStart( i_ciPlayer )
{
  local ciBroadcast = ::rumCreate( Merchant_Weapons_BroadcastID, MerchantWeaponryTransaction.Greet );
  ::rumSendPrivate( i_ciPlayer.GetSocket(), ciBroadcast );
}


function TransactWeaponSteal( i_ciCreature, i_ciPlayer )
{
  local iModifier = 40;
  local bSuccess = i_ciPlayer.AttemptSteal( i_ciCreature, iModifier );
  if( bSuccess )
  {
    local eWeaponArray = null;

    // Determine the data table row
    local eMapArray = ::rumGetDataTableColumn( merchant_weapons_DataTableID, 0 );
    local iRow = eMapArray.find( ciMap.GetAssetID() );
    if( iRow != null )
    {
      // Fetch the row and slice off the map column
      eWeaponArray = ::rumGetDataTableRow( merchant_weapons_DataTableID, iRow ).slice( 1 );
    }

    local iIndex = eWeaponArray.find( rumInvalidAssetID );
    local iNumAvailable = ( null == iIndex ) ? iMaxItems : iIndex;

    local eWeaponType = rumInvalidAssetID;
    local bDone = false;

    // Basically, the player takes a skill roll for each element in the array, with weapon value increasing the
    // further into the array the player gets. Any time a skill roll fails, walking the array is done and the
    // player is given the weapon at the achieved index.
    for( local i = 0; i < iNumAvailable && !bDone; ++i )
    {
      eArmourType = eWeaponArray[i];
      if( !i_ciPlayer.SkillRoll( i_ciPlayer.GetDexterity() ) )
      {
        bDone = true;
      }
    }

    // Add weapon to player
    if( i_ciPlayer.AddOrCreateItem( eWeaponType ) )
    {
      local ciBroadcast = ::rumCreate( Player_Steal_BroadcastID, eWeaponType );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
    else
    {
      // Failed to create weapon
      local ciBroadcast = ::rumCreate( Merchant_Weapons_BroadcastID, MerchantWeaponryTransaction.ServerTerminated );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
  }
}
