function TransactHealerDonateBlood( i_ciPlayer, i_bDonate )
{
  // Double-check that the player is eligible to donate
  local eVersion = i_ciPlayer.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
  if( GameType.Ultima4 != eVersion )
  {
    i_ciPlayer.IncrementHackAttempts();
    return;
  }

  local iLevel = i_ciPlayer.GetProperty( U4_Level_PropertyID, 1 );
  if( iLevel >= 4 )
  {
    local iHitpoints = i_ciPlayer.GetProperty( U4_Hitpoints_PropertyID, 0 );
    if( iHitpoints > 100 )
    {
      if( i_bDonate )
      {
        i_ciPlayer.Damage( 100, null );
        i_ciPlayer.AffectVirtue( VirtueType.Sacrifice, 5, false, true );
      }
      else
      {
        // This differs from the original by only penalizing the player by a single point instead of 5
        i_ciPlayer.AffectVirtue( VirtueType.Sacrifice, -1, true, true );
      }
    }
  }
}


function TransactHealerStart( i_ciPlayer )
{
  local ciBroadcast = ::rumCreate( Merchant_Healer_BroadcastID, MerchantHealerTransaction.Greet );
  ::rumSendPrivate( i_ciPlayer.GetSocket(), ciBroadcast );
}


function TransactHealerRecv( i_ciPlayer, i_eTransactionType, i_eService )
{
  if( MerchantHealerTransaction.DonateBlood == i_eTransactionType )
  {
    TransactHealerDonateBlood( i_ciPlayer, i_eService );
    return;
  }

  local ciMap = i_ciPlayer.GetMap();

  local fDiscount = 0.0;
  local eVersion = i_ciPlayer.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
  switch( eVersion )
  {
    case GameType.Ultima1:
    case GameType.Ultima2:
      fDiscount = i_ciPlayer.GetDiscountPercent();
      break;
  }

  // Can the player afford the service?
  local iPlayerGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
  local iPriceArray = ::rumGetDataTableColumn( merchant_healer_DataTableID, 1 );
  local iPrice = iPriceArray[i_eService];
  iPrice -= ( iPrice * fDiscount ).tointeger();
  if( iPrice <= 0 )
  {
    // Something is wrong with pricing - terminate the transaction
    local ciBroadcast = ::rumCreate( Merchant_Healer_BroadcastID, MerchantHealerTransaction.ServerTerminated,
                                     u4_merchant_healer_bye_client_StringID );
    i_ciPlayer.SendBroadcast( ciBroadcast );
    return;
  }

  if( iPlayerGold < iPrice )
  {
    // Terminate the transaction
    local ciBroadcast = ::rumCreate( Merchant_Healer_BroadcastID, MerchantHealerTransaction.ServerTerminated,
                                     u4_merchant_healer_cant_pay_client_StringID );
    i_ciPlayer.SendBroadcast( ciBroadcast );
    return;
  }

  local uiStringID;
  local bNeedsService = true;

  switch( i_eService )
  {
    case MerchantHealerService.Cure:
      if( i_ciPlayer.IsPoisoned() )
      {
        i_ciPlayer.RemoveVersionedProperty( g_ePoisonedPropertyVersionArray );
        uiStringID = u4_merchant_healer_cure_ok_client_StringID;
        SendClientEffect( i_ciPlayer, ClientEffectType.Cast );
        local ciNPC = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
        if( ciNPC )
        {
          SendClientEffect( ciNPC, ClientEffectType.Cast );
        }
      }
      else
      {
        uiStringID = u4_merchant_healer_cant_cure_client_StringID;
        bNeedsService = false;
      }
      break;

    case MerchantHealerService.Heal:
      local iHitpoints = i_ciPlayer.GetVersionedProperty( g_eHitpointsPropertyVersionArray );
      local iMaxHitpoints = i_ciPlayer.GetMaxHitpoints();
      if( iHitpoints < iMaxHitpoints )
      {
        i_ciPlayer.SetVersionedProperty( g_eHitpointsPropertyVersionArray, iMaxHitpoints );
        uiStringID = u4_merchant_healer_heal_ok_client_StringID;
        SendClientEffect( i_ciPlayer, ClientEffectType.Cast );
        local ciNPC = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
        if( ciNPC )
        {
          SendClientEffect( ciNPC, ClientEffectType.Cast );
        }
      }
      else
      {
        uiStringID = u4_merchant_healer_cant_heal_client_StringID;
        bNeedsService = false;
      }
      break;

    case MerchantHealerService.Resurrect:
      local bBound = i_ciPlayer.GetVersionedProperty( g_eSpiritBoundPropertyVersionArray );
      if( !bBound )
      {
        i_ciPlayer.SetVersionedProperty( g_eSpiritBoundPropertyVersionArray, true );
        uiStringID = u4_merchant_healer_res_ok_client_StringID;
        SendClientEffect( i_ciPlayer, ClientEffectType.Cast );
        local ciNPC = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
        if( ciNPC )
        {
          SendClientEffect( ciNPC, ClientEffectType.Cast );
        }
      }
      else
      {
        uiStringID = u4_merchant_healer_cant_resurrect_client_StringID;
        bNeedsService = false;
      }
      break;

    case MerchantHealerService.Potions:
      i_ciPlayer.AdjustVersionedProperty( g_ePotionsPropertyVersionArray, 5 );
      uiStringID = u4_merchant_healer_potions_sold_client_StringID;
      break;
  }

  if( bNeedsService )
  {
    // Show results and offer more services
    local ciBroadcast = ::rumCreate( Merchant_Healer_BroadcastID, MerchantHealerTransaction.Purchase, uiStringID );
    i_ciPlayer.SendBroadcast( ciBroadcast );

    // Deduct the cost from the player
    i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iPrice );
  }
  else
  {
    // Terminate the transaction
    local ciBroadcast = ::rumCreate( Merchant_Healer_BroadcastID, MerchantHealerTransaction.ServerTerminated,
                                     uiStringID );
    i_ciPlayer.SendBroadcast( ciBroadcast );
  }
}
