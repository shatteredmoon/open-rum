// Received from server when player transacts with an oracle
class Merchant_Oracle_Broadcast extends rumBroadcast
{
  var1 = 0; // Transaction type
  var2 = 0; // Message
  var3 = 0; // Message index
  var4 = 0; // Charge

  static s_iNumHints = 10;


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() >= 1 )
    {
      var1 = vargv[0]; // Transaction type

      if( vargv.len() >= 2 )
      {
        var2 = vargv[1]; // Hint index
      }
    }
  }


  function OnRecv()
  {
    local eTransactionType = var1;
    local strDesc;

    // Save a reference to this instance, otherwise it'll be garbage collected before the callback is made
    g_ciCUO.m_ciMerchantRef = this;

    if( MerchantOracleTransaction.Greet == eTransactionType )
    {
      PlayMerchantMusic();

      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();

      // Greet the player and question interest
      strDesc = ::rumGetString( u3_merchant_oracle_greet_client_StringID );
      ShowString( format( "%s<b>", strDesc ), g_strColorTagArray.Cyan );
    }
    else if( MerchantOracleTransaction.Hint == eTransactionType )
    {
      ShowString( format( "%s<b>", var2 ), g_strColorTagArray.Cyan );

      local iNextHintIndex = var3 + 1;
      if( iNextHintIndex < s_iNumHints )
      {
        local bChargeForNext = var4;
        if( bChargeForNext )
        {
          // Prompt for next hint
          strDesc = ::rumGetString( u3_merchant_oracle_prompt_client_StringID );
          strDesc = format( strDesc, iNextHintIndex * 100 );
          ShowString( format( "%s", strDesc ), g_strColorTagArray.Cyan );

          g_ciUI.m_ciYesNoListView.Clear();
          g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                             rumKeypress.KeyY() );
          g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                             rumKeypress.KeyN() );

          g_ciUI.m_ciYesNoListView.m_funcAccept = HintInterestCallback.bindenv( this );
          g_ciUI.m_ciYesNoListView.m_funcCancel = TransactionCanceled.bindenv( this );
          g_ciUI.m_ciYesNoListView.SetActive( true );
          g_ciUI.m_ciYesNoListView.Focus();
        }
        else
        {
          // Get a keypress from the player
          g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.PressAnyKey, GetNextHint.bindenv( this ) );
        }
      }
      else
      {
        EndTransaction( true /* inform server */, u3_merchant_oracle_bye_client_StringID );
      }
    }
    else if( MerchantOracleTransaction.ServerTerminated == eTransactionType )
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


  function GetNextHint()
  {
    local iNextHintIndex = var3 + 1;
    local ciBroadcast = ::rumCreate( Merchant_Oracle_BroadcastID, MerchantOracleTransaction.RequestNext,
                                     iNextHintIndex );
    ::rumSendBroadcast( ciBroadcast );
  }


  function HintInterestCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();

    Ultima_ListSelectionEnd();

    if( YesNoResponse.Yes == eResponse )
    {
      local iNextHintIndex = var3 + 1;
      local iCost = iNextHintIndex * 100;
      local iPlayerGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
      if( iPlayerGold >= iCost )
      {
        local ciBroadcast = ::rumCreate( Merchant_Oracle_BroadcastID, MerchantOracleTransaction.RequestNext,
                                         iNextHintIndex );
        ::rumSendBroadcast( ciBroadcast );
      }
      else
      {
        EndTransaction( true /* inform server */, u3_merchant_oracle_cant_pay_client_StringID );
      }
    }
    else
    {
      EndTransaction( true /* inform server */, u3_merchant_oracle_bye_client_StringID );
    }
  }


  function TransactionCanceled()
  {
    ShowString( "" );
    Ultima_ListSelectionEnd();
    EndTransaction( true /* inform server */, u3_merchant_oracle_bye_client_StringID );
  }
}
