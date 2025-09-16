// Received from server when player transacts with a reagent merchant
class Merchant_Reagents_Broadcast extends rumBroadcast
{
  var1 = 0; // Reagent type
  var2 = 0; // Transaction result
  var3 = 0;
  var4 = 0;


  constructor( ... )
  {
    base.constructor();

    if( 4 == vargv.len() )
    {
      var1 = vargv[0]; // Reagent transaction type
      var2 = vargv[1]; // Reagent type
      var3 = vargv[2]; // Reagent amount
      var4 = vargv[3]; // Amount paid
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

    if( MerchantReagentTransaction.Greet == eTransactionType )
    {
      // Show the relevant equipment stat page
      g_ciUI.m_ePreviousStatPage = g_ciUI.m_eCurrentStatPage;
      g_ciUI.m_eCurrentStatPage = U4_StatPage.Reagents;

      Ultima_Stat_Update();

      PlayMerchantMusic();

      local eTokenID = ciMap.GetProperty( Merchant_Reagent_Name_PropertyID, rumInvalidStringToken );
      local strShopName = ::rumGetString( eTokenID );

      eTokenID = ciMap.GetProperty( Merchant_Reagent_Proprietor_PropertyID, rumInvalidStringToken );
      local strProprietor = ::rumGetString( eTokenID );

      // Greet the player and question interest
      strDesc = format( "%s %s.<b><b>%s %s.<b><b>%s",
                        ::rumGetString( u4_merchant_reagents_greet_client_StringID ),
                        strShopName,
                        ::rumGetString( u4_merchant_reagents_greet_2_client_StringID ),
                        strProprietor,
                        ::rumGetString( u4_merchant_reagents_prompt_client_StringID ) );
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
    else if( MerchantReagentTransaction.Purchase == eTransactionType )
    {
      OfferService( u4_merchant_reagents_purchase_done_client_StringID );
    }
    else if( MerchantReagentTransaction.ServerTerminated == eTransactionType )
    {
      EndTransaction( false /* do not inform server */ );
    }
  }


  function AmountSpecified( i_eReagent, i_iAmount )
  {
    if( null == i_iAmount )
    {
      PurchaseCanceled();
      return;
    }

    ShowString( i_iAmount.tostring() + "<b>" );
    g_ciUI.m_ciGameInputTextBox.Clear();

    if( i_iAmount > 0 )
    {
      i_iAmount = clamp( i_iAmount, 0, 99 );

      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();

      local uiPriceArray = null;
      local iPrice = 0;

      local iMapArray = ::rumGetDataTableColumn( merchant_reagents_DataTableID, 0 );
      local iRow = iMapArray.find( ciMap.GetAssetID() );
      if( iRow != null )
      {
        // Fetch the row and slice off the map column
        uiPriceArray = ::rumGetDataTableRow( merchant_reagents_DataTableID, iRow ).slice( 1 );
        iPrice = uiPriceArray[i_eReagent] * i_iAmount;
      }

      local strDesc = ::rumGetString( u4_merchant_reagents_price_client_StringID );
      strDesc = format( strDesc, iPrice );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      g_ciUI.m_ciGameInputTextBox.Clear();
      g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );

      // Note that we are sending a callback to a member function, so we have to bind to "this"
      local iReagentInfoArray = [i_eReagent, i_iAmount];
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, Purchase.bindenv( this ), iReagentInfoArray );
    }
    else
    {
      // Player is not interested in purchasing the reagent
      OfferService( u4_merchant_reagents_purchase_cancel_client_StringID );
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

      local eTokenID = ciMap.GetProperty( Merchant_Reagent_Proprietor_PropertyID, rumInvalidStringToken );
      local strProprietor = ::rumGetString( eTokenID );

      strDesc = format( "%s %s", strProprietor, ::rumGetString( u4_merchant_reagents_bye_client_StringID ) );
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

    local strDesc;

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    if( YesNoResponse.Yes == eResponse )
    {
      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();

      // Build the reagents list
      g_ciUI.m_ciGameListView.Clear();
      g_ciUI.m_ciGameListView.DisableMultiSelect();
      g_ciUI.m_ciGameListView.SetFormat( "0.05|0.15" );
      g_ciUI.m_ciGameListView.ShowPrompt( false );

      local uiPriceArray = null;

      local iMapArray = ::rumGetDataTableColumn( merchant_reagents_DataTableID, 0 );
      local iRow = iMapArray.find( ciMap.GetAssetID() );
      if( iRow != null )
      {
        // Fetch the row and slice off the map column
        uiPriceArray = ::rumGetDataTableRow( merchant_reagents_DataTableID, iRow ).slice( 1 );
      }

      strDesc = ::rumGetString( u4_merchant_reagents_desc_client_StringID );
      strDesc += " ";

      local uiNumReagents = uiPriceArray.len();
      for( local iIndex = 0; iIndex < uiNumReagents; ++iIndex )
      {
        if( uiPriceArray[iIndex] > 0 )
        {
          local strReagentDesc = ::rumGetString( g_eU4ReagentStringArray[iIndex] );
          local iKey = rumKeypress.KeyA() + iIndex;
          local strAlpha = ::rumGetStringByName( format( "alpha%02d_client_StringID", iIndex ) );
          g_ciUI.m_ciGameListView.SetEntry( iIndex, strAlpha + "|" + strReagentDesc, iKey );

          if( iIndex > 0 )
          {
            strDesc += ", ";

            if( ( iIndex + 1 ) == uiNumReagents )
            {
              strDesc += ::rumGetString( token_and_client_StringID );
              strDesc += " ";
            }
          }

          strDesc += strReagentDesc;
        }
      }

      strDesc += ".<b><b>";
      strDesc += ::rumGetString( u4_merchant_reagents_prompt_4_client_StringID );

      ShowString( strDesc, g_strColorTagArray.Cyan );

      CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

      g_ciUI.m_ciGameListView.m_funcAccept = ReagentSelectCallback.bindenv( this );
      g_ciUI.m_ciGameListView.m_funcCancel = PurchaseCanceled.bindenv( this );
      g_ciUI.m_ciGameListView.m_funcIndexChanged = SelectionChanged.bindenv( this );
      g_ciUI.m_ciGameListView.SetActive( true );
      g_ciUI.m_ciGameListView.Focus();
    }
    else
    {
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
                        ::rumGetString( u4_merchant_reagents_prompt_3_client_StringID ) );
    }
    else
    {
      strDesc = ::rumGetString( u4_merchant_reagents_prompt_3_client_StringID );
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


  function Purchase( i_iReagentInfoArray, i_iPaid )
  {
    if( null == i_iPaid )
    {
      // Player is not interested in purchasing the reagent
      ShowString( "" );
      OfferService( u4_merchant_reagents_purchase_cancel_client_StringID );
      return;
    }

    ShowString( i_iPaid.tostring() + "<b>" );

    g_ciUI.m_ciGameInputTextBox.Clear();

    if( i_iPaid > 0 )
    {
      local ciBroadcast = ::rumCreate( Merchant_Reagents_BroadcastID, MerchantReagentTransaction.Purchase,
                                       i_iReagentInfoArray[0], i_iReagentInfoArray[1], i_iPaid );
      ::rumSendBroadcast( ciBroadcast );
    }
    else
    {
      // Player is not interested in purchasing the reagent
      OfferService( u4_merchant_reagents_purchase_cancel_client_StringID );
    }
  }


  function PurchaseCanceled()
  {
    ShowString( "" );
    Ultima_ListSelectionEnd();
    OfferService( u4_merchant_reagents_purchase_cancel_client_StringID );
  }


  function ReagentSelectCallback()
  {
    // Push the player's selection to output
    local strReagent = g_ciUI.m_ciGameInputTextBox.GetText();
    ShowString( strReagent + "<b>" );

    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();

    local eReagent = g_ciUI.m_ciGameListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    local uiPriceArray = null;
    local iMapArray = ::rumGetDataTableColumn( merchant_reagents_DataTableID, 0 );
    local iRow = iMapArray.find( ciMap.GetAssetID() );
    if( iRow != null )
    {
      // Fetch the row and slice off the map column
      uiPriceArray = ::rumGetDataTableRow( merchant_reagents_DataTableID, iRow ).slice( 1 );
      local iPrice = uiPriceArray[eReagent];

      local strDesc = ::rumGetString( u4_merchant_reagents_desc_2_client_StringID );
      strDesc = format( strDesc, strReagent, iPrice );

      ShowString( strDesc, g_strColorTagArray.Cyan );
    }

    g_ciUI.m_ciGameInputTextBox.Clear();
    g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );

    // Note that we are sending a callback to a member function, so we have to bind to "this"
    g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, AmountSpecified.bindenv( this ), eReagent );
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
