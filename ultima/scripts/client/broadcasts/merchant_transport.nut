// Received from server when player transacts with an transport merchant
class Merchant_Transport_Broadcast extends rumBroadcast
{
  var1 = 0; // Transport transaction type
  var2 = 0;

  constructor( ... )
  {
    base.constructor();

    if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Transport transaction type
      var2 = vargv[1]; // Transport class to buy
    }
  }


  function OnRecv()
  {
    // Save a reference to this instance, otherwise it'll be garbage collected before the callback is made
    g_ciCUO.m_ciMerchantRef = this;

    local eTransactionType = var1;

    if( MerchantTransportTransaction.Greet == eTransactionType )
    {
      PlayMerchantMusic();

      // Show the relevant transport stat page
      g_ciUI.m_ePreviousStatPage = g_ciUI.m_eCurrentStatPage;

      if( GameType.Ultima1 == g_ciCUO.m_eVersion )
      {
        g_ciUI.m_eCurrentStatPage = U1_StatPage.Transports;
      }

      Ultima_Stat_Update();

      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();

      local eTokenID = ciMap.GetProperty( Merchant_Transport_Name_PropertyID, rumInvalidStringToken );
      local strShopName = ::rumGetString( eTokenID );

      local strDesc = format( "%s %s.<b>", ::rumGetString( merchant_transport_greet_client_StringID ), strShopName );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      BuySelect();
    }
    else if( MerchantTransportTransaction.Purchase == eTransactionType )
    {
      local strDesc = var2;
      OfferService( strDesc );
    }
    else if( MerchantTransportTransaction.ServerTerminated == eTransactionType )
    {
      local strReason = var2;
      EndTransaction( false /* do not inform server */, strReason );
    }
  }


  function BuySelect()
  {
    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();

    local eInventoryArray = null;

    // Determine the data table row
    local eMapArray = ::rumGetDataTableColumn( merchant_transports_DataTableID, 0 );
    local iRow = eMapArray.find( ciMap.GetAssetID() );
    if( iRow != null )
    {
      // Fetch the row and slice off the map column
      eInventoryArray = ::rumGetDataTableRow( merchant_transports_DataTableID, iRow ).slice( 1 );
    }

    // Transport merchants only sell up to 6 items
    local iMaxItems = 6;
    local iIndex = eInventoryArray.find( rumInvalidAssetID );
    local iNumAvailable = ( null == iIndex ) ? iMaxItems : iIndex;
    eInventoryArray.resize( iNumAvailable );

    // Build the transport list
    g_ciUI.m_ciGameListView.Clear();
    g_ciUI.m_ciGameListView.SetFormat( "0.05|0.2|0.75" );
    g_ciUI.m_ciGameListView.ShowPrompt( false );

    local eTransportArray = null;
    local fDiscount = ciPlayer.GetDiscountPercent();

    // Visit each transport
    foreach( iIndex, eTransportType in eInventoryArray )
    {
      local ciTransport = ::rumGetWidgetAsset( eTransportType );
      if( ciTransport != null )
      {
        local strDesc = ::rumGetStringByName( ciTransport.GetName() + "_Widget_client_StringID" );
        strDesc = strDesc.slice( 0, 1 ).toupper() + strDesc.slice( 1, strDesc.len() );
        local strAlpha = ::rumGetStringByName( format( "alpha%02d_client_StringID", iIndex ) );

        local iPrice = ciTransport.GetProperty( Merchant_Price_PropertyID, 0 );
        iPrice = iPrice - ( iPrice * fDiscount ).tointeger();
        iPrice = max( iPrice, 1 );

        if( U1_Shuttle_Widget == eTransportType )
        {
          // If the player already has a shuttle pass, then the ride is free
          if( ciPlayer.GetProperty( U1_Shuttle_Pass_PropertyID, false ) )
          {
            iPrice = 0;
          }
        }

        g_ciUI.m_ciGameListView.SetEntry( iIndex, strAlpha + "|" + strDesc + "|" + iPrice,
                                          rumKeypress.KeyA() + iIndex );
      }
    }

    local strDesc = ::rumGetString( merchant_transport_prompt_client_StringID );
    ShowString( strDesc, g_strColorTagArray.Cyan );

    CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

    g_ciUI.m_ciGameListView.m_funcAccept = BuySelectCallback.bindenv( this );
    g_ciUI.m_ciGameListView.m_funcCancel = PurchaseCanceled.bindenv( this );
    g_ciUI.m_ciGameListView.m_funcIndexChanged = SelectionChanged.bindenv( this );
    g_ciUI.m_ciGameListView.SetActive( true );
    g_ciUI.m_ciGameListView.Focus();
  }


  function BuySelectCallback()
  {
    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();
    local iTransportKey = g_ciUI.m_ciGameListView.GetSelectedKey();
    local eInventoryArray = null;

    // Determine the data table row
    local eMapArray = ::rumGetDataTableColumn( merchant_transports_DataTableID, 0 );
    local iRow = eMapArray.find( ciMap.GetAssetID() );
    if( iRow != null )
    {
      // Fetch the row and slice off the map column
      eInventoryArray = ::rumGetDataTableRow( merchant_transports_DataTableID, iRow ).slice( 1 );
    }

    local eTranportType = eInventoryArray[iTransportKey];
    local ciAsset = ::rumGetWidgetAsset( eTranportType );

    // Push the player's selection to output
    local strDesc = g_ciUI.m_ciGameInputTextBox.GetText();
    ShowString( strDesc + "<b>" );

    Ultima_ListSelectionEnd();

    // Does the player already own the transport?
    if( ( ( U1_Aircar_WidgetID == eTranportType ) && ( ciPlayer.GetProperty( U1_Aircar_PropertyID, false ) ) ) ||
        ( ( U1_Horse_WidgetID == eTranportType )  && ( ciPlayer.GetProperty( U1_Horse_PropertyID, false ) ) )  ||
        ( ( U1_Cart_WidgetID == eTranportType )   && ( ciPlayer.GetProperty( U1_Cart_PropertyID, false ) ) )   ||
        ( ( U1_Raft_WidgetID == eTranportType )   && ( ciPlayer.GetProperty( U1_Raft_PropertyID, false ) ) ) )
    {
      // Already owned
      OfferService( u4_merchant_rations_full_client_StringID );
      return;
    }

    strDesc = ::rumGetString( merchant_transport_buy_prompt_client_StringID );

    ShowString( strDesc, g_strColorTagArray.Cyan );

    g_ciUI.m_ciYesNoListView.Clear();
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                       rumKeypress.KeyY() );
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                       rumKeypress.KeyN() );

    g_ciUI.m_ciYesNoListView.m_funcAccept = BuyVerifyCallback.bindenv( this );
    g_ciUI.m_ciYesNoListView.m_funcCancel = TransactionCanceled.bindenv( this );
    g_ciUI.m_ciYesNoListView.SetActive( true );
    g_ciUI.m_ciYesNoListView.Focus();

    g_ciUI.m_ciGameInputTextBox.m_vPayload = ciAsset;
  }


  function BuyVerifyCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();
    local ciAsset = g_ciUI.m_ciGameInputTextBox.m_vPayload;

    Ultima_ListSelectionEnd();

    if( YesNoResponse.Yes == eResponse )
    {
      // Does the player have enough gold to make the purchase?
      local ciPlayer = ::rumGetMainPlayer();
      local iPlayerGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );

      if( ciAsset != null )
      {
        local strDesc = ::rumGetStringByName( ciAsset.GetName() + "_Widget_client_StringID" );
        strDesc = strDesc.slice( 0, 1 ).toupper() + strDesc.slice( 1, strDesc.len() );

        local fDiscount = ciPlayer.GetDiscountPercent();
        local iPrice = ciAsset.GetProperty( Merchant_Price_PropertyID, 0 );
        iPrice = iPrice - ( iPrice * fDiscount ).tointeger();
        iPrice = max( iPrice, 1 );

        if( iPlayerGold >= iPrice )
        {
          // Send the buy request to the server
          local ciBroadcast = ::rumCreate( Merchant_Transport_BroadcastID,
                                           MerchantTransportTransaction.Purchase, ciAsset.GetAssetID() );
          ::rumSendBroadcast( ciBroadcast );
        }
        else
        {
          // Not enough gold!
          OfferService( merchant_transport_cant_pay_client_StringID );
        }
      }
      else
      {
        OfferService( merchant_transport_buy_no_client_StringID );
      }
    }
    else
    {
      // The player does not want the transport after all
      OfferService( merchant_transport_buy_no_client_StringID );
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
      BuySelect();
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
      strDesc = ::rumGetString( merchant_transport_bye_client_StringID );
    }

    ShowString( format( "%s<b>", strDesc ), g_strColorTagArray.Cyan );

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

    if( 1 == vargv.len() && vargv[0] )
    {
      strDesc = format( "%s<b><b>%s",
                        ::rumGetString( vargv[0] ),
                        ::rumGetString( merchant_transport_continue_client_StringID ) );
    }
    else
    {
      strDesc = ::rumGetString( merchant_transport_continue_client_StringID );
    }

    ShowString( strDesc, g_strColorTagArray.Cyan );

    // See if the player wants anything else
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


  function PurchaseCanceled()
  {
    ShowString( "" );
    Ultima_ListSelectionEnd();
    OfferService();
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


  function TransactionCanceled()
  {
    ShowString( "" );
    Ultima_ListSelectionEnd();
    EndTransaction( true /* inform server */ );
  }
}
