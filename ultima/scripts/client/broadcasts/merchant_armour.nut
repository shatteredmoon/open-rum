// Received from server when player transacts with an armour merchant
class Merchant_Armour_Broadcast extends rumBroadcast
{
  var1 = 0; // Armour transaction type
  var2 = 0;


  constructor( ... )
  {
    base.constructor();

    if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Armour transaction type
      var2 = vargv[1]; // Armour class to buy, or armour unique id to sell
    }
  }


  function OnRecv()
  {
    // Save a reference to this instance, otherwise it'll be garbage collected before the callback is made
    g_ciCUO.m_ciMerchantRef = this;

    local eTransactionType = var1;

    if( MerchantArmouryTransaction.Greet == eTransactionType )
    {
      PlayMerchantMusic();

      // Show the relevant armour stat page
      g_ciUI.m_ePreviousStatPage = g_ciUI.m_eCurrentStatPage;

      switch( g_ciCUO.m_eVersion )
      {
        case GameType.Ultima1: g_ciUI.m_eCurrentStatPage = U1_StatPage.Armour; break;
        case GameType.Ultima2: g_ciUI.m_eCurrentStatPage = U2_StatPage.Armour; break;
        case GameType.Ultima3: g_ciUI.m_eCurrentStatPage = U3_StatPage.Armour; break;
        case GameType.Ultima4: g_ciUI.m_eCurrentStatPage = U4_StatPage.Armour; break;
      }

      Ultima_Stat_Update();

      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();

      local eTokenID = ciMap.GetProperty( Merchant_Armour_Name_PropertyID, rumInvalidStringToken );
      local strShopName = ::rumGetString( eTokenID );

      eTokenID = ciMap.GetProperty( Merchant_Armour_Proprietor_PropertyID, rumInvalidStringToken );
      local strProprietor = ::rumGetString( eTokenID );

      local strDesc = format( "%s %s.<b><b>%s %s %s",
                              ::rumGetString( u4_merchant_armour_greet_client_StringID ),
                              strShopName,
                              strProprietor,
                              ::rumGetString( u4_merchant_armour_greet_2_client_StringID ),
                              ::rumGetString( u4_merchant_armour_prompt_2_client_StringID ) );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      g_ciUI.m_ciYesNoListView.Clear();
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_buy_client_StringID ),
                                         rumKeypress.KeyY() );
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_sell_client_StringID ),
                                         rumKeypress.KeyN() );

      g_ciUI.m_ciYesNoListView.m_funcAccept = BuySellCallback.bindenv( this );
      g_ciUI.m_ciYesNoListView.m_funcCancel = TransactionCanceled.bindenv( this );
      g_ciUI.m_ciYesNoListView.SetActive( true );
      g_ciUI.m_ciYesNoListView.Focus();
    }
    else if( MerchantArmouryTransaction.Purchase == eTransactionType )
    {
      local strDesc = var2;
      OfferService( strDesc );
    }
    else if( MerchantArmouryTransaction.Sell == eTransactionType )
    {
      OfferService( u4_merchant_armour_sell_yes_client_StringID );
    }
    else if( MerchantArmouryTransaction.ServerTerminated == eTransactionType )
    {
      local strReason = var2;
      EndTransaction( false /* do not inform server */, strReason );
    }
  }


  function BuySelect()
  {
    local strDesc = ::rumGetString( u4_merchant_armour_buy_client_StringID );
    strDesc += " ";

    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();

    // Build the armour list
    g_ciUI.m_ciGameListView.Clear();
    g_ciUI.m_ciGameListView.SetFormat( "0.05|0.15|0.75" );
    g_ciUI.m_ciGameListView.ShowPrompt( false );

    local iNumShown = 0;
    local eArmourInventoryArray = null;

    local fDiscount = 0.0;

    switch( g_ciCUO.m_eVersion )
    {
      case GameType.Ultima1:
        eArmourInventoryArray = g_eU1ArmourInventoryArray;
        fDiscount = ciPlayer.GetDiscountPercent();
        break;

      case GameType.Ultima2:
        eArmourInventoryArray = g_eU2ArmourInventoryArray;
        fDiscount = ciPlayer.GetDiscountPercent();
        break;

      case GameType.Ultima3:
        eArmourInventoryArray = g_eU3ArmourInventoryArray;
        break;

      case GameType.Ultima4:
        eArmourInventoryArray = g_eU4ArmourInventoryArray;
        break;
    }

    local eArmourArray = null;

    // Determine the data table row
    local eMapArray = ::rumGetDataTableColumn( merchant_armour_DataTableID, 0 );
    local iRow = eMapArray.find( ciMap.GetAssetID() );
    if( iRow != null )
    {
      // Fetch the row and slice off the map column
      eArmourArray = ::rumGetDataTableRow( merchant_armour_DataTableID, iRow ).slice( 1 );
    }

    // Merchants can only sell up to 8 items
    local iMaxItems = 8;
    local iIndex = eArmourArray.find( rumInvalidAssetID );
    local iNumAvailable = ( null == iIndex ) ? iMaxItems : iIndex;

    // Sort ascending
    eArmourArray.resize( iNumAvailable );
    eArmourArray.sort();

    // Visit each armour
    foreach( iIndex, eArmourType in eArmourInventoryArray )
    {
      if( ValueInContainer( eArmourType, eArmourArray ) )
      {
        local ciAsset = ::rumGetInventoryAsset( eArmourType );
        local strArmourDesc = ::rumGetStringByName( ciAsset.GetName() + "_client_StringID" );
        local strAlpha = ::rumGetStringByName( format( "alpha%02d_client_StringID", iIndex ) );

        local iPrice = ciAsset.GetProperty( Inventory_Price_PropertyID, 0 );
        if( iPrice > 0 )
        {
          iPrice -= ( iPrice * fDiscount ).tointeger();

          g_ciUI.m_ciGameListView.SetEntry( iIndex,
                                            strAlpha + "|" + strArmourDesc + "|" + iPrice,
                                            rumKeypress.KeyA() + iIndex );

          ++iNumShown;
          if( iNumShown > 1 && iNumShown < iNumAvailable )
          {
            strDesc += ", ";
          }
          else if( iNumShown > 1 && ( iNumShown == iNumAvailable ) )
          {
            strDesc += " ";
            strDesc += ::rumGetString( token_or_client_StringID );
            strDesc += " ";
          }

          strDesc += strArmourDesc;
        }
      }
    }

    strDesc += ".<b><b>";
    strDesc += ::rumGetString( u4_merchant_armour_prompt_client_StringID );
    ShowString( strDesc, g_strColorTagArray.Cyan );

    CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

    g_ciUI.m_ciGameListView.m_funcAccept = BuySelectCallback.bindenv( this );
    g_ciUI.m_ciGameListView.m_funcCancel = TransactionCanceled.bindenv( this );
    g_ciUI.m_ciGameListView.m_funcIndexChanged = SelectionChanged.bindenv( this );
    g_ciUI.m_ciGameListView.SetActive( true );
    g_ciUI.m_ciGameListView.Focus();
  }


  function BuySelectCallback()
  {
    // Push the player's selection to output
    local strDesc = g_ciUI.m_ciGameInputTextBox.GetText();
    ShowString( strDesc + "<b>" );

    strDesc = "";

    local ciPlayer = ::rumGetMainPlayer();
    local eArmourInventoryArray = null;
    local fDiscount = 0.0;

    switch( g_ciCUO.m_eVersion )
    {
      case GameType.Ultima1:
        eArmourInventoryArray = g_eU1ArmourInventoryArray;
        fDiscount = ciPlayer.GetDiscountPercent();
        break;

      case GameType.Ultima2:
        eArmourInventoryArray = g_eU2ArmourInventoryArray;
        fDiscount = ciPlayer.GetDiscountPercent();
        break;

      case GameType.Ultima3:
        eArmourInventoryArray = g_eU3ArmourInventoryArray;
        break;

      case GameType.Ultima4:
        eArmourInventoryArray = g_eU4ArmourInventoryArray;
        break;
    }

    local iArmourKey = g_ciUI.m_ciGameListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    local eArmourType = eArmourInventoryArray[iArmourKey];
    local ciAsset = ::rumGetInventoryAsset( eArmourType );
    local iPrice = ciAsset.GetProperty( Inventory_Price_PropertyID, 0 );
    if( iPrice > 0 )
    {
      iPrice -= ( iPrice * fDiscount ).tointeger();

      if( GameType.Ultima4 == g_ciCUO.m_eVersion )
      {
        strDesc = ::rumGetStringByName( "u4_merchant_armour_desc" + ( iArmourKey + 1 ) + "_client_StringID" );
        strDesc = format( strDesc, iPrice );
        strDesc += "<b><b>";
      }

      strDesc += ::rumGetString( u4_merchant_armour_buy_prompt_client_StringID );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      g_ciUI.m_ciYesNoListView.Clear();
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                         rumKeypress.KeyY() );
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                         rumKeypress.KeyN() );

      g_ciUI.m_ciYesNoListView.m_funcAccept = BuyVerifyCallback.bindenv( this );
      g_ciUI.m_ciYesNoListView.m_funcCancel = PurchaseCanceled.bindenv( this );
      g_ciUI.m_ciYesNoListView.SetActive( true );
      g_ciUI.m_ciYesNoListView.Focus();

      g_ciUI.m_ciGameInputTextBox.m_vPayload = eArmourType;
    }
    else
    {
      // Something went wrong with pricing
      OfferService();
    }
  }


  function BuySellCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    if( BuySellResponse.Buy == eResponse )
    {
      BuySelect();
    }
    else
    {
      SellSelect();
    }
  }


  function BuyVerifyCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local ciPlayer = ::rumGetMainPlayer();

    local eArmourInventoryArray = null;
    local fDiscount = 0.0;

    switch( g_ciCUO.m_eVersion )
    {
      case GameType.Ultima1:
        eArmourInventoryArray = g_eU1ArmourInventoryArray;
        fDiscount = ciPlayer.GetDiscountPercent();
        break;

      case GameType.Ultima2:
        eArmourInventoryArray = g_eU2ArmourInventoryArray;
        fDiscount = ciPlayer.GetDiscountPercent();
        break;

      case GameType.Ultima3:
        eArmourInventoryArray = g_eU3ArmourInventoryArray;
        break;

      case GameType.Ultima4:
        eArmourInventoryArray = g_eU4ArmourInventoryArray;
        break;
    }

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();
    local eArmourType = g_ciUI.m_ciGameInputTextBox.m_vPayload;

    Ultima_ListSelectionEnd();

    local ciAsset = ::rumGetInventoryAsset( eArmourType );
    local iPrice = ciAsset.GetProperty( Inventory_Price_PropertyID, 0 );

    if( iPrice <= 0 || ( YesNoResponse.No == eResponse ) )
    {
      // The player does not want the armour after all
      OfferService( u4_merchant_armour_buy_no_client_StringID );
    }
    else if( YesNoResponse.Yes == eResponse )
    {
      // Does the player have enough gold to make the purchase?
      local iPlayerGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );

      // Apply discount
      iPrice -= ( iPrice * fDiscount ).tointeger();
      if( iPlayerGold >= iPrice )
      {
        // Send the buy request to the server
        local ciBroadcast = ::rumCreate( Merchant_Armour_BroadcastID, MerchantArmouryTransaction.Purchase,
                                         eArmourType );
        ::rumSendBroadcast( ciBroadcast );
      }
      else
      {
        // Not enough gold!
        OfferService( u4_merchant_armour_cant_pay_client_StringID );
      }
    }
  }


  function ContinueCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    if( YesNoResponse.Yes == eResponse )
    {
      local strDesc = ::rumGetString( u4_merchant_armour_prompt_2_client_StringID );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      g_ciUI.m_ciYesNoListView.Clear();
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_buy_client_StringID ),
                                         rumKeypress.KeyY() );
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_sell_client_StringID ),
                                         rumKeypress.KeyN() );

      g_ciUI.m_ciYesNoListView.m_funcAccept = BuySellCallback.bindenv( this );
      g_ciUI.m_ciYesNoListView.m_funcCancel = TransactionCanceled.bindenv( this );
      g_ciUI.m_ciYesNoListView.SetActive( true );
      g_ciUI.m_ciYesNoListView.Focus();
    }
    else
    {
      EndTransaction( true /* inform server */ );
    }
  }


  function EndTransaction( ... )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    local strDesc;

    if( vargv.len() > 1 && vargv[1] )
    {
      strDesc = ::rumGetString( vargv[1] );
    }
    else
    {
      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();

      local eTokenID = ciMap.GetProperty( Merchant_Armour_Proprietor_PropertyID, rumInvalidStringToken );
      local strProprietor = ::rumGetString( eTokenID );

      local strDesc = ::rumGetString( u4_merchant_armour_bye_client_StringID );
      strDesc = format( "%s %s<b>" strProprietor, strDesc );
      ShowString( strDesc, g_strColorTagArray.Cyan );
    }

    local bSendTermination = vargv.len() >= 1 ? vargv[0] : true;
    if( bSendTermination )
    {
      // End transaction on the server
      local ciBroadcast = ::rumCreate( Player_Talk_Bye_BroadcastID, DialogueTerminationType.Standard );
      ::rumSendBroadcast( ciBroadcast );
    }

    StopMusic();

    // Show the player's previously selected stat page
    Ultima_Stat_Restore();

    g_ciCUO.m_ciMerchantRef = null;
  }


  function OfferService( ... )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    local strDesc;

    if( ( 1 == vargv.len() ) && vargv[0] )
    {
      strDesc = format( "%s<b><b>%s",
                        ::rumGetString( vargv[0] ),
                        ::rumGetString( u4_merchant_armour_continue_client_StringID ) );
    }
    else
    {
      strDesc = ::rumGetString( u4_merchant_armour_continue_client_StringID );
    }

    ShowString( strDesc, g_strColorTagArray.Cyan );

    // See if the player wants to buy or sell again
    g_ciUI.m_ciYesNoListView.Clear();
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                       rumKeypress.KeyY() );
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                       rumKeypress.KeyN() );

    g_ciUI.m_ciYesNoListView.m_funcAccept = ContinueCallback.bindenv( this );
    g_ciUI.m_ciYesNoListView.m_funcCancel = TransactionCanceled.bindenv( this );
    g_ciUI.m_ciYesNoListView.SetActive( true );
    g_ciUI.m_ciYesNoListView.Focus();
  }


  function PurchaseCanceled( ... )
  {
    ShowString( "" );
    Ultima_ListSelectionEnd();
    OfferService( u4_merchant_armour_buy_no_client_StringID );
  }


  function SelectionChanged( i_iIndex )
  {
    local strText = g_ciUI.m_ciGameListView.GetCurrentEntry();
    if( strText )
    {
      local strArray = split( strText, "|" );
      g_ciUI.m_ciGameInputTextBox.SetText( strArray[1] );
    }
    else
    {
      g_ciUI.m_ciGameInputTextBox.Clear();
    }
  }


  function SellCallback()
  {
    // Push the player's selection to output
    local strDesc = g_ciUI.m_ciGameInputTextBox.GetText();
    ShowString( strDesc + "<b>" );

    local iArmourID = g_ciUI.m_ciGameListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    local ciPlayer = ::rumGetMainPlayer();
    local ciArmour = ciPlayer.GetInventory( iArmourID );
    if( !( ciArmour instanceof rumInventory ) )
    {
      return;
    }

    local fDiscount = 0.0;

    switch( g_ciCUO.m_eVersion )
    {
      case GameType.Ultima1:
      case GameType.Ultima2:
        fDiscount = ciPlayer.GetDiscountPercent();
        break;
    }

    local iValue = ( ciArmour.GetProperty( Inventory_Price_PropertyID, 0 ) / 2.0 - ( 2.0 * fDiscount ) ).tointeger();

    // Merchant offer for the armour
    local strDesc = ::rumGetString( u4_merchant_armour_sell_offer_client_StringID );
    strDesc = format( strDesc, iValue, ::rumGetStringByName( ciArmour.GetName() + "_client_StringID" ) );
    ShowString( strDesc, g_strColorTagArray.Cyan );

    g_ciUI.m_ciYesNoListView.Clear();
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                       rumKeypress.KeyY() );
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                       rumKeypress.KeyN() );

    g_ciUI.m_ciYesNoListView.m_funcAccept = SellVerifyCallback.bindenv( this );
    g_ciUI.m_ciYesNoListView.m_funcCancel = SellCanceled.bindenv( this );
    g_ciUI.m_ciYesNoListView.SetActive( true );
    g_ciUI.m_ciYesNoListView.Focus();

    g_ciUI.m_ciGameInputTextBox.m_vPayload = iArmourID;
  }


  function SellCanceled( ... )
  {
    ShowString( "" );
    Ultima_ListSelectionEnd();
    OfferService( u4_merchant_armour_sell_no_client_StringID );
  }


  function SellSelect()
  {
    // Iterate over the player's armour and build the armour list
    g_ciUI.m_ciGameListView.Clear();
    g_ciUI.m_ciGameListView.SetFormat( "0.05|0.5" );
    g_ciUI.m_ciGameListView.ShowPrompt( false );

    local ciPlayer = ::rumGetMainPlayer();
    local uiEquippedArmourID = ciPlayer.GetEquippedArmourID();

    local ciInventory = ciPlayer.GetInventory();
    local ciItem;
    while( ciItem = ciInventory.GetNextObject() )
    {
      if( null == ciItem )
      {
        continue;
      }

      local uiItemID = ciItem.GetID();

      // Add the armour to the UI control - the key of the armour is the armour's UniqueID
      if( uiItemID != uiEquippedArmourID )
      {
        local eType = ciItem.GetProperty(Inventory_Type_PropertyID, InventoryType.Standard);
        if( InventoryType.Armour == eType )
        {
          local eVersion = ciItem.GetProperty(Ultima_Version_PropertyID, 0);
          if( eVersion == g_ciCUO.m_eVersion)
          {
            local strEntry = ::rumGetStringByName( ciItem.GetName() + "_client_StringID" );
            g_ciUI.m_ciGameListView.SetEntry( uiItemID, strEntry );
          }
        }
      }
    }

    if( g_ciUI.m_ciGameListView.GetNumEntries() > 0 )
    {
      local strDesc = ::rumGetString( u4_merchant_armour_sell_client_StringID );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

      g_ciUI.m_ciGameListView.m_funcAccept = SellCallback.bindenv( this );
      g_ciUI.m_ciGameListView.m_funcCancel = PurchaseCanceled.bindenv( this );
      g_ciUI.m_ciGameListView.SetActive( true );
      g_ciUI.m_ciGameListView.Focus();
    }
    else
    {
      OfferService( u4_merchant_armour_sell_no_items_client_StringID );
    }
  }


  function SellVerifyCallback()
  {
    // Push the selection
    local strDesc = g_ciUI.m_ciGameInputTextBox.GetText();
    ShowString( strDesc + "<b>" );

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();
    local iArmourID = g_ciUI.m_ciGameInputTextBox.m_vPayload;

    Ultima_ListSelectionEnd();

    if( YesNoResponse.No == eResponse )
    {
      // The player does not want to sell the armour after all
      OfferService( u4_merchant_armour_sell_no_client_StringID );
    }
    else if( YesNoResponse.Yes == eResponse )
    {
      // The player definitely wants to sell the armour
      local ciBroadcast = ::rumCreate( Merchant_Armour_BroadcastID, MerchantArmouryTransaction.Sell, iArmourID );
      ::rumSendBroadcast( ciBroadcast );
    }
  }


  function TransactionCanceled()
  {
    ShowString( "" );
    Ultima_ListSelectionEnd();
    EndTransaction( true /* inform server */ );
  }
}
