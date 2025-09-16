class AccountLoginTitleStage
{
  m_ciSettingsTable = { bNeedsRenderInit = true };


  function BeginStage()
  {
    if( ::rumIsAccountLoggedIn() )
    {
      EndStage( TitleStages.MainMenu );
    }
    else
    {
      g_ciUI.m_ciInfoLabel.SetActive( true );
      g_ciUI.m_ciTitleLabel.SetActive( true );

      g_ciUI.m_ciTitleListMenu.SetActive( true );
      g_ciUI.m_ciTitleListMenu.Focus();
      g_ciUI.m_ciTitleListMenu.Clear();
      g_ciUI.m_ciTitleListMenu.SetEntry( AccountMenu.Login, ::rumGetString( account_login_client_StringID ) );
      g_ciUI.m_ciTitleListMenu.SetEntry( AccountMenu.CreateAccount, ::rumGetString( account_create_client_StringID ) );
      g_ciUI.m_ciTitleListMenu.SetEntry( AccountMenu.Back, ::rumGetString( token_back_client_StringID ) );
      g_ciUI.m_ciTitleListMenu.SetEntry( AccountMenu.ExitGame, ::rumGetString( title_exit_game_client_StringID ) );
      g_ciUI.m_ciTitleListMenu.SetWidth( g_ciUI.s_iMenuWidth );
      g_ciUI.m_ciTitleListMenu.m_funcAccept = OnListMenuAccepted.bindenv( AccountLoginTitleStage );
      g_ciUI.m_ciTitleListMenu.m_funcCancel = OnListMenuCanceled.bindenv( AccountLoginTitleStage );

      CalculateListViewSize( g_ciUI.m_ciTitleListMenu, "default", 4 );
    }
  }


  function EndStage( i_eNextStage )
  {
    m_ciSettingsTable.bNeedsRenderInit = true;

    g_ciUI.m_ciInfoLabel.SetActive( false );
    g_ciUI.m_ciTitleLabel.SetActive( false );

    g_ciUI.m_ciTitleListMenu.SetActive( false );
    g_ciUI.m_ciTitleListMenu.ClearHandlers();

    InitTitleStage( i_eNextStage );
  }


  function OnListMenuAccepted()
  {
    g_ciUI.m_ciInfoLabel.SetText( "" );

    local iSelectedKey = g_ciUI.m_ciTitleListMenu.GetSelectedKey();

    if( AccountMenu.Login == iSelectedKey )
    {
      g_ciCUO.m_bAccountCreate = false;
      EndStage( TitleStages.Login );
    }
    else if( AccountMenu.CreateAccount == iSelectedKey )
    {
      g_ciCUO.m_bAccountCreate = true;
      EndStage( TitleStages.Login );
    }
    else if( AccountMenu.Back == iSelectedKey )
    {
      g_ciCUO.m_bAccountCreate = false;
      EndStage( TitleStages.Splash );
    }
    else if( AccountMenu.ExitGame == iSelectedKey )
    {
      ::rumShutdown();
    }
  }


  function OnListMenuCanceled()
  {
    EndStage( TitleStages.Splash );
  }


  function Render( i_iOffsetY )
  {
    if( !m_ciSettingsTable.bNeedsRenderInit )
    {
      return;
    }

    local iScreenWidth = ::rumGetScreenWidth();
    local iScreenCenterX = iScreenWidth / 2;

    i_iOffsetY += g_ciUI.s_iDefaultLabelHeight * 2;

    // Display menu title
    g_ciUI.m_ciTitleLabel.SetPos( rumPoint( iScreenCenterX - ( g_ciUI.m_ciTitleLabel.GetWidth() / 2 ), i_iOffsetY ) );
    i_iOffsetY += g_ciUI.m_ciTitleLabel.GetHeight() + ( g_ciUI.s_iDefaultLabelHeight * 2 );

    // Display menu
    g_ciUI.m_ciTitleListMenu.SetPos( rumPoint( iScreenCenterX - ( g_ciUI.m_ciTitleListMenu.GetWidth() / 2 ),
                                               i_iOffsetY ) );
    i_iOffsetY += g_ciUI.m_ciTitleListMenu.GetHeight() + ( g_ciUI.s_iDefaultLabelHeight * 2 );

    g_ciUI.m_ciInfoLabel.SetPos( rumPoint( iScreenCenterX - ( g_ciUI.m_ciInfoLabel.GetWidth() / 2 ), i_iOffsetY ) );

    m_ciSettingsTable.bNeedsRenderInit = false;
  }
}
