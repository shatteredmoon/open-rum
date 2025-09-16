// Received from server when player transacts with a stable merchant
class Merchant_Stable_Broadcast extends rumBroadcast
{
  var1 = 0; // Stable transaction type
  var2 = 0; // Response


  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Stable transaction type
    }
  }


  function OnRecv()
  {
    local eTransactionType = var1;
    local strDesc;

    // Save a reference to this instance, otherwise it'll be garbage collected before the callback is made
    g_ciCUO.m_ciMerchantRef = this;

    if( MerchantStableTransaction.Greet == eTransactionType )
    {
      PlayMerchantMusic();

      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();

      local eTokenID = ciMap.GetProperty( Merchant_Stable_Name_PropertyID, rumInvalidStringToken );
      local strShopName = ::rumGetString( eTokenID );

      eTokenID = ciMap.GetProperty( Merchant_Stable_Proprietor_PropertyID, rumInvalidStringToken );
      local strProprietor = ::rumGetString( eTokenID );

      // Greet the player and question interest
      strDesc = format( "%s %s %s.<b><b>%s",
                        strProprietor,
                        ::rumGetString( u4_merchant_stable_greet_client_StringID ),
                        strShopName,
                        ::rumGetString( u4_merchant_stable_prompt_client_StringID ) );
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
    else if( MerchantStableTransaction.Purchase == eTransactionType )
    {
      local eStringTokenID = var2;
      EndTransaction( true /* inform server */, eStringTokenID );
    }
    else if( MerchantStableTransaction.ServerTerminated == eTransactionType )
    {
      local eStringTokenID = var2;
      EndTransaction( false /* do not inform server */, eStringTokenID );
    }
  }


  function EndTransaction( ... )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    if( vargv.len() > 1 && vargv[1] )
    {
      ShowString( ::rumGetString( vargv[1] ) + "<b>", g_strColorTagArray.Cyan );
    }

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
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    // Does the player already own a horse?
    local ciPlayer = ::rumGetMainPlayer();
    local bOwnsHorse = ciPlayer.GetProperty( U4_Horse_PropertyID, false );
    if( bOwnsHorse )
    {
      EndTransaction( true /* inform server */, u4_merchant_stable_owns_horse_client_StringID );
      return;
    }

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    if( YesNoResponse.Yes == eResponse )
    {
      local ciMap = ciPlayer.GetMap();
      local iPrice = ciMap.GetProperty( Merchant_Stable_Price_PropertyID, 0 );
      local strDesc = format( ::rumGetString( u4_merchant_stable_desc_client_StringID ), iPrice );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      g_ciUI.m_ciYesNoListView.Clear();
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                         rumKeypress.KeyY() );
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                         rumKeypress.KeyN() );

      g_ciUI.m_ciYesNoListView.m_funcAccept = PurchaseCallback.bindenv( this );
      g_ciUI.m_ciYesNoListView.m_funcCancel = TransactionCanceled.bindenv( this );
      g_ciUI.m_ciYesNoListView.SetActive( true );
      g_ciUI.m_ciYesNoListView.Focus();
    }
    else
    {
      EndTransaction( true /* inform server */, u4_merchant_stable_buy_cancel_client_StringID );
    }
  }


  function PurchaseCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    if( YesNoResponse.Yes == eResponse )
    {
      // Can player afford the horse?
      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();
      local iPrice = ciMap.GetProperty( Merchant_Stable_Price_PropertyID, 0 );
      if( iPrice > 0 )
      {
        local iPlayerGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
        if( iPlayerGold >= iPrice )
        {
          local ciBroadcast = ::rumCreate( Merchant_Stable_BroadcastID, MerchantReagentTransaction.Purchase );
          ::rumSendBroadcast( ciBroadcast );
        }
        else
        {
          EndTransaction( true /* inform server */, u4_merchant_stable_cant_pay_client_StringID );
        }
      }
      else
      {
        // Something wrong with price
        EndTransaction( true /* inform server */, u4_merchant_stable_buy_cancel_client_StringID );
      }
    }
    else
    {
      // Player not interested
      EndTransaction( true /* inform server */, u4_merchant_stable_buy_cancel_client_StringID );
    }
  }


  function TransactionCanceled()
  {
    ShowString( "" );
    Ultima_ListSelectionEnd();
    EndTransaction( true /* inform server */, u4_merchant_stable_buy_cancel_client_StringID );
  }
}
