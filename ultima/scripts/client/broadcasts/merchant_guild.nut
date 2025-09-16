// Received from server when player transacts with a guild merchant
class Merchant_Guild_Broadcast extends rumBroadcast
{
  var1 = 0; // Guild transaction type
  var2 = 0; // Response


  constructor( ... )
  {
    base.constructor();

    if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Guild transaction type
      var2 = vargv[1]; // Purchase type
    }
  }


  function OnRecv()
  {
    local eTransactionType = var1;
    local strDesc;

    // Save a reference to this instance, otherwise it'll be garbage collected before the callback is made
    g_ciCUO.m_ciMerchantRef = this;

    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();

    if( MerchantGuildTransaction.Greet == eTransactionType )
    {
      // Show the relevant equipment stat page
      g_ciUI.m_ePreviousStatPage = g_ciUI.m_eCurrentStatPage;
      g_ciUI.m_eCurrentStatPage = ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ? U4_StatPage.Equipment
                                                                             : U3_StatPage.Equipment;
      Ultima_Stat_Update();

      PlayMerchantMusic();

      local eTokenID = ciMap.GetProperty( Merchant_Guild_Name_PropertyID, rumInvalidStringToken );
      local strShopName = ::rumGetString( eTokenID );

      eTokenID = ciMap.GetProperty( Merchant_Guild_Proprietor_PropertyID, rumInvalidStringToken );
      local strProprietor = ::rumGetString( eTokenID );

      // Greet the player and question interest
      strDesc = format( "%s %s?<b><b>%s %s %s<b><b>%s",
                        ::rumGetString( u4_merchant_guild_greet_client_StringID ),
                        strProprietor,
                        strProprietor,
                        ::rumGetString( u4_merchant_guild_greet_2_client_StringID ),
                        strShopName,
                        ::rumGetString( u4_merchant_guild_prompt_client_StringID ) );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      g_ciUI.m_ciYesNoListView.Clear();
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                         rumKeypress.KeyY() );
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                         rumKeypress.KeyN() );

      g_ciUI.m_ciYesNoListView.m_funcAccept = InterestedCallback.bindenv( this );
      g_ciUI.m_ciYesNoListView.m_funcCancel = TransactionCanceled.bindenv( this );
      g_ciUI.m_ciYesNoListView.SetActive( true );
      g_ciUI.m_ciYesNoListView.Focus();
    }
    else if( MerchantGuildTransaction.Purchase == eTransactionType )
    {
      local eTokenID = ciMap.GetProperty( Merchant_Guild_Proprietor_PropertyID, rumInvalidStringToken );
      local strProprietor = ::rumGetString( eTokenID );

      strDesc = format( "%s<b><b>%s %s",
                        ::rumGetString( u4_merchant_guild_buy_ok_client_StringID ),
                        strProprietor,
                        ::rumGetString( u4_merchant_guild_prompt_3_client_StringID ) );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      g_ciUI.m_ciYesNoListView.Clear();
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                         rumKeypress.KeyY() );
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                         rumKeypress.KeyN() );

      g_ciUI.m_ciYesNoListView.m_funcAccept = InterestedCallback.bindenv( this );
      g_ciUI.m_ciYesNoListView.m_funcCancel = TransactionCanceled.bindenv( this );
      g_ciUI.m_ciYesNoListView.SetActive( true );
      g_ciUI.m_ciYesNoListView.Focus();
    }
    else if( MerchantGuildTransaction.ServerTerminated == eTransactionType )
    {
      local strReason = var2;
      EndTransaction( false /* do not inform server */, strReason );
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

      local eTokenID = ciMap.GetProperty( Merchant_Guild_Proprietor_PropertyID, rumInvalidStringToken );
      local strProprietor = ::rumGetString( eTokenID );

      strDesc = format( "%s %s", strProprietor, ::rumGetString( u4_merchant_guild_bye_client_StringID ) );
    }

    ShowString( strDesc + "<b>", g_strColorTagArray.Cyan );

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


  function InterestedCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    if( YesNoResponse.Yes == eResponse )
    {
      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();

      local eTokenID = ciMap.GetProperty( Merchant_Guild_Proprietor_PropertyID, rumInvalidStringToken );
      local strProprietor = ::rumGetString( eTokenID );

      local strDesc = format( "%s %s<b><b>%s<b><b>%s",
                              strProprietor,
                              ::rumGetString( u4_merchant_guild_yes_client_StringID ),
                              ::rumGetString( u4_merchant_guild_desc_client_StringID ),
                              ::rumGetString( u4_merchant_guild_prompt4_client_StringID ) );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      // Build the guild products list
      g_ciUI.m_ciGameListView.Clear();
      g_ciUI.m_ciGameListView.DisableMultiSelect();
      g_ciUI.m_ciGameListView.SetFormat( "0.05|0.15" );
      g_ciUI.m_ciGameListView.ShowPrompt( false );

      local bSextantAccessible = ciPlayer.GetProperty( U4_Sextant_Accessible_PropertyID, false );

      local eTableId = merchant_guild_u4_DataTableID;
      if( g_ciCUO.m_eVersion == GameType.Ultima3 )
      {
        eTableId = merchant_guild_u3_DataTableID;
      }

      local eServiceArray = ::rumGetDataTableColumn( eTableId, 0 );
      local eNameArray = ::rumGetDataTableColumn( eTableId, 3 );
      for( local iIndex = 0; iIndex < eServiceArray.len(); ++iIndex )
      {
        local strAlpha = ::rumGetStringByName( format( "alpha%02d_client_StringID", iIndex ) );
        local iKey = rumKeypress.KeyA() + iIndex;
        if( bSextantAccessible || eServiceArray[iIndex] < MerchantGuildProducts.Sextant )
        {
          g_ciUI.m_ciGameListView.SetEntry( eServiceArray[iIndex],
                                            strAlpha + "|" + ::rumGetString( eNameArray[iIndex] ),
                                            iKey );
        }
      }

      CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

      g_ciUI.m_ciGameListView.m_funcAccept = ItemSelectCallback.bindenv( this );
      g_ciUI.m_ciGameListView.m_funcCancel = TransactionCanceled.bindenv( this );
      g_ciUI.m_ciGameListView.m_funcIndexChanged = SelectionChanged.bindenv( this );
      g_ciUI.m_ciGameListView.SetActive( true );
      g_ciUI.m_ciGameListView.Focus();
    }
    else
    {
      EndTransaction( true /* inform server */ );
    }
  }


  function ItemSelectCallback()
  {
    // Push the player's selection to output
    local strItem = g_ciUI.m_ciGameInputTextBox.GetText();
    if( strItem == "" )
    {
      strItem = ::rumGetString( u4_item_sextant_client_StringID );
    }

    ShowString( strItem + "<b>" );

    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();

    local eSelection = g_ciUI.m_ciGameListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    local eTableId = merchant_guild_u4_DataTableID;
    if( g_ciCUO.m_eVersion == GameType.Ultima3 )
    {
      eTableId = merchant_guild_u3_DataTableID;
    }

    local iPriceArray = ::rumGetDataTableColumn( eTableId, 1 );
    local iPrice = iPriceArray[eSelection];

    local iQuantityArray = ::rumGetDataTableColumn( eTableId, 2 );
    local iAmount = iQuantityArray[eSelection];

    local eDescArray = ::rumGetDataTableColumn( eTableId, 4 );
    local strDesc;
    if( iAmount > 1 )
    {
      strDesc = format( ::rumGetString( eDescArray[eSelection] ), iAmount, iPrice );
    }
    else
    {
      strDesc = format( ::rumGetString( eDescArray[eSelection] ), iPrice );
    }
    strDesc += "<b><b>";
    strDesc += ::rumGetString( u4_merchant_guild_prompt_2_client_StringID );
    ShowString( strDesc, g_strColorTagArray.Cyan );

    g_ciUI.m_ciYesNoListView.Clear();
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                       rumKeypress.KeyY() );
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                       rumKeypress.KeyN() );

    g_ciUI.m_ciYesNoListView.m_funcAccept = PurchaseCallback.bindenv( this );
    g_ciUI.m_ciYesNoListView.m_funcCancel = PurchaseCanceled.bindenv( this );
    g_ciUI.m_ciYesNoListView.SetActive( true );
    g_ciUI.m_ciYesNoListView.Focus();

    g_ciUI.m_ciGameInputTextBox.m_vPayload = eSelection;
  }


  function OfferService( ... )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();

    local eTokenID = ciMap.GetProperty( Merchant_Guild_Proprietor_PropertyID, rumInvalidStringToken );
    local strProprietor = ::rumGetString( eTokenID );

    local strDesc;

    if( ( 1 == vargv.len() ) && vargv[0] )
    {
      strDesc = format( "%s<b><b>%s %s",
                        ::rumGetString( vargv[0] ), strProprietor,
                        ::rumGetString( u4_merchant_guild_prompt_3_client_StringID ) );
    }
    else
    {
      strDesc = format( "%s %s", strProprietor, ::rumGetString( u4_merchant_guild_prompt_3_client_StringID ) );
      strDesc = ::rumGetString( u4_merchant_guild_prompt_3_client_StringID );
    }

    ShowString( strDesc, g_strColorTagArray.Cyan );

    g_ciUI.m_ciYesNoListView.Clear();
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                       rumKeypress.KeyY() );
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                       rumKeypress.KeyN() );

    g_ciUI.m_ciYesNoListView.m_funcAccept = InterestedCallback.bindenv( this );
    g_ciUI.m_ciYesNoListView.m_funcCancel = TransactionCanceled.bindenv( this );
    g_ciUI.m_ciYesNoListView.SetActive( true );
    g_ciUI.m_ciYesNoListView.Focus();
  }


  function PurchaseCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();
    local eSelection = g_ciUI.m_ciGameInputTextBox.m_vPayload;

    Ultima_ListSelectionEnd();

    if( YesNoResponse.Yes == eResponse )
    {
      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();

      local eTableId = merchant_guild_u4_DataTableID;
      if( g_ciCUO.m_eVersion == GameType.Ultima3 )
      {
        eTableId = merchant_guild_u3_DataTableID;
      }

      local iPriceArray = ::rumGetDataTableColumn( eTableId, 1 );

      // Player wants to buy, do they have enough gold?
      local iPlayerGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
      local iPrice = iPriceArray[eSelection];
      if( iPlayerGold >= iPrice )
      {
        local ciBroadcast = ::rumCreate( Merchant_Guild_BroadcastID, MerchantGuildTransaction.Purchase, eSelection );
        ::rumSendBroadcast( ciBroadcast );
      }
      else
      {
        EndTransaction( true /* inform server */, u4_merchant_guild_cant_pay_client_StringID );
      }
    }
    else
    {
      OfferService( u4_merchant_guild_buy_canc_client_StringID );
    }
  }


  function PurchaseCanceled()
  {
    ShowString( "" );
    Ultima_ListSelectionEnd();
    OfferService( u4_merchant_guild_buy_canc_client_StringID );
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
