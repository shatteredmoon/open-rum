// Received from server when player transacts with a tavern merchant
class Merchant_Tavern_Broadcast extends rumBroadcast
{
  var1 = 0; // Tavern transaction type
  var2 = 0; // Transaction result
  var3 = 0; // Response


  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Tavern transaction type
    }
    else if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Tavern transaction type
      var2 = vargv[1]; // Payment amount
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

    if( MerchantTavernTransaction.Greet == eTransactionType )
    {
      PlayMerchantMusic();

      local eTokenID = ciMap.GetProperty( Merchant_Tavern_Name_PropertyID, rumInvalidStringToken );
      local strShopName = ::rumGetString( eTokenID );

      eTokenID = ciMap.GetProperty( Merchant_Tavern_Proprietor_PropertyID, rumInvalidStringToken );
      local strProprietor = ::rumGetString( eTokenID );

      // Greet the player and question interest
      strDesc = format( "%s %s %s.<b><b>%s",
                        strProprietor,
                        ::rumGetString( u4_merchant_tavern_greet_client_StringID ),
                        strShopName,
                        ::rumGetString( u4_merchant_tavern_prompt_client_StringID ) );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      g_ciUI.m_ciYesNoListView.Clear();
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes,
                                         ::rumGetString( u4_merchant_tavern_food_client_StringID ),
                                         rumKeypress.KeyY() );
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No,
                                         ::rumGetString( u4_merchant_tavern_ale_client_StringID ),
                                         rumKeypress.KeyN() );

      g_ciUI.m_ciYesNoListView.m_funcAccept = InterestedCallback.bindenv( this );
      g_ciUI.m_ciYesNoListView.m_funcCancel = TransactionCanceled.bindenv( this );
      g_ciUI.m_ciYesNoListView.SetActive( true );
      g_ciUI.m_ciYesNoListView.Focus();
    }
    else if( MerchantTavernTransaction.PurchaseFood == eTransactionType )
    {
      OfferService( ::rumGetString( u4_merchant_tavern_food_2_client_StringID ) );
    }
    else if( MerchantTavernTransaction.PurchaseAle == eTransactionType )
    {
      if( GameType.Ultima4 == g_ciCUO.m_eVersion )
      {
        local bQuestion = var2;
        if( bQuestion )
        {
          strDesc = ::rumGetString( u4_merchant_tavern_tip_response_client_StringID );
          ShowString( strDesc, g_strColorTagArray.Cyan );

          g_ciUI.m_ciGameInputTextBox.Clear();
          g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
          g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Text_Response, QuestionCallback.bindenv( this ) );
        }
        else
        {
          OfferService();
        }
      }
      else
      {
        // TODO - handle clues
      }
    }
    else if( MerchantTavernTransaction.Question == eTransactionType )
    {
      local bMatch = var2;
      if( bMatch )
      {
        strDesc = ::rumGetString( u4_merchant_tavern_foggy_client_StringID );
        ShowString( strDesc, g_strColorTagArray.Cyan );

        g_ciUI.m_ciGameInputTextBox.Clear();
        g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );

        // Handle the selection menu - note that we are sending a callback to a member function, so we have to bind
        // the function to "this"
        g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, FoggyMemoryCallback.bindenv( this ) );
      }
      else
      {
        OfferService( ::rumGetString( u4_merchant_tavern_no_info_client_StringID ) );
      }
    }
    else if( MerchantTavernTransaction.Tip == eTransactionType )
    {
      local strClue = var2;
      OfferService( strClue );
    }
    else if( MerchantTavernTransaction.ServerTerminated == eTransactionType )
    {
      local strReason = var2;
      EndTransaction( false /* do not inform server */, strReason );
    }
  }


  function AleResponseCallback( i_iAmount )
  {
    if( null == i_iAmount )
    {
      ShowString( "" );
      EndTransaction( true /* inform server */, ::rumGetString( u4_merchant_tavern_wont_pay_client_StringID ) );
      return;
    }

    // Push the player specified amount
    ShowString( i_iAmount.tostring() + "<b>" );
    g_ciUI.m_ciGameInputTextBox.Clear();

    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();

    // Does player have the amount specified?
    local iPrice = ciMap.GetProperty( Merchant_Tavern_Ale_Price_PropertyID, 1 );
    local iGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
    if( iGold >= iPrice )
    {
      // Did the player offer a larger tip than they can provide?
      if( i_iAmount > iGold )
      {
        // Just scale down to what they can afford
        i_iAmount = iGold;
      }

      local ciBroadcast = ::rumCreate( Merchant_Tavern_BroadcastID,
                                       MerchantTavernTransaction.PurchaseAle, i_iAmount );
      ::rumSendBroadcast( ciBroadcast );
    }
    else
    {
      EndTransaction( true /* inform server */, ::rumGetString( u4_merchant_tavern_wont_pay_client_StringID ) );
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
      local strDesc = ::rumGetString( u4_merchant_tavern_prompt_client_StringID );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      g_ciUI.m_ciYesNoListView.Clear();
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes,
                                         ::rumGetString( u4_merchant_tavern_food_client_StringID ),
                                         rumKeypress.KeyY() );
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No,
                                         ::rumGetString( u4_merchant_tavern_ale_client_StringID ),
                                         rumKeypress.KeyN() );

      g_ciUI.m_ciYesNoListView.m_funcAccept = InterestedCallback.bindenv( this );
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
      strDesc = ::rumGetString( u4_merchant_tavern_bye_client_StringID );
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


  function FoggyMemoryCallback( i_iAmount )
  {
    if( null == i_iAmount )
    {
      PurchaseCanceled( ::rumGetString( u4_merchant_tavern_no_info_2_client_StringID ) );
      return;
    }

    // Push the player specified amount
    ShowString( i_iAmount.tostring() + "<b>" );
    g_ciUI.m_ciGameInputTextBox.Clear();

    local ciPlayer = ::rumGetMainPlayer();

    // Does player have the amount specified?
    local iGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );

    // Did the player offer a larger tip than they can provide?
    if( i_iAmount > iGold )
    {
      // Just scale down to what they can afford
      i_iAmount = iGold;
    }

    if( i_iAmount > 0 )
    {
      local ciBroadcast = ::rumCreate( Merchant_Tavern_BroadcastID, MerchantTavernTransaction.Tip, i_iAmount );
      ::rumSendBroadcast( ciBroadcast );
    }
    else
    {
      OfferService( ::rumGetString( u4_merchant_tavern_no_info_2_client_StringID ) );
    }
  }


  function FoodResponseCallback()
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

      // Player wants food, do they have enough gold?
      local iPrice = ciMap.GetProperty( Merchant_Tavern_Food_Price_PropertyID, 2 );
      local iGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
      if( iGold >= iPrice )
      {
        local ciBroadcast = ::rumCreate( Merchant_Tavern_BroadcastID, MerchantTavernTransaction.PurchaseFood );
        ::rumSendBroadcast( ciBroadcast );
      }
      else
      {
        EndTransaction( true /* inform server */, ::rumGetString( u4_merchant_tavern_wont_pay_client_StringID ) );
      }
    }
    else
    {
      OfferService();
    }
  }


  function InterestedCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();

    local strCurrencyDesc;
    if( GameType.Ultima1 == g_ciCUO.m_eVersion )
    {
      // TODO - this doesn't really work when dealing with just 1 piece
      strCurrencyDesc = ::rumGetString( token_copper_pieces_client_StringID );
    }
    else
    {
      strCurrencyDesc = ::rumGetString( Gold_Property_client_StringID );
    }

    local strDesc;
    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    if( YesNoResponse.Yes == eResponse )
    {
      // Player wants food
      local eTokenID = ciMap.GetProperty( Merchant_Tavern_Special_PropertyID,
                                          u1_merchant_tavern_special_client_StringID );
      local strSpecialty = ::rumGetString( eTokenID );
      local iPrice = ciMap.GetProperty( Merchant_Tavern_Food_Price_PropertyID, 2 );

      strDesc = ::rumGetString( u4_merchant_tavern_specialty_client_StringID );
      strDesc = format( strDesc, strSpecialty, iPrice, strCurrencyDesc.tolower() );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      g_ciUI.m_ciYesNoListView.Clear();
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                         rumKeypress.KeyY() );
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                         rumKeypress.KeyN() );

      g_ciUI.m_ciYesNoListView.m_funcAccept = FoodResponseCallback.bindenv( this );
      g_ciUI.m_ciYesNoListView.m_funcCancel = PurchaseCanceled.bindenv( this );
      g_ciUI.m_ciYesNoListView.SetActive( true );
      g_ciUI.m_ciYesNoListView.Focus();
    }
    else
    {
      // Player wants ale
      strDesc = ::rumGetString( u4_merchant_tavern_ale_2_client_StringID );

      local iPrice = ciMap.GetProperty( Merchant_Tavern_Ale_Price_PropertyID, 1 );

      strDesc = format( strDesc, iPrice, strCurrencyDesc.tolower() );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      g_ciUI.m_ciGameInputTextBox.Clear();
      g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );

      // Handle the selection menu - note that we are sending a callback to a member function, so we have to bind
      // the function to "this"
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, AleResponseCallback.bindenv( this ) );
    }
  }


  function OfferService( ... )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    local strDesc;

    if( ( 1 == vargv.len() ) && vargv[0] )
    {
      strDesc = format( "%s<b><b>%s", vargv[0], ::rumGetString( u4_merchant_tavern_prompt_2_client_StringID ) );
    }
    else
    {
      strDesc = ::rumGetString( u4_merchant_tavern_prompt_2_client_StringID );
    }

    ShowString( strDesc, g_strColorTagArray.Cyan );

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

    if( vargv.len() > 0 )
    {
      OfferService( vargv[0] );
    }
    else
    {
      OfferService();
    }
  }


  function QuestionCallback( ... )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    if( vargv.len() > 0 )
    {
      local strQuestion = vargv[0];

      // Push the player keyword
      ShowString( strQuestion + "<b>" );

      local ciBroadcast = ::rumCreate( Merchant_Tavern_BroadcastID, MerchantTavernTransaction.Question, strQuestion );
      ::rumSendBroadcast( ciBroadcast );
    }
    else
    {
      ShowString( "" );
      OfferService();
    }
  }


  function TransactionCanceled()
  {
    ShowString( "" );
    Ultima_ListSelectionEnd();
    EndTransaction( true /* inform server */ );
  }
}
