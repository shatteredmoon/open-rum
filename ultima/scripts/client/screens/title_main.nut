class MainMenuTitleStage
{
  m_ciSettingsTable = { bNeedsRenderInit = true };


  function BeginStage()
  {
    g_ciCUO.m_bCharacterDelete = false;

    g_ciUI.m_ciInfoLabel.SetActive( true );
    g_ciUI.m_ciInfoLabel.SetText( "" );

    g_ciUI.m_ciTitleLabel.SetActive( true );

    g_ciUI.m_ciTitleListMenu.SetActive( true );
    g_ciUI.m_ciTitleListMenu.Focus();
    g_ciUI.m_ciTitleListMenu.Clear();
    g_ciUI.m_ciTitleListMenu.SetEntry( MainMenu.JourneyOnward, ::rumGetString( title_journey_onward_client_StringID ) );
    g_ciUI.m_ciTitleListMenu.SetEntry( MainMenu.InitiateGame, ::rumGetString( title_initiate_game_client_StringID ) );
    g_ciUI.m_ciTitleListMenu.SetEntry( MainMenu.DeleteChar, ::rumGetString( title_delete_character_client_StringID ) );
    g_ciUI.m_ciTitleListMenu.SetEntry( MainMenu.Logout, ::rumGetString( title_logout_client_StringID ) );
    g_ciUI.m_ciTitleListMenu.SetEntry( MainMenu.Quit, ::rumGetString( title_exit_game_client_StringID ) );
    g_ciUI.m_ciTitleListMenu.SetWidth( g_ciUI.s_iMenuWidth );
    g_ciUI.m_ciTitleListMenu.m_funcAccept = OnListMenuAccepted.bindenv( MainMenuTitleStage );
    g_ciUI.m_ciTitleListMenu.m_funcCancel = OnListMenuCanceled.bindenv( MainMenuTitleStage );

    CalculateListViewSize( g_ciUI.m_ciTitleListMenu, "default", 5 );
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

    if( MainMenu.JourneyOnward == iSelectedKey )
    {
      local iNumCharacters = ::rumGetNumAccountCharacters();
      if( iNumCharacters > 0 )
      {
        EndStage( TitleStages.CharSelect );
      }
      else
      {
        // No characters on this account, so start character generation instead
        g_ciCUO.m_eCurrentGameMode = GameMode.CharGen;
        EndStage( TitleStages.Done );
        InitCharGen();
      }
    }
    else if( MainMenu.InitiateGame == iSelectedKey )
    {
      // Determine if there's room for another character
      local iNumCharacters = ::rumGetNumAccountCharacters();
      if( iNumCharacters < g_ciCUO.s_iMaxCharactersPerAccount )
      {
        // Room available, so off to character generation
        g_ciCUO.m_eCurrentGameMode = GameMode.CharGen;
        EndStage( TitleStages.Done );
        InitCharGen();
      }
      else
      {
        g_ciUI.m_ciInfoLabel.SetText( ::rumGetString( chargen_slots_full_client_StringID ) );
      }
    }
    else if( MainMenu.DeleteChar == iSelectedKey )
    {
      local iNumCharacters = ::rumGetNumAccountCharacters();
      if( iNumCharacters > 0 )
      {
        g_ciCUO.m_bCharacterDelete = true;
        EndStage( TitleStages.CharSelect );
      }
      else
      {
        g_ciUI.m_ciInfoLabel.SetText( ::rumGetString( chargen_slots_empty_client_StringID ) );
      }
    }
    else if( MainMenu.Logout == iSelectedKey )
    {
      ::rumAccountLogout();
      EndStage( TitleStages.AccountMenu );
    }
    else if( MainMenu.Quit == iSelectedKey )
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
