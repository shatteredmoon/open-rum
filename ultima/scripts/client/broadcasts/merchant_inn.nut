// Sent to server when a room is purchased
// Received from server when player transacts with an inn merchant
class Merchant_Inn_Broadcast extends rumBroadcast
{
  var1 = 0; // Inn transaction type
  var2 = 0; // Transaction result


  constructor( ... )
  {
    base.constructor();

    if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Inn transaction type
      var2 = vargv[1]; // Room # or 1 if only 1 room is available
    }
  }


  function OnRecv()
  {
    local eTransactionType = var1;

    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();

    local eTokenID = ciMap.GetProperty( Merchant_Inn_Name_PropertyID, rumInvalidStringToken );
    local strShopName = ::rumGetString( eTokenID );

    eTokenID = ciMap.GetProperty( Merchant_Inn_Proprietor_PropertyID, rumInvalidStringToken );
    local strProprietor = ::rumGetString( eTokenID );

    local strDesc;

    if( ( MerchantInnTransaction.Greet == eTransactionType ) ||
        ( MerchantInnTransaction.NoVacancy == eTransactionType ) )
    {
      // No service to players riding horses
      local ciTransport = ciPlayer.GetTransport();
      local eTransportType = ciTransport != null ? ciTransport.GetType() : TransportType.None;
      if( TransportType.Horse == eTransportType )
      {
        EndTransaction( true /* inform server */, ::rumGetString( u4_merchant_inn_horse_client_StringID ) );
        return;
      }

      strDesc = ::rumGetString( u4_merchant_inn_greet_client_StringID );
      strDesc = format( "%s %s.<b><b>", strDesc, strShopName );
    }

    if( MerchantInnTransaction.Greet == eTransactionType )
    {
      PlayMerchantMusic();

      // Save a reference to this instance, otherwise it'll be garbage collected before the callback is made
      g_ciCUO.m_ciMerchantRef = this;

      strDesc += format( "%s %s.<b><b>%s",
                         ::rumGetString( u4_merchant_inn_greet_2_client_StringID ),
                         strProprietor,
                         ::rumGetString( u4_merchant_inn_prompt_client_StringID ) );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      g_ciUI.m_ciYesNoListView.Clear();
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                         rumKeypress.KeyY() );
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                         rumKeypress.KeyN() );

      g_ciUI.m_ciYesNoListView.m_funcAccept = RoomInterestCallback.bindenv( this );
      g_ciUI.m_ciYesNoListView.m_funcCancel = TransactionCanceled.bindenv( this );
      g_ciUI.m_ciYesNoListView.SetActive( true );
      g_ciUI.m_ciYesNoListView.Focus();
    }
    else if( MerchantInnTransaction.NoVacancy == eTransactionType )
    {
      // This inn has no vacancy
      strDesc += ::rumGetString( u4_merchant_inn_no_room_client_StringID );
      ShowString( strDesc + "<b>", g_strColorTagArray.Cyan );

      g_ciUI.m_ciGameInputTextBox.Clear();
    }
    else if( MerchantInnTransaction.RoomPurchase == eTransactionType )
    {
      // The room was successfully purchased
      strDesc = ::rumGetString( u4_merchant_inn_buy_ok_client_StringID );

      // 25% chance of joking with the player
      if( rand() % 4 == 0 )
      {
        strDesc += "<b><b>";
        strDesc += ::rumGetString( u4_merchant_inn_joke_client_StringID );
      }

      ShowString( strDesc + "<b>", g_strColorTagArray.Cyan );

      EndTransaction( true /* inform server */ );
    }
    else if( MerchantInnTransaction.ServerTerminated == eTransactionType )
    {
      // Force transaction quit
      local strReason = var2;
      EndTransaction( false /* do not inform server */, ::rumGetString( strReason ) );
    }
  }


  function EndTransaction( ... )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    if( vargv.len() > 1 && vargv[1] )
    {
      ShowString( vargv[1] + "<b>", g_strColorTagArray.Cyan );
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


  function RoomAcceptedCallback()
  {
    local bInterested = true;
    local eRoomArray = null;
    local eRoomKey = 0;
    local iNumBedChoices = 1;
    local strResponse = "";

    if( g_ciUI.m_ciGameListView.HasInputFocus() )
    {
      eRoomArray = g_ciUI.m_ciGameInputTextBox.m_vPayload["slot1"];
      iNumBedChoices = g_ciUI.m_ciGameInputTextBox.m_vPayload["slot2"];

      eRoomKey = g_ciUI.m_ciGameListView.GetSelectedKey();
      strResponse = g_ciUI.m_ciGameListView.GetCurrentEntry();
    }
    else
    {
      eRoomArray = g_ciUI.m_ciGameInputTextBox.m_vPayload["slot1"];
      iNumBedChoices = g_ciUI.m_ciGameInputTextBox.m_vPayload["slot2"];

      eRoomKey = g_ciUI.m_ciYesNoListView.GetSelectedKey();
      bInterested = ( YesNoResponse.Yes == eRoomKey );
      strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    }

    ShowString( strResponse + "<b>" );

    Ultima_ListSelectionEnd();

    local iPrice = eRoomArray[( eRoomKey - 1 ) * 3];
    if( bInterested && iPrice > 0 )
    {
      // Can the player afford the room?
      local ciPlayer = ::rumGetMainPlayer();
      local iPlayerGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
      if( iPrice <= iPlayerGold )
      {
        // Purchase the room
        local ciBroadcast = ::rumCreate( Merchant_Inn_BroadcastID, MerchantInnTransaction.RoomPurchase, eRoomKey );
        ::rumSendBroadcast( ciBroadcast );
      }
      else
      {
        // Player cannot afford the room
        EndTransaction( true /* inform server */, ::rumGetString( u4_merchant_inn_cant_pay_client_StringID ) );
      }
    }
    else
    {
      // Player does not want a room
      EndTransaction( true /* inform server */, ::rumGetString( u4_merchant_inn_buy_cancel_client_StringID ) );
    }
  }


  function RoomInterestCallback()
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
      // Player is interested in a room
      local eRoomArray = null;

      // Determine the data table row
      local eMapArray = ::rumGetDataTableColumn( merchant_inn_DataTableID, 0 );
      local iRow = eMapArray.find( ciMap.GetAssetID() );
      if( iRow != null )
      {
        // Fetch the row and slice off the map column
        eRoomArray = ::rumGetDataTableRow( merchant_inn_DataTableID, iRow ).slice( 1 );
      }

      local eTokenID = eRoomArray[12];
      local strRoomDesc = ::rumGetString( eTokenID );

      // Inns can have up to 4 rooms
      local iNumBedChoices = ( eRoomArray[0] > 0 ? 1 : 0 ) +
                             ( eRoomArray[3] > 0 ? 1 : 0 ) +
                             ( eRoomArray[6] > 0 ? 1 : 0 ) +
                             ( eRoomArray[9] > 0 ? 1 : 0 );

      if( iNumBedChoices > 1 )
      {
        // Player has a choice of rooms
        local strDesc;
        switch( iNumBedChoices )
        {
          case 1: strDesc = format( strRoomDesc, eRoomArray[0] ); break;
          case 2: strDesc = format( strRoomDesc, eRoomArray[0], eRoomArray[3] ); break;
          case 3: strDesc = format( strRoomDesc, eRoomArray[0], eRoomArray[3], eRoomArray[6] ); break;
          case 4: strDesc = format( strRoomDesc, eRoomArray[0], eRoomArray[3], eRoomArray[6], eRoomArray[9] ); break;
        }

        strDesc += "<b><b>";
        strDesc += ::rumGetString( u4_merchant_inn_choice_client_StringID );
        ShowString( strDesc, g_strColorTagArray.Cyan );

        // Build the room choice list
        g_ciUI.m_ciGameListView.Clear();
        g_ciUI.m_ciGameListView.DisableMultiSelect();
        g_ciUI.m_ciGameListView.SetFormat( "0.05" );
        g_ciUI.m_ciGameListView.ShowPrompt( false );

        g_ciUI.m_ciGameListView.SetEntry( 1, "1", rumKeypress.Key1() );
        g_ciUI.m_ciGameListView.SetEntry( 2, "2", rumKeypress.Key2() );
        g_ciUI.m_ciGameListView.SetEntry( 3, "3", rumKeypress.Key3() );

        CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

        g_ciUI.m_ciGameListView.m_funcAccept = RoomAcceptedCallback.bindenv( this );
        g_ciUI.m_ciGameListView.m_funcCancel = TransactionCanceled.bindenv( this );
        g_ciUI.m_ciGameListView.SetActive( true );
        g_ciUI.m_ciGameListView.Focus();

        g_ciUI.m_ciGameInputTextBox.m_vPayload = { slot1 = eRoomArray, slot2 = iNumBedChoices };
      }
      else
      {
        // Player only has one room to choose from
        local strDesc = format( strRoomDesc, eRoomArray[0] );
        strDesc += "<b><b>";
        strDesc += ::rumGetString( u4_merchant_inn_prompt_2_client_StringID );
        ShowString( strDesc, g_strColorTagArray.Cyan );

        g_ciUI.m_ciYesNoListView.Clear();
        g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                           rumKeypress.KeyY() );
        g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                           rumKeypress.KeyN() );

        g_ciUI.m_ciYesNoListView.m_funcAccept = RoomAcceptedCallback.bindenv( this );
        g_ciUI.m_ciYesNoListView.m_funcCancel = TransactionCanceled.bindenv( this );
        g_ciUI.m_ciYesNoListView.SetActive( true );
        g_ciUI.m_ciYesNoListView.Focus();

        g_ciUI.m_ciGameInputTextBox.m_vPayload = { slot1 = eRoomArray, slot2 = iNumBedChoices };
      }
    }
    else
    {
      // Player does not want a room
      local eTokenID = ciMap.GetProperty( Merchant_Inn_Proprietor_PropertyID, rumInvalidStringToken );
      local strProprietor = ::rumGetString( eTokenID );

      local strDesc = format( "%s %s",
                              strProprietor,
                              ::rumGetString( u4_merchant_inn_not_interested_client_StringID ) );
      EndTransaction( true /* inform server */, strDesc );
    }
  }


  function TransactionCanceled()
  {
    ShowString( "" );
    Ultima_ListSelectionEnd();
    EndTransaction( true /* inform server */, ::rumGetString( u4_merchant_inn_buy_cancel_client_StringID ) );
  }
}
