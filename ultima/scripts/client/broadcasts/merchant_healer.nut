// Received from server when player transacts with a healer merchant
class Merchant_Healer_Broadcast extends rumBroadcast
{
  var1 = 0; // Healer transaction type
  var2 = 0; // Transaction result

  constructor( ... )
  {
    base.constructor();

    if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Healer transaction Type
      var2 = vargv[1]; // Service Type (Cure, Heal, or Resurrect)
    }
  }


  function OnRecv()
  {
    // Save a reference to this instance, otherwise it'll be garbage collected before the callback is made
    g_ciCUO.m_ciMerchantRef = this;

    local eTransactionType = var1;
    local strDesc;

    if( MerchantHealerTransaction.Greet == eTransactionType )
    {
      PlayMerchantMusic();

      g_ciUI.m_ePreviousStatPage = g_ciUI.m_eCurrentStatPage;

      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();

      local eTokenID = ciMap.GetProperty( Merchant_Healer_Name_PropertyID, rumInvalidStringToken );
      local strShopName = ::rumGetString( eTokenID );

      eTokenID = ciMap.GetProperty( Merchant_Healer_Proprietor_PropertyID, rumInvalidStringToken );
      local strProprietor = ::rumGetString( eTokenID );

      strDesc = format( "%s %s.<b><b>%s %s",
                        ::rumGetString( u4_merchant_healer_greet_client_StringID ),
                        strShopName,
                        strProprietor,
                        ::rumGetString( u4_merchant_healer_greet_2_client_StringID ) );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      g_ciUI.m_ciYesNoListView.Clear();
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                         rumKeypress.KeyY() );
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                         rumKeypress.KeyN() );

      g_ciUI.m_ciYesNoListView.m_funcAccept = HealingInterestCallback.bindenv( this );
      g_ciUI.m_ciYesNoListView.m_funcCancel = TransactionCanceled.bindenv( this );
      g_ciUI.m_ciYesNoListView.SetActive( true );
      g_ciUI.m_ciYesNoListView.Focus();
    }
    else if( MerchantHealerTransaction.Purchase == eTransactionType )
    {
      local eStringID = var2;
      OfferService( eStringID );
    }
    else if( MerchantHealerTransaction.ServerTerminated == eTransactionType )
    {
      local eStringID = var2;
      strDesc = format( "<b>%s<b>", ::rumGetString( eStringID ) );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      // Force transaction quit
      EndTransaction( false /* do not inform server */, strDesc );
    }
  }


  function DonateBloodCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry()
    ShowString( strResponse + "<b>" );

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    local bResponse = ( YesNoResponse.Yes == eResponse );
    if( bResponse )
    {
      local strDesc = ::rumGetString( u4_merchant_healer_donate_yes_client_StringID );
      ShowString( strDesc + "<b>", g_strColorTagArray.Cyan );
    }

    // Send the donation response to the server
    local ciBroadcast = ::rumCreate( Merchant_Healer_BroadcastID,
                                     MerchantHealerTransaction.DonateBlood, bResponse );
    ::rumSendBroadcast( ciBroadcast );

    EndTransaction( true /* inform server */ );
  }


  function EndTransaction( ... )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();

    local eTokenID = ciMap.GetProperty( Merchant_Healer_Proprietor_PropertyID, rumInvalidStringToken );
    local strProprietor = ::rumGetString( eTokenID );

    local strDesc = format( "%s %s<b>", strProprietor, ::rumGetString( u4_merchant_healer_bye_client_StringID ) );
    ShowString( strDesc, g_strColorTagArray.Cyan );

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


  function HealingInterestCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    if( YesNoResponse.Yes == eResponse )
    {
      // Player is interested
      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();

      switch( g_ciCUO.m_eVersion )
      {
        case GameType.Ultima1: g_ciUI.m_eCurrentStatPage = U1_StatPage.Main; break;
        case GameType.Ultima2: g_ciUI.m_eCurrentStatPage = U2_StatPage.Main; break;
        case GameType.Ultima3: g_ciUI.m_eCurrentStatPage = U3_StatPage.Main; break;
        case GameType.Ultima4: g_ciUI.m_eCurrentStatPage = U4_StatPage.Main; break;
      }

      Ultima_Stat_Update();

      local eTokenID = ciMap.GetProperty( Merchant_Healer_Proprietor_PropertyID, rumInvalidStringToken );
      local strProprietor = ::rumGetString( eTokenID );

      local strDesc = format( "%s %s<b><b>%s",
                              strProprietor,
                              ::rumGetString( u4_merchant_healer_desc_client_StringID ),
                              ::rumGetString( u4_merchant_healer_prompt_3_client_StringID ) );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      // Build the healer services list
      g_ciUI.m_ciGameListView.Clear();
      g_ciUI.m_ciGameListView.DisableMultiSelect();
      g_ciUI.m_ciGameListView.SetFormat( "0.05|0.15" );
      g_ciUI.m_ciGameListView.ShowPrompt( false );

      local eServiceArray = ::rumGetDataTableColumn( merchant_healer_DataTableID, 0 );
      local eStringArray = ::rumGetDataTableColumn( merchant_healer_DataTableID, 2 );

      for( local iIndex = 0; iIndex < eServiceArray.len(); ++iIndex )
      {
        local strAlpha = ::rumGetStringByName( format( "alpha%02d_client_StringID", iIndex ) );
        local iKey = rumKeypress.KeyA() + iIndex;
        g_ciUI.m_ciGameListView.SetEntry( eServiceArray[iIndex],
                                          strAlpha + "|" + ::rumGetString( eStringArray[iIndex] ),
                                          iKey );
      }

      CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

      g_ciUI.m_ciGameListView.m_funcAccept = HealingSelectCallback.bindenv( this );
      g_ciUI.m_ciGameListView.m_funcCancel = TransactionCanceled.bindenv( this );
      g_ciUI.m_ciGameListView.m_funcIndexChanged = SelectionChanged.bindenv( this );
      g_ciUI.m_ciGameListView.SetActive( true );
      g_ciUI.m_ciGameListView.Focus();

      return;
    }
    else
    {
      if( GameType.Ultima4 == g_ciCUO.m_eVersion )
      {
        // Ask the player if they can donate blood (providing they meet certain conditions)
        local ciPlayer = ::rumGetMainPlayer();

        local iLevel = ciPlayer.GetProperty( U4_Level_PropertyID, 1 );
        if( iLevel >= 4 )
        {
          local iHitpoints = ciPlayer.GetProperty( U4_Hitpoints_PropertyID, 0 );
          if( iHitpoints > 100 )
          {
            local strDesc = ::rumGetString( u4_merchant_healer_donate_client_StringID );
            ShowString( strDesc, g_strColorTagArray.Cyan );

            g_ciUI.m_ciYesNoListView.Clear();
            g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                               rumKeypress.KeyY() );
            g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                               rumKeypress.KeyN() );

            g_ciUI.m_ciYesNoListView.m_funcAccept = DonateBloodCallback.bindenv( this );
            g_ciUI.m_ciYesNoListView.m_funcCancel = DonateBloodCallback.bindenv( this );
            g_ciUI.m_ciYesNoListView.SetActive( true );
            g_ciUI.m_ciYesNoListView.Focus();

            return;
          }
        }
      }
    }

    EndTransaction( true /* inform server */ );
  }


  function HealingSelectCallback()
  {
    // Push the player's selection to output
    local strService = g_ciUI.m_ciGameInputTextBox.GetText();
    ShowString( strService + "<b>" );

    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();

    local eStringID = rumInvalidStringToken;
    local fDiscount = 0.0;

    switch( g_ciCUO.m_eVersion )
    {
      case GameType.Ultima1: fDiscount = ciPlayer.GetDiscountPercent(); break;
      case GameType.Ultima2: fDiscount = ciPlayer.GetDiscountPercent(); break;
    }

    local eService = g_ciUI.m_ciGameListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    local iPriceArray = ::rumGetDataTableColumn( merchant_healer_DataTableID, 1 );
    local iPrice = iPriceArray[eService];
    iPrice -= ( iPrice * fDiscount ).tointeger();
    if( iPrice > 0 )
    {
      local bNeedsService = false;

      local eAcceptArray = ::rumGetDataTableColumn( merchant_healer_DataTableID, 3 );
      local eRefuseArray = ::rumGetDataTableColumn( merchant_healer_DataTableID, 4 );

      switch( eService )
      {
        case MerchantHealerService.Cure:
          if( ciPlayer.IsPoisoned() )
          {
            eStringID = eAcceptArray[eService];
            bNeedsService = true;
          }
          else
          {
            eStringID = eRefuseArray[eService];
          }
          break;

        case MerchantHealerService.Heal:
          local iHitpoints = ciPlayer.GetVersionedProperty( g_eHitpointsPropertyVersionArray );
          if( iHitpoints < ciPlayer.GetMaxHitpoints() )
          {
            eStringID = eAcceptArray[eService];
            bNeedsService = true;
          }
          else
          {
            eStringID = eRefuseArray[eService];;
          }
          break;

        case MerchantHealerService.Resurrect:
          local bBound = ciPlayer.GetVersionedProperty( g_eSpiritBoundPropertyVersionArray );
          if( !bBound )
          {
            eStringID = eAcceptArray[eService];
            bNeedsService = true;
          }
          else
          {
            eStringID = eRefuseArray[eService];
          }
          break;

        case MerchantHealerService.Potions:
          bNeedsService = true;
          eStringID = eAcceptArray[eService];
          break;
      }

      if( bNeedsService )
      {
        if( MerchantHealerService.Potions == eService )
        {
          switch( g_ciCUO.m_eVersion )
          {
            case GameType.Ultima1: g_ciUI.m_eCurrentStatPage = U1_StatPage.Equipment; break;
            case GameType.Ultima2: g_ciUI.m_eCurrentStatPage = U2_StatPage.Equipment; break;
            case GameType.Ultima3: g_ciUI.m_eCurrentStatPage = U3_StatPage.Equipment; break;
            case GameType.Ultima4: g_ciUI.m_eCurrentStatPage = U4_StatPage.Equipment; break;
          }
        }
        else
        {
          switch( g_ciCUO.m_eVersion )
          {
            case GameType.Ultima1: g_ciUI.m_eCurrentStatPage = U1_StatPage.Main; break;
            case GameType.Ultima2: g_ciUI.m_eCurrentStatPage = U2_StatPage.Main; break;
            case GameType.Ultima3: g_ciUI.m_eCurrentStatPage = U3_StatPage.Main; break;
            case GameType.Ultima4: g_ciUI.m_eCurrentStatPage = U4_StatPage.Main; break;
          }
        }

        Ultima_Stat_Update();

        // Embed the price for the service and format the response
        local strDesc = format( ::rumGetString( eStringID ), iPrice );
        strDesc = format( "%s<b><b>%s", strDesc, ::rumGetString( u4_merchant_healer_prompt_client_StringID ) );
        ShowString( strDesc, g_strColorTagArray.Cyan );

        g_ciUI.m_ciYesNoListView.Clear();
        g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                           rumKeypress.KeyY() );
        g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                           rumKeypress.KeyN() );

        g_ciUI.m_ciYesNoListView.m_funcAccept = PayResponseCallback.bindenv( this );
        g_ciUI.m_ciYesNoListView.m_funcCancel = PurchaseCanceled.bindenv( this );
        g_ciUI.m_ciYesNoListView.SetActive( true );
        g_ciUI.m_ciYesNoListView.Focus();

        g_ciUI.m_ciGameInputTextBox.m_vPayload = eService;
      }
      else
      {
        OfferService( eStringID );
      }
    }
    else
    {
      // Something went wrong with pricing
      EndTransaction( true /* inform server */ );
    }
  }


  function OfferService( ... )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    local strDesc;

    if( 1 == vargv.len() && vargv[0] )
    {
      strDesc = format( "%s<b><b>%s",
                        ::rumGetString( vargv[0] ),
                        ::rumGetString( u4_merchant_healer_prompt_2_client_StringID ) );
    }
    else
    {
      strDesc = ::rumGetString( u4_merchant_healer_prompt_2_client_StringID );
    }

    ShowString( strDesc, g_strColorTagArray.Cyan );

    g_ciUI.m_ciYesNoListView.Clear();
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                       rumKeypress.KeyY() );
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                       rumKeypress.KeyN() );

    g_ciUI.m_ciYesNoListView.m_funcAccept = HealingInterestCallback.bindenv( this );
    g_ciUI.m_ciYesNoListView.m_funcCancel = TransactionCanceled.bindenv( this );
    g_ciUI.m_ciYesNoListView.SetActive( true );
    g_ciUI.m_ciYesNoListView.Focus();
  }


  function PayResponseCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();
    local eService = g_ciUI.m_ciGameInputTextBox.m_vPayload;

    Ultima_ListSelectionEnd();

    if( YesNoResponse.Yes == eResponse )
    {
      // Can the player afford to pay?
      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();
      local iPlayerGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );

      local fDiscount = 0.0;

      switch( g_ciCUO.m_eVersion )
      {
        case GameType.Ultima1: fDiscount = ciPlayer.GetDiscountPercent(); break;
        case GameType.Ultima2: fDiscount = ciPlayer.GetDiscountPercent(); break;
      }

      local iPriceArray = ::rumGetDataTableColumn( merchant_healer_DataTableID, 1 );
      local iPrice = iPriceArray[eService];
      iPrice -= ( iPrice * fDiscount ).tointeger();
      if( iPrice > 0 )
      {
        if( iPrice <= iPlayerGold )
        {
          // Send the heal request to the server
          local ciBroadcast = ::rumCreate( Merchant_Healer_BroadcastID, MerchantHealerTransaction.Purchase, eService );
          ::rumSendBroadcast( ciBroadcast );
        }
        else
        {
          OfferService( u4_merchant_healer_cant_pay_client_StringID );
        }
      }
      else
      {
        // Something went wrong with pricing
        EndTransaction( true /* inform server */ );
      }
    }
    else
    {
      OfferService( u4_merchant_healer_wont_pay_client_StringID );
    }
  }


  function PurchaseCanceled()
  {
    ShowString( "" );
    Ultima_ListSelectionEnd();
    OfferService( u4_merchant_healer_wont_pay_client_StringID );
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
