class U1_Pond_Widget extends U1_Widget
{
  function Look()
  {
    local strPrompt = ::rumGetString( command_look_pond_client_StringID );
    ShowString( strPrompt );

    g_ciUI.m_ciGameListView.Clear();
    g_ciUI.m_ciGameListView.DisableMultiSelect();
    g_ciUI.m_ciGameListView.SetFormat( "0.05" );
    g_ciUI.m_ciGameListView.ShowPrompt( false );

    local ciPlayer = ::rumGetMainPlayer();
    local iCoin = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );

    if( iCoin > 0 )
    {
      g_ciUI.m_ciGameListView.SetEntry( U1_CoinType.Copper, ::rumGetString( token_copper_pieces_client_StringID ),
                                        rumKeypress.KeyA() );
    }

    if( iCoin >= 10 )
    {
      g_ciUI.m_ciGameListView.SetEntry( U1_CoinType.Silver, ::rumGetString( token_silver_pieces_client_StringID ),
                                        rumKeypress.KeyB() );
    }

    if( iCoin >= 100 )
    {
      g_ciUI.m_ciGameListView.SetEntry( U1_CoinType.Gold, ::rumGetString( token_gold_crowns_client_StringID ),
                                        rumKeypress.KeyC() );
    }

    CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 3 );

    g_ciUI.m_ciGameListView.m_funcAccept = LookCallback.bindenv( this );
    g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
    g_ciUI.m_ciGameListView.SetActive( true );
    g_ciUI.m_ciGameListView.Focus();
  }


  function LookCallback()
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciGameListView.GetCurrentEntry();
    local iIndex = strResponse.find( " " );
    strResponse = strResponse.slice( 0, iIndex );
    ShowString( strResponse + "<b>" );

    local ciPlayer = ::rumGetMainPlayer();
    local iCoin = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );

    local eCoinType = g_ciUI.m_ciGameListView.GetSelectedKey();
    if( ( eCoinType == U1_CoinType.Copper && iCoin > 0 )   ||
        ( eCoinType == U1_CoinType.Silver && iCoin >= 10 ) ||
        ( eCoinType == U1_CoinType.Gold && iCoin >= 100 ) )
    {
      // Only send if this widget exists on the server
      if( GetOriginType() == rumServerOriginType )
      {
        local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetID(), eCoinType );
        ::rumSendBroadcast( ciBroadcast );
      }
    }
    else
    {
      ShowString( ::rumGetString( msg_none_owned_client_StringID ), g_strColorTagArray.Red );
    }

    Ultima_ListSelectionEnd();
  }
}
