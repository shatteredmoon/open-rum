function TransactTavernRecv( i_ciPlayer, i_eTransactionType, i_vValue )
{
  local ciMap = i_ciPlayer.GetMap();

  if( MerchantTavernTransaction.PurchaseFood == i_eTransactionType )
  {
    local iPrice = ciMap.GetProperty( Merchant_Tavern_Food_Price_PropertyID, 2 );

    // Can the player afford the purchase?
    local iGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
    if( iGold >= iPrice )
    {
      i_ciPlayer.AdjustVersionedProperty( g_eFoodPropertyVersionArray, 1 );
      i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iPrice );

      local ciBroadcast = ::rumCreate( Merchant_Tavern_BroadcastID, MerchantTavernTransaction.PurchaseFood );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
    else
    {
      local ciBroadcast = ::rumCreate( Merchant_Tavern_BroadcastID, MerchantTavernTransaction.ServerTerminated,
                                       u4_merchant_tavern_wont_pay_client_StringID );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
  }
  else if( MerchantTavernTransaction.PurchaseAle == i_eTransactionType )
  {
    local iPurchaseAmount = i_vValue;
    local iPrice = ciMap.GetProperty( Merchant_Tavern_Ale_Price_PropertyID, 1 );

    // Can the player afford the purchase?
    local iGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
    if( iPurchaseAmount >= iPrice && iGold >= iPrice )
    {
      // Did the player offer a larger tip than they can provide?
      if( iPurchaseAmount > iGold )
      {
        // Just scale down to what they can afford
        iPurchaseAmount = iGold;
      }

      local iTipAmount = iPurchaseAmount - iPrice;
      local iTipAmount = clamp( iTipAmount, 0, 99 );

      i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iPurchaseAmount );

      local bTipped = ( iTipAmount > 0 );
      local eVersion = i_ciPlayer.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
      if( GameType.Ultima4 == eVersion )
      {
        // If the player left a tip, allow the player to ask about something
        local ciBroadcast = ::rumCreate( Merchant_Tavern_BroadcastID, MerchantTavernTransaction.PurchaseAle, bTipped );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else
      {
        local iClueIndex = iTipAmount / 10;
        local strToken = format( "u%d_merchant_tavern_clue_%d_server_StringID", eVersion, iClueIndex );
        local strClue = ::rumGetStringByName( strToken, i_ciPlayer.m_iLanguageID );

        local ciBroadcast = ::rumCreate( Merchant_Tavern_BroadcastID, MerchantTavernTransaction.Tip, strClue );
        i_ciPlayer.SendBroadcast( ciBroadcast );

        // TODO - U1 wench/lecher
      }
    }
    else
    {
      local ciBroadcast = ::rumCreate( Merchant_Tavern_BroadcastID, MerchantTavernTransaction.ServerTerminated,
                                       u4_merchant_tavern_wont_pay_client_StringID );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
  }
  else if( MerchantTavernTransaction.Question == i_eTransactionType )
  {
    local strPlayerKeyword = i_vValue;
    local eTokenID = ciMap.GetProperty( Merchant_Tavern_Keyword_PropertyID, rumInvalidStringToken );
    local strMerchantKeyword = ::rumGetString( eTokenID, i_ciPlayer.m_iLanguageID );
    local bMatch = ( i_vValue != "" && ( strPlayerKeyword == strMerchantKeyword ) );

    local ciBroadcast = ::rumCreate( Merchant_Tavern_BroadcastID, MerchantTavernTransaction.Question, bMatch );
    i_ciPlayer.SendBroadcast( ciBroadcast );
  }
  else if( MerchantTavernTransaction.Tip == i_eTransactionType )
  {
    local iTipAmount = i_vValue;

    // Can the player afford the tip?
    local iPlayerGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );

    // Did the player offer a larger tip than they can provide?
    if( iTipAmount > iPlayerGold )
    {
      // Just scale down to what they can afford
      iTipAmount = iPlayerGold;
    }

    if( iTipAmount > 0 )
    {
      i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iTipAmount );

      // Determine if the merchant feels like talking (the player's tip is higher than a random roll)
      local iChance = rand() % 90 + 10; // 10 - 99
      local bTalk = ( iTipAmount >= iChance );
      if( bTalk )
      {
        local eTokenID = ciMap.GetProperty( Merchant_Tavern_Clue_PropertyID, rumInvalidStringToken );
        local strClue = ::rumGetString( eTokenID, i_ciPlayer.m_iLanguageID );

        // Remember if a player has learned about sextants!
        if( u4_merchant_tavern_jhelom_clue_server_StringID == eTokenID )
        {
          i_ciPlayer.SetProperty( U4_Sextant_Accessible_PropertyID, true );
        }

        local ciBroadcast = ::rumCreate( Merchant_Tavern_BroadcastID, MerchantTavernTransaction.Tip, bTalk, strClue );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else
      {
        local ciBroadcast = ::rumCreate( Merchant_Tavern_BroadcastID, MerchantTavernTransaction.Tip, bTalk );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
    }
    else
    {
      local ciBroadcast = ::rumCreate( Merchant_Tavern_BroadcastID, MerchantTavernTransaction.Tip, bTalk );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
  }
  else
  {
    // Hack attempt - invalid transaction type
    i_ciPlayer.IncrementHackAttempts();

    local ciBroadcast = ::rumCreate( Merchant_Tavern_BroadcastID, MerchantTavernTransaction.ServerTerminated );
    i_ciPlayer.SendBroadcast( ciBroadcast );
  }
}


function TransactTavernStart( i_ciPlayer )
{
  local ciBroadcast = ::rumCreate( Merchant_Tavern_BroadcastID, MerchantTavernTransaction.Greet );
  ::rumSendPrivate( i_ciPlayer.GetSocket(), ciBroadcast );
}
