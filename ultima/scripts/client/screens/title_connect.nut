class ConnectTitleStage
{
  m_ciSettingsTable =
  {
    bNeedsRenderInit = true,
    bTriedConnecting = false
  };


  function BeginStage()
  {
    // TODO - base this on ConnectionStatus rather than socket
    local iSocket = ::rumGetSocket();
    if( iSocket != -1 )
    {
      // Already connected
      EndStage( TitleStages.AccountMenu );
      return;
    }

    g_ciUI.m_ciTitleLabel.SetActive( true );
    g_ciUI.m_ciInfoLabel.SetActive( true );

    // Make sure server and port values are filled out!
    local strServer = g_ciCUO.m_ciGameConfigTable["cuo:server"];
    local strPort = g_ciCUO.m_ciGameConfigTable["cuo:port"];

    if( !m_ciSettingsTable.bTriedConnecting && strServer != "" )
    {
      ::rumServerConnect( strServer, strPort );
      local strConnectDesc = ::rumGetString( msg_connect_desc_client_StringID );
      strConnectDesc = format( strConnectDesc, strServer, strPort );
      g_ciUI.m_ciInfoLabel.SetText( strConnectDesc );

      // The next title stage will be set in OnConnectionComplete()
      m_ciSettingsTable.bTriedConnecting = true;
      g_ciCUO.m_bWaitingForResponse = true;
    }
    else
    {
      g_ciUI.m_ciInfoLabel.SetText( ::rumGetString( msg_connect_address_invalid_client_StringID ) );

      local strServer = g_ciCUO.m_ciGameConfigTable["cuo:server"];
      local strPort = g_ciCUO.m_ciGameConfigTable["cuo:port"]

      g_ciUI.m_ciTitleListMenu.SetActive( true );
      g_ciUI.m_ciTitleListMenu.Clear();
      g_ciUI.m_ciTitleListMenu.SetEntry( ConnectMenu.Server, ::rumGetString( token_address_client_StringID ) );
      g_ciUI.m_ciTitleListMenu.SetEntry( ConnectMenu.Port, ::rumGetString( token_port_client_StringID ) );
      g_ciUI.m_ciTitleListMenu.SetEntry( ConnectMenu.Connect, ::rumGetString( token_connect_client_StringID ) );
      g_ciUI.m_ciTitleListMenu.SetWidth( g_ciUI.s_iMenuWidth );
      g_ciUI.m_ciTitleListMenu.m_funcAccept = OnListMenuAccepted.bindenv( ConnectTitleStage );
      g_ciUI.m_ciTitleListMenu.m_funcCancel = OnListMenuCanceled.bindenv( ConnectTitleStage );
      g_ciUI.m_ciTitleListMenu.m_funcKeyPress = OnListMenuKeyPressed.bindenv( ConnectTitleStage );

      CalculateListViewSize( g_ciUI.m_ciTitleListMenu, "default", 3 );

      g_ciUI.m_ciTextBoxArray[ConnectMenu.Server].SetText( strServer );
      g_ciUI.m_ciTextBoxArray[ConnectMenu.Server].ObscureText( false );
      g_ciUI.m_ciTextBoxArray[ConnectMenu.Server].SetActive( true );
      g_ciUI.m_ciTextBoxArray[ConnectMenu.Server].ShowPrompt( true );
      g_ciUI.m_ciTextBoxArray[ConnectMenu.Server].m_funcAccept = OnTextBoxDone.bindenv( ConnectTitleStage );
      g_ciUI.m_ciTextBoxArray[ConnectMenu.Server].m_funcCancel = OnTextBoxDone.bindenv( ConnectTitleStage );

      g_ciUI.m_ciTextBoxArray[ConnectMenu.Port].SetText( strPort );
      g_ciUI.m_ciTextBoxArray[ConnectMenu.Port].ObscureText( false );
      g_ciUI.m_ciTextBoxArray[ConnectMenu.Port].SetActive( true );
      g_ciUI.m_ciTextBoxArray[ConnectMenu.Port].ShowPrompt( true );
      g_ciUI.m_ciTextBoxArray[ConnectMenu.Port].m_funcAccept = OnTextBoxDone.bindenv( ConnectTitleStage );
      g_ciUI.m_ciTextBoxArray[ConnectMenu.Port].m_funcCancel = OnTextBoxDone.bindenv( ConnectTitleStage );

      g_ciUI.m_ciTitleListMenu.Focus();
    }
  }


  function EndStage( i_eNextStage )
  {
    m_ciSettingsTable.bNeedsRenderInit = true;

    g_ciUI.m_ciInfoLabel.SetActive( false );
    g_ciUI.m_ciInfoLabel.SetText( "" );
    g_ciUI.m_ciTextBoxArray[ConnectMenu.Server].SetActive( false );
    g_ciUI.m_ciTextBoxArray[ConnectMenu.Server].ClearHandlers();
    g_ciUI.m_ciTextBoxArray[ConnectMenu.Port].SetActive( false );
    g_ciUI.m_ciTextBoxArray[ConnectMenu.Port].ClearHandlers();
    g_ciUI.m_ciTitleLabel.SetActive( false );
    g_ciUI.m_ciTitleListMenu.SetActive( false );
    g_ciUI.m_ciTitleListMenu.ClearHandlers();

    InitTitleStage( i_eNextStage );
  }


  function OnListMenuAccepted()
  {
    g_ciUI.m_ciInfoLabel.SetText( "" );

    local iSelectedKey = g_ciUI.m_ciTitleListMenu.GetSelectedKey();

    if( ConnectMenu.Server == iSelectedKey )
    {
      g_ciUI.m_ciTextBoxArray[ConnectMenu.Server].Focus();
    }
    else if( ConnectMenu.Port == iSelectedKey )
    {
      g_ciUI.m_ciTextBoxArray[ConnectMenu.Port].Focus();
    }
    else if( ConnectMenu.Connect == iSelectedKey )
    {
      local bUpdate = false;

      if( !g_ciCUO.m_bWaitingForResponse )
      {
        local strServer = g_ciUI.m_ciTextBoxArray[ConnectMenu.Server].GetText();
        if( g_ciCUO.m_ciGameConfigTable["cuo:server"] != strServer )
        {
          g_ciCUO.m_ciGameConfigTable["cuo:server"] = strServer;
          bUpdate = true;
        }

        local strPort = g_ciUI.m_ciTextBoxArray[ConnectMenu.Port].GetText();
        if( g_ciCUO.m_ciGameConfigTable["cuo:port"] != g_ciUI.m_ciTextBoxArray[ConnectMenu.Port].GetText() )
        {
          g_ciCUO.m_ciGameConfigTable["cuo:port"] = g_ciUI.m_ciTextBoxArray[ConnectMenu.Port].GetText();
          bUpdate = true;
        }

        if( bUpdate )
        {
          ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );
        }

        local strConnectDesc = ::rumGetString( msg_connect_desc_client_StringID );
        strConnectDesc = format( strConnectDesc, strServer, strPort );
        g_ciUI.m_ciInfoLabel.SetText( strConnectDesc );

        m_ciSettingsTable.bTriedConnecting = false;

        // Retry connection
        EndStage( TitleStages.Connect );
      }
    }
  }


  function OnListMenuCanceled()
  {
    g_ciUI.m_ciInfoLabel.SetText( "" );
    EndStage( TitleStages.Splash );
  }


  function OnListMenuKeyPressed( i_ciKeyInput )
  {
    g_ciUI.m_ciInfoLabel.SetText( "" );

    local eKey = i_ciKeyInput.GetKey();
    local iSelectedKey = g_ciUI.m_ciTitleListMenu.GetSelectedKey();

    if( ( rumKeypress.KeyRight() == eKey ) || ( rumKeypress.KeyTab() == eKey ) )
    {
      if( ConnectMenu.Server == iSelectedKey )
      {
        g_ciUI.m_ciTextBoxArray[ConnectMenu.Server].Focus();
      }
      else if( ConnectMenu.Port == iSelectedKey )
      {
        g_ciUI.m_ciTextBoxArray[ConnectMenu.Port].Focus();
      }
    }
  }


  function OnTextBoxDone()
  {
    if( g_ciUI.m_ciTextBoxArray[ConnectMenu.Server].HasInputFocus() ||
        g_ciUI.m_ciTextBoxArray[ConnectMenu.Port].HasInputFocus() )
    {
      g_ciUI.m_ciTitleListMenu.MoveNext();
    }

    g_ciUI.m_ciTitleListMenu.Focus();
  }


  function Render( i_iOffsetY )
  {
    if( !m_ciSettingsTable.bNeedsRenderInit )
    {
      return;
    }

    local iScreenWidth = ::rumGetScreenWidth();
    local iScreenCenterX = iScreenWidth / 2;

    local iControlWidth = g_ciUI.s_iTextBoxWidth + g_ciUI.s_iMenuWidth;
    local iMenuOffsetX = iScreenCenterX - ( iControlWidth / 2 );
    local iTextOffsetX = iMenuOffsetX + g_ciUI.m_ciTitleListMenu.GetWidth();

    i_iOffsetY += g_ciUI.s_iDefaultLabelHeight * 2;

    // Display menu title
    g_ciUI.m_ciTitleLabel.SetPos( rumPoint( iScreenCenterX - ( g_ciUI.m_ciTitleLabel.GetWidth() / 2 ), i_iOffsetY ) );
    i_iOffsetY += g_ciUI.m_ciTitleLabel.GetHeight() + ( g_ciUI.s_iDefaultLabelHeight * 2 );

    // Display menu
    g_ciUI.m_ciTitleListMenu.SetPos( rumPoint( iMenuOffsetX, i_iOffsetY ) );

    g_ciUI.m_ciTextBoxArray[ConnectMenu.Server].SetPos( rumPoint( iTextOffsetX, i_iOffsetY ) );
    i_iOffsetY += g_ciUI.m_ciTextBoxArray[ConnectMenu.Server].GetHeight();

    g_ciUI.m_ciTextBoxArray[ConnectMenu.Port].SetPos( rumPoint( iTextOffsetX, i_iOffsetY ) );
    i_iOffsetY += g_ciUI.m_ciTextBoxArray[ConnectMenu.Port].GetHeight();

    i_iOffsetY += g_ciUI.s_iDefaultLabelHeight * 2;

    g_ciUI.m_ciInfoLabel.SetPos( rumPoint( iScreenCenterX - ( g_ciUI.m_ciInfoLabel.GetWidth() / 2 ), i_iOffsetY ) );

    m_ciSettingsTable.bNeedsRenderInit = false;
  }
}
