// Received from server when player transacts with a weapons merchant
class Merchant_Weapons_Broadcast extends rumBroadcast
{
  var1 = 0; // Weapon transaction type
  var2 = 0; // Result
  var3 = 0;


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 1 )
    {
      var1 = vargv[0]; // Weapon transaction type
      var2 = vargv[1]; // Weapon class to buy, or weapon unique id to sell

      if( vargv.len() > 2 )
      {
        var3 = vargv[2]; // Quantity
      }
    }
  }


  function OnRecv()
  {
    // Save a reference to this instance, otherwise it'll be garbage collected before the callback is made
    g_ciCUO.m_ciMerchantRef = this;

    local eTransactionType = var1;

    if( MerchantWeaponryTransaction.Greet == eTransactionType )
    {
      PlayMerchantMusic();

      // Show the relevant weapon stat page
      g_ciUI.m_ePreviousStatPage = g_ciUI.m_eCurrentStatPage;

      switch( g_ciCUO.m_eVersion )
      {
        case GameType.Ultima1: g_ciUI.m_eCurrentStatPage = U1_StatPage.Weapons; break;
        case GameType.Ultima2: g_ciUI.m_eCurrentStatPage = U2_StatPage.Weapons; break;
        case GameType.Ultima3: g_ciUI.m_eCurrentStatPage = U3_StatPage.Weapons; break;
        case GameType.Ultima4: g_ciUI.m_eCurrentStatPage = U4_StatPage.Weapons; break;
      }

      Ultima_Stat_Update();

      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();

      local eTokenID = ciMap.GetProperty( Merchant_Weapons_Name_PropertyID, rumInvalidStringToken );
      local strShopName = ::rumGetString( eTokenID );

      eTokenID = ciMap.GetProperty( Merchant_Weapons_Proprietor_PropertyID, rumInvalidStringToken );
      local strProprietor = ::rumGetString( eTokenID );

      local strDesc = format( "%s %s.<b><b>%s %s",
                              ::rumGetString( u4_merchant_weapon_greet_client_StringID ),
                              strShopName,
                              strProprietor,
                              ::rumGetString( u4_merchant_weapon_greet_2_client_StringID ) );
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
    else if( MerchantWeaponryTransaction.Purchase == eTransactionType )
    {
      local strDesc = var2;
      OfferService( strDesc );
    }
    else if( MerchantWeaponryTransaction.Sell == eTransactionType )
    {
      OfferService( u4_merchant_weapon_sell_yes_client_StringID );
    }
    else if( MerchantWeaponryTransaction.ServerTerminated == eTransactionType )
    {
      local strReason = var2;
      EndTransaction( false /* do not inform server */, strReason );
    }
  }


  function BuySelect()
  {
    local strDesc = ::rumGetString( u4_merchant_weapon_buy_client_StringID );
    strDesc += " ";

    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();

    // Build the weapon list
    g_ciUI.m_ciGameListView.Clear();
    g_ciUI.m_ciGameListView.SetFormat( "0.05|0.15|0.75" );
    g_ciUI.m_ciGameListView.ShowPrompt( false );

    local iNumShown = 0;
    local eWeaponInventoryArray = null;

    local fDiscount = 0.0;

    switch( g_ciCUO.m_eVersion )
    {
      case GameType.Ultima1:
        eWeaponInventoryArray = g_eU1WeaponInventoryArray;
        fDiscount = ciPlayer.GetDiscountPercent();
        break;

      case GameType.Ultima2:
        eWeaponInventoryArray = g_eU2WeaponInventoryArray;
        fDiscount = ciPlayer.GetDiscountPercent();
        break;

      case GameType.Ultima3:
        eWeaponInventoryArray = g_eU3WeaponInventoryArray;
        break;

      case GameType.Ultima4:
        eWeaponInventoryArray = g_eU4WeaponInventoryArray;
        break;
    }

    local eWeaponArray = null;

    // Determine the data table row
    local eMapArray = ::rumGetDataTableColumn( merchant_weapons_DataTableID, 0 );
    local iRow = eMapArray.find( ciMap.GetAssetID() );
    if( iRow != null )
    {
      // Fetch the row and slice off the map column
      eWeaponArray = ::rumGetDataTableRow( merchant_weapons_DataTableID, iRow ).slice( 1 );
    }

    // Merchants can only sell up to 8 items
    local iMaxItems = 8;
    local iIndex = eWeaponArray.find( rumInvalidAssetID );
    local iNumAvailable = ( null == iIndex ) ? iMaxItems : iIndex;

    // Sort ascending
    eWeaponArray.resize( iNumAvailable );
    eWeaponArray.sort();

    // Visit each weapon
    foreach( iIndex, eWeaponType in eWeaponInventoryArray )
    {
      if( ValueInContainer( eWeaponType, eWeaponArray ) )
      {
        local ciAsset = ::rumGetInventoryAsset( eWeaponType );
        local strWeaponDesc = ::rumGetStringByName( ciAsset.GetName() + "_client_StringID" );
        local strAlpha = ::rumGetStringByName( format( "alpha%02d_client_StringID", iIndex ) );

        local iPrice = ciAsset.GetProperty( Inventory_Price_PropertyID, 0 );
        if( iPrice > 0 )
        {
          iPrice -= ( iPrice * fDiscount ).tointeger();

          g_ciUI.m_ciGameListView.SetEntry( iIndex,
                                            strAlpha + "|" + strWeaponDesc + "|" + iPrice,
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

          strDesc += strWeaponDesc;
        }
      }
    }

    strDesc += ".<b><b>";
    strDesc += ::rumGetString( u4_merchant_weapon_prompt_client_StringID );
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
    local eWeaponInventoryArray = null;
    local fDiscount = 0.0;

    switch( g_ciCUO.m_eVersion )
    {
      case GameType.Ultima1:
        eWeaponInventoryArray = g_eU1WeaponInventoryArray;
        fDiscount = ciPlayer.GetDiscountPercent();
        break;

      case GameType.Ultima2:
        eWeaponInventoryArray = g_eU2WeaponInventoryArray;
        fDiscount = ciPlayer.GetDiscountPercent();
        break;

      case GameType.Ultima3:
        eWeaponInventoryArray = g_eU3WeaponInventoryArray;
        break;

      case GameType.Ultima4:
        eWeaponInventoryArray = g_eU4WeaponInventoryArray;
        break;
    }

    local iWeaponKey = g_ciUI.m_ciGameListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    local eWeaponType = eWeaponInventoryArray[iWeaponKey];
    local ciAsset = ::rumGetInventoryAsset( eWeaponType );
    local iPrice = ciAsset.GetProperty( Inventory_Price_PropertyID, 0 );
    if( iPrice > 0 )
    {
      iPrice -= ( iPrice * fDiscount ).tointeger();

      if( GameType.Ultima4 == g_ciCUO.m_eVersion )
      {
        strDesc = ::rumGetStringByName( "u4_merchant_weapon_desc" + ( iWeaponKey + 1 ) + "_client_StringID" );
        strDesc = format( strDesc, iPrice );
        strDesc += "<b><b>";
      }

      strDesc += ::rumGetString( u4_merchant_weapon_buy_prompt_client_StringID );
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

      g_ciUI.m_ciGameInputTextBox.m_vPayload = eWeaponType;
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

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();
    local eWeaponType = g_ciUI.m_ciGameInputTextBox.m_vPayload;

    Ultima_ListSelectionEnd();

    if( YesNoResponse.Yes == eResponse )
    {
      local ciAsset = ::rumGetInventoryAsset( eWeaponType );
      if( ciAsset.GetProperty( Inventory_Stacks_PropertyID, false ) )
      {
        ShowString( ::rumGetString( merchant_magic_buy_prompt_client_StringID ), g_strColorTagArray.Cyan );
        g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, BuyVerifyCallbackQuantity.bindenv( this ),
                                                  eWeaponType );
        g_ciUI.m_ciGameInputTextBox.Focus();
      }
      else
      {
        BuyVerifyCallbackQuantity( eWeaponType, 1 );
      }
    }
    else
    {
      // The player does not want the weapon after all
      OfferService( u4_merchant_weapon_buy_no_client_StringID );
    }
  }


  function BuyVerifyCallbackQuantity( i_eWeaponType, i_iQuantity )
  {
    local ciProperty = ::rumGetPropertyAsset( Inventory_Quantity_PropertyID );
    i_iQuantity = clamp( i_iQuantity, ciProperty.GetMinValue(), ciProperty.GetMaxValue() );

    local ciAsset = ::rumGetInventoryAsset( i_eWeaponType );
    if( ciAsset.GetProperty( Inventory_Stacks_PropertyID, false ) )
    {
      // Push the amount player wants to sell to output
      local strAmount = g_ciUI.m_ciGameInputTextBox.GetText();
      ShowString( strAmount + "<b>" );
    }

    local ciPlayer = ::rumGetMainPlayer();
    local eWeaponInventoryArray = null;
    local fDiscount = 0.0;

    switch( g_ciCUO.m_eVersion )
    {
      case GameType.Ultima1:
        eWeaponInventoryArray = g_eU1WeaponInventoryArray;
        fDiscount = ciPlayer.GetDiscountPercent();
        break;

      case GameType.Ultima2:
        eWeaponInventoryArray = g_eU2WeaponInventoryArray;
        fDiscount = ciPlayer.GetDiscountPercent();
        break;

      case GameType.Ultima3:
        eWeaponInventoryArray = g_eU3WeaponInventoryArray;
        break;

      case GameType.Ultima4:
        eWeaponInventoryArray = g_eU4WeaponInventoryArray;
        break;
    }

    local iPrice = ciAsset.GetProperty( Inventory_Price_PropertyID, 0 ) * i_iQuantity;
    if( iPrice > 0 )
    {
      // Verify that the player can carry more
      local ciItem = ciPlayer.GetInventoryByType( i_eWeaponType );
      if( ciItem != null )
      {
        local iNumOwned = ciItem.GetProperty( Inventory_Quantity_PropertyID, 1 );
        local ciProperty = ::rumGetPropertyAsset( Inventory_Quantity_PropertyID );
        i_iQuantity = min( i_iQuantity, ciProperty.GetMaxValue() - iNumOwned );
      }

      if( i_iQuantity > 0 )
      {
        // Does the player have enough gold to make the purchase?
        local iPlayerGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );

        // Apply discount
        iPrice -= ( iPrice * fDiscount ).tointeger();
        if( iPlayerGold >= iPrice )
        {
          // Send the buy request to the server
          local ciBroadcast = ::rumCreate( Merchant_Weapons_BroadcastID, MerchantWeaponryTransaction.Purchase,
                                           i_eWeaponType, i_iQuantity );
          ::rumSendBroadcast( ciBroadcast );
        }
        else
        {
          // Not enough gold!
          OfferService( u4_merchant_weapon_cant_pay_client_StringID );
        }
      }
      else
      {
        // Player can't carry more
        OffserService( msg_no_room_client_StringID );
      }
    }
    else
    {
      // Something is wrong with the price
      OfferService( u4_merchant_weapon_buy_no_client_StringID );
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
      local strDesc = ::rumGetString( u4_merchant_weapon_prompt_2_client_StringID );
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

      local eTokenID = ciMap.GetProperty( Merchant_Weapons_Proprietor_PropertyID, rumInvalidStringToken );
      local strProprietor = ::rumGetString( eTokenID );

      local strDesc = ::rumGetString( u4_merchant_weapon_bye_client_StringID );
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
                        ::rumGetString( u4_merchant_weapon_continue_client_StringID ) );
    }
    else
    {
      strDesc = ::rumGetString( u4_merchant_weapon_continue_client_StringID );
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
    OfferService( u4_merchant_weapon_buy_no_client_StringID );
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

    local iWeaponID = g_ciUI.m_ciGameListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    local ciPlayer = ::rumGetMainPlayer();
    local ciWeapon = ciPlayer.GetInventory( iWeaponID );
    if( !( ciWeapon instanceof rumInventory ) )
    {
      SellCanceled();
      return;
    }

    // If the weapon stacks, determine how many the player wants to sell
    if( ciWeapon.GetProperty( Inventory_Stacks_PropertyID, false ) )
    {
      ShowString( ::rumGetString( merchant_sell_quantity_prompt_client_StringID ), g_strColorTagArray.Cyan );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, SellCallbackQuantity.bindenv( this ), iWeaponID );
      g_ciUI.m_ciGameInputTextBox.Focus();
    }
    else
    {
      SellCallbackQuantity( iWeaponID, 1 );
    }
  }


  function SellCallbackQuantity( i_iWeaponID, i_iQuantity )
  {
    local ciProperty = ::rumGetPropertyAsset( Inventory_Quantity_PropertyID );

    local ciPlayer = ::rumGetMainPlayer();
    local ciWeapon = ciPlayer.GetInventory( i_iWeaponID );
    if( !( ciWeapon instanceof rumInventory ) )
    {
      SellCanceled();
      return;
    }

    if( i_iQuantity > 1 )
    {
      // Does the player own the amount they're trying to sell?
      local iNumOwned = ciWeapon.GetProperty( Inventory_Quantity_PropertyID, 1 );
      i_iQuantity = min( i_iQuantity, iNumOwned );

      // Push the amount player wants to sell to output
      ShowString( i_iQuantity + "<b>" );
    }

    local fDiscount = 0.0;

    switch( g_ciCUO.m_eVersion )
    {
      case GameType.Ultima1:
      case GameType.Ultima2:
        fDiscount = ciPlayer.GetDiscountPercent();
        break;
    }

    local iPrice = ciWeapon.GetProperty( Inventory_Price_PropertyID, 0 );
    local iValue = ( iPrice / 2.0 - ( 2.0 * fDiscount ) ).tointeger();

    local strDesc;
    local strName = ::rumGetStringByName( ciWeapon.GetName() + "_client_StringID" );

    // Merchant offer for the weapon
    if( i_iQuantity > 1 )
    {
      local iTotalValue = iValue * i_iQuantity;
      strDesc = ::rumGetString( merchant_weapon_sell_quantity_offer_client_StringID );
      strDesc = format( strDesc, strName, iValue, iTotalValue );
    }
    else
    {
      strDesc = ::rumGetString( u4_merchant_weapon_sell_offer_client_StringID );
      strDesc = format( strDesc, iValue, strName );
    }

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

    g_ciUI.m_ciGameInputTextBox.m_vPayload = { slot1 = i_iWeaponID, slot2 = i_iQuantity };
  }


  function SellCanceled( ... )
  {
    ShowString( "" );
    Ultima_ListSelectionEnd();
    OfferService( u4_merchant_weapon_sell_no_client_StringID );
  }


  function SellSelect()
  {
    // Iterate over the player's weapons and build the weapon list
    g_ciUI.m_ciGameListView.Clear();
    g_ciUI.m_ciGameListView.SetFormat( "0.05|0.5" );
    g_ciUI.m_ciGameListView.ShowPrompt( false );

    local ciPlayer = ::rumGetMainPlayer();
    local uiEquippedWeaponID = ciPlayer.GetEquippedWeaponID();

    local ciInventory = ciPlayer.GetInventory();
    local ciItem;
    while( ciItem = ciInventory.GetNextObject() )
    {
      if( null == ciItem )
      {
        continue;
      }

      local uiItemID = ciItem.GetID();

      // Add the weapon to the UI control - the key of the weapon is the weapon's UniqueID
      if( uiItemID != uiEquippedWeaponID )
      {
        local eType = ciItem.GetProperty(Inventory_Type_PropertyID, InventoryType.Standard);
        if( InventoryType.Weapon == eType )
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
      local strDesc = ::rumGetString( u4_merchant_weapon_sell_client_StringID );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

      g_ciUI.m_ciGameListView.m_funcAccept = SellCallback.bindenv( this );
      g_ciUI.m_ciGameListView.m_funcCancel = PurchaseCanceled.bindenv( this );
      g_ciUI.m_ciGameListView.SetActive( true );
      g_ciUI.m_ciGameListView.Focus();
    }
    else
    {
      OfferService( u4_merchant_weapon_sell_no_items_client_StringID );
    }
  }


  function SellVerifyCallback()
  {
    // Push the selection
    local strDesc = g_ciUI.m_ciGameInputTextBox.GetText();
    ShowString( strDesc + "<b>" );

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();
    local iWeaponID =  g_ciUI.m_ciGameInputTextBox.m_vPayload["slot1"]
    local iQuantity =  g_ciUI.m_ciGameInputTextBox.m_vPayload["slot2"]

    Ultima_ListSelectionEnd();

    if( YesNoResponse.No == eResponse )
    {
      // The player does not want to sell the weapon after all
      OfferService( u4_merchant_weapon_sell_no_client_StringID );
    }
    else if( YesNoResponse.Yes == eResponse )
    {
      // The player definitely wants to sell the weapon
      local ciBroadcast = ::rumCreate( Merchant_Weapons_BroadcastID, MerchantWeaponryTransaction.Sell, iWeaponID,
                                       iQuantity );
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
