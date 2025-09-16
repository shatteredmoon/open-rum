function TransactRationsStart( i_ciPlayer )
{
  local ciBroadcast = ::rumCreate( Merchant_Rations_BroadcastID, MerchantRationTransaction.Greet );
  ::rumSendPrivate( i_ciPlayer.GetSocket(), ciBroadcast );
}


function TransactRationsRecv( i_ciPlayer, i_eTransactionType, i_iNumPacks )
{
  local strResult = "u4_merchant_rations_buy_cancel";

  if( i_iNumPacks > 0 )
  {
    local eVersion = i_ciPlayer.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );

    local iCurrentFood = i_ciPlayer.GetVersionedProperty( g_eFoodPropertyVersionArray );
    local iFreeFoodSlots = ::rumGetMaxPropertyValue( g_eFoodPropertyVersionArray[eVersion] ) - iCurrentFood;

    local ciMap = i_ciPlayer.GetMap();
    local iPackSize = ciMap.GetProperty( Merchant_Rations_Pack_Size_PropertyID, 0 );
    if( iPackSize > 0 )
    {
      local fNumFreePacks = iFreeFoodSlots / iPackSize;
      local iNumFreePacks = fNumFreePacks.tointeger();
      if( iNumFreePacks > 0 )
      {
        local fDiscount = 0.0;
        switch( eVersion )
        {
          case GameType.Ultima1:
          case GameType.Ultima2:
            fDiscount = i_ciPlayer.GetDiscountPercent();
            break;
        }

        // Can the player afford the purchase?
        local iPackPrice = ciMap.GetProperty( Merchant_Rations_Price_PropertyID, 0 );
        iPackPrice -= ( iPackPrice * fDiscount ).tointeger();
        if( iPackPrice > 0 )
        {
          local iPurchasePrice = i_iNumPacks * iPackPrice;
          local iPlayerGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
          if( iPlayerGold >= iPurchasePrice )
          {
            i_ciPlayer.AdjustVersionedProperty( g_eFoodPropertyVersionArray, i_iNumPacks * iPackSize );
            i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iPurchasePrice );

            strResult = "u4_merchant_rations_purchase";
          }
          else
          {
            // Player cannot afford the purchase - why was this message even sent?
            strResult = "u4_merchant_healer_cant_pay";
            i_ciPlayer.IncrementHackAttempts();
          }
        }
      }
      else
      {
        // Player doesn't need food - why was this message even sent?
        strResult = "u4_merchant_rations_full";
        i_ciPlayer.IncrementHackAttempts();
      }
    }
  }
  else
  {
    // Player didn't want food - why was this message even sent?
    i_ciPlayer.IncrementHackAttempts();
  }

  local ciBroadcast = ::rumCreate( Merchant_Rations_BroadcastID, MerchantRationTransaction.Purchase, strResult );
  i_ciPlayer.SendBroadcast( ciBroadcast );
}


function TransactRationsSteal( i_ciCreature, i_ciPlayer )
{
  local ciMap = i_ciPlayer.GetMap();

  local bSuccess = i_ciPlayer.AttemptSteal( i_ciCreature, ciMap.s_cRationStealModifier );
  if( bSuccess )
  {
    // Player steals 1-20 rations
    local iStolenAmount = rand() % 20 + 1;
    i_ciPlayer.AdjustVersionedProperty( g_eFoodPropertyVersionArray, iStolenAmount );

    local ciBroadcast = ::rumCreate( Player_Steal_BroadcastID, Food_PropertyID, iStolenAmount );
    i_ciPlayer.SendBroadcast( ciBroadcast );
  }
}
