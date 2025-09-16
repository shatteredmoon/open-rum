class U1_Pond_Widget extends U1_Widget
{
  function Use( i_ciPlayer, i_eCoinType )
  {
    if( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() )
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      return;
    }

    local iCoin = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );

    // This greatly differs from actual U1
    if( ( U1_CoinType.Copper == i_eCoinType ) && iCoin > 0 )
    {
      i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -1 );

      // 20% chance of getting nothing
      // 50% chance of getting hp
      // 30% chance of getting food
      local iRoll = rand() % 100;
      if( iRoll >= 70 )
      {
        // Gain 3 food
        i_ciPlayer.AdjustVersionedProperty( g_eFoodPropertyVersionArray, 3 );

        local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), U1_PondResultType.Food, 3 );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else if( iRoll >= 20 )
      {
        // Gain 5 hitpoints
        i_ciPlayer.AffectHitpoints( 5 );

        local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), U1_PondResultType.Hitpoints, 5 );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
      }
    }
    else if( ( U1_CoinType.Silver == i_eCoinType ) && iCoin >= 10 )
    {
      i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -10 );

      // 20% chance of getting nothing
      // 45% chance of getting hp
      // 25% chance of getting food
      // 10% chance of getting a spell
      local iRoll = rand() % 100;
      if( iRoll >= 90 )
      {
        // Gain a random spell
        local iIndex = rand() % g_eU1SpellPropertyArray.len();
        local ePropertyID = g_eU1SpellPropertyArray[iIndex];
        i_ciPlayer.SetProperty( ePropertyID, i_ciPlayer.GetProperty( ePropertyID, 0 ) + 1 );

        local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), U1_PondResultType.Spell, ePropertyID );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else if( iRoll >= 65 )
      {
        // Gain 30 food
        i_ciPlayer.AdjustVersionedProperty( g_eFoodPropertyVersionArray, 30 );

        local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), U1_PondResultType.Food, 30 );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else if( iRoll >= 20 )
      {
        // Gain 15 hitpoints
        i_ciPlayer.AffectHitpoints( 15 );

        local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), U1_PondResultType.Hitpoints, 15 );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
      }
    }
    else if( ( U1_CoinType.Gold == i_eCoinType ) && iCoin >= 100 )
    {
      i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -100 );

      // 20% chance of getting nothing
      // 40% chance of getting hp
      // 15% chance of getting food
      // 10% chance of getting any random spell
      // 10% chance of getting any random weapon
      //  5% chance of getting any random stat increase
      local iRoll = rand() % 100;
      if( iRoll >= 95 )
      {
        // Random stat increase by 5
        local iStatIndex = rand() % g_eU1StatPropertyArray.len();
        local eStatPropertyID = g_eU1StatPropertyArray[iStatIndex];
        i_ciPlayer.SetProperty( eStatPropertyID, i_ciPlayer.GetProperty( eStatPropertyID, 0 ) + 1 );

        local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), U1_PondResultType.Stat,
                                         eStatPropertyID );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else if( iRoll >= 85 )
      {
        // Gain a random weapon
        local iWeaponIndex = rand() % g_eU1WeaponInventoryArray.len();
        local eWeaponType = g_eU1WeaponInventoryArray[iWeaponIndex];
        if( i_ciPlayer.AddOrCreateItem( eWeaponType, 1 ) )
        {
          local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), U1_PondResultType.Weapon,
                                           eWeaponType );
          i_ciPlayer.SendBroadcast( ciBroadcast );
        }
      }
      else if( iRoll >= 75 )
      {
        // Gain a random spell
        local ePropertyID = rand() % g_eU1SpellPropertyArray.len();
        i_ciPlayer.SetProperty( ePropertyID, i_ciPlayer.GetProperty( ePropertyID, 0 ) + 1 );

        local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), U1_PondResultType.Spell, ePropertyID );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else if( iRoll >= 65 )
      {
        // Gain 300 food
        i_ciPlayer.AdjustVersionedProperty( g_eFoodPropertyVersionArray, 300 );

        local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), U1_PondResultType.Food, 300 );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else if( iRoll >= 20 )
      {
        // Gain 150 hitpoints
        i_ciPlayer.AffectHitpoints( 150 );

        local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), U1_PondResultType.Hitpoints, 150 );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_none_owned_client_StringID );
    }
  }
}
