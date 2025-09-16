// Received from server when player transacts with a rations merchant
class Merchant_Rations_Broadcast extends rumBroadcast
{
  var1 = 0; // Ration transaction type
  var2 = 0; // Transaction result


  constructor( ... )
  {
    base.constructor();

    if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Ration transaction Type
      var2 = vargv[1]; // Num ration packs
    }
  }


  function OnRecv()
  {
    local eTransactionType = var1;
    local strDesc;

    // Save a reference to this instance, otherwise it'll be garbage collected before the callback is made
    g_ciCUO.m_ciMerchantRef = this;

    if( MerchantRationTransaction.Greet == eTransactionType )
    {
      PlayMerchantMusic();

      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();

      local eTokenID = ciMap.GetProperty( Merchant_Rations_Name_PropertyID, rumInvalidStringToken );
      local strShopName = ::rumGetString( eTokenID );

      eTokenID = ciMap.GetProperty( Merchant_Rations_Proprietor_PropertyID, rumInvalidStringToken );
      local strProprietor = ::rumGetString( eTokenID );

      // Greet the player and question interest
      strDesc = format( "%s %s.<b><b>%s %s<b><b>%s",
                        ::rumGetString( u4_merchant_rations_welcome_client_StringID ),
                        strShopName,
                        strProprietor,
                        ::rumGetString( u4_merchant_rations_greet_client_StringID ),
                        ::rumGetString( u4_merchant_rations_prompt_client_StringID ) );
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
    else if( MerchantRationTransaction.Purchase == eTransactionType )
    {
      local strResponse = var2;
      strDesc = format( "%s<b><b>%s",
                        ::rumGetStringByName( strResponse + "_client_StringID" ),
                        ::rumGetString( u4_merchant_rations_prompt_3_client_StringID ) );
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
    else if( MerchantRationTransaction.ServerTerminated == eTransactionType )
    {
      local strReason = var2;
      EndTransaction( false /* do not inform server */, strReason );
    }
  }


  function AmountSpecified( i_iAmount )
  {
    if( null == i_iAmount )
    {
      // Player no longer wants the rations
      ShowString( "" );
      EndTransaction( true /* inform server */, ::rumGetString( u4_merchant_rations_buy_cancel_client_StringID ) );
      return;
    }

    ShowString( i_iAmount.tostring() + "<b>" );

    g_ciUI.m_ciGameInputTextBox.Clear();

    if( i_iAmount > 0 )
    {
      local ciPlayer = ::rumGetMainPlayer();
      local iCurrentFood = ciPlayer.GetVersionedProperty( g_eFoodPropertyVersionArray );
      local iFreeFoodSlots =
        ::rumGetMaxPropertyValue( g_eFoodPropertyVersionArray[g_ciCUO.m_eVersion] ) - iCurrentFood;
      if( iFreeFoodSlots > 0 )
      {
        local ciMap = ciPlayer.GetMap();
        local iPackSize = ciMap.GetProperty( Merchant_Rations_Pack_Size_PropertyID, 0 );
        if( iPackSize > 0 )
        {
          local fNumPacks = iFreeFoodSlots / iPackSize;
          local iNumPacks = fNumPacks.tointeger();
          if( iNumPacks > 0 )
          {
            // Can the player afford the purchase?
            local fDiscount = 0.0;

            switch( g_ciCUO.m_eVersion )
            {
              case GameType.Ultima1: fDiscount = ciPlayer.GetDiscountPercent(); break;
              case GameType.Ultima2: fDiscount = ciPlayer.GetDiscountPercent(); break;
            }

            local iPackPrice = ciMap.GetProperty( Merchant_Rations_Price_PropertyID, 0 );
            iPackPrice -= ( iPackPrice * fDiscount ).tointeger();
            if( iPackPrice > 0 )
            {
              local iPurchasePrice = i_iAmount * iPackPrice;
              local iPlayerGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
              if( iPlayerGold >= iPurchasePrice )
              {
                // Purchase the designated amount of rations
                local ciBroadcast = ::rumCreate( Merchant_Rations_BroadcastID,
                                                 MerchantRationTransaction.Purchase, i_iAmount );
                ::rumSendBroadcast( ciBroadcast );
              }
              else
              {
                // Player cannot afford the purchase
                EndTransaction( true /* inform server */,
                                ::rumGetString( u4_merchant_rations_cant_pay_client_StringID ) );
              }
            }
            else
            {
              // Price invalid
              EndTransaction( true /* inform server */ );
            }
          }
          else
          {
            // Player doesn't need the full pack
            EndTransaction( true /* inform server */, ::rumGetString( u4_merchant_rations_full_client_StringID ) );
          }
        }
        else
        {
          // Pack size invalid
          EndTransaction( true /* inform server */ );
        }
      }
      else
      {
        // Player doesn't need food
        EndTransaction( true /* inform server */, ::rumGetString( u4_merchant_rations_full_client_StringID ) );
      }
    }
    else
    {
      // Player is not interested in purchasing rations
      EndTransaction( true /* inform server */, ::rumGetString( u4_merchant_rations_buy_cancel_client_StringID ) );
    }
  }


  function EndTransaction( ... )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    local strDesc;

    if( vargv.len() > 1 && vargv[1] )
    {
      strDesc = format( "%s<b><b>%s", vargv[1], ::rumGetString( u4_merchant_rations_bye_client_StringID ) );
    }
    else
    {
      strDesc = ::rumGetString( u4_merchant_rations_bye_client_StringID );
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

    g_ciCUO.m_ciMerchantRef = null;
  }


  function InterestedCallback()
  {
    local bSuccess = false;

    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    if( YesNoResponse.Yes == eResponse )
    {
      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();

      local fDiscount = 0.0;

      switch( g_ciCUO.m_eVersion )
      {
        case GameType.Ultima1: fDiscount = ciPlayer.GetDiscountPercent(); break;
        case GameType.Ultima2: fDiscount = ciPlayer.GetDiscountPercent(); break;
      }

      local iPrice = ciMap.GetProperty( Merchant_Rations_Price_PropertyID, 0 );
      iPrice -= ( iPrice * fDiscount ).tointeger();

      if( iPrice > 0 )
      {
        local strCurrencyDesc;
        if( GameType.Ultima1 == g_ciCUO.m_eVersion )
        {
          strCurrencyDesc = ::rumGetString( token_copper_pieces_client_StringID );
        }
        else
        {
          strCurrencyDesc = ::rumGetString( Gold_Property_client_StringID );
        }

        local iPackSize = ciMap.GetProperty( Merchant_Rations_Pack_Size_PropertyID, 0 );
        if( iPackSize > 0 )
        {
          // Player is interested in purchasing rations
          local strDesc = ::rumGetString( u4_merchant_rations_desc_client_StringID );
          strDesc = format( strDesc, iPackSize, iPrice, strCurrencyDesc );
          strDesc = format( "%s<b><b>%s", strDesc, ::rumGetString( u4_merchant_rations_prompt_2_client_StringID ) );
          strDesc = format( strDesc, iPackSize );
          ShowString( strDesc, g_strColorTagArray.Cyan );

          g_ciUI.m_ciGameInputTextBox.Clear();
          g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );

          // Note that we are sending a callback to a member function, so we have to bind to "this"
          g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, AmountSpecified.bindenv( this ) );

          bSuccess = true;
        }
      }
    }

    if( !bSuccess )
    {
      EndTransaction( true /* inform server */ );
    }
  }


  function TransactionCanceled()
  {
    ShowString( "" );
    Ultima_ListSelectionEnd();
    EndTransaction( true /* inform server */, ::rumGetString( u4_merchant_rations_buy_cancel_client_StringID ) );
  }
}
