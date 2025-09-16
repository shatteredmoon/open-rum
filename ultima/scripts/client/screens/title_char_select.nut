class CharacterSelectTitleStage
{
  m_ciSettingsTable = { bNeedsRenderInit = true };


  function BeginStage()
  {
    g_ciUI.m_ciInfoLabel.SetActive( true );
    g_ciUI.m_ciInfoLabel.SetText( "" );

    g_ciUI.m_ciTitleLabel.SetActive( true );

    local strNameArray = ::rumGetAccountCharacters();
    local iIndex = 0;

    g_ciUI.m_ciTitleListMenu.SetActive( true );
    g_ciUI.m_ciTitleListMenu.Focus();
    g_ciUI.m_ciTitleListMenu.Clear();

    foreach( strName in strNameArray )
    {
      g_ciUI.m_ciTitleListMenu.SetEntry( iIndex++, strName );
    }

    g_ciUI.m_ciTitleListMenu.SetEntry( iIndex, ::rumGetString( token_cancel_client_StringID ) );
    g_ciUI.m_ciTitleListMenu.SetWidth( g_ciUI.s_iMenuWidth );
    g_ciUI.m_ciTitleListMenu.m_funcAccept = OnListMenuAccepted.bindenv( CharacterSelectTitleStage );
    g_ciUI.m_ciTitleListMenu.m_funcCancel = OnListMenuCanceled.bindenv( CharacterSelectTitleStage );

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

    local strNameArray = ::rumGetAccountCharacters();
    local strPlayerName = g_ciUI.m_ciTitleListMenu.GetEntry( iSelectedKey );
    if( strNameArray.len() <= iSelectedKey )
    {
      // Player selected 'Cancel' or invalid selection
      g_ciCUO.m_bCharacterDelete = false;
      EndStage( TitleStages.MainMenu );
      return;
    }

    if( g_ciCUO.m_bCharacterDelete )
    {
      g_ciCUO.m_bCharacterDelete = false;
      g_ciCUO.m_strCharacterDeleteName = strPlayerName;

      // Verify the deletion
      EndStage( TitleStages.DeleteChar );
    }
    else
    {
      g_ciCUO.m_bWaitingForResponse = true;

      ::rumPlayerLogin( strPlayerName );

      g_ciCUO.m_eCurrentGameMode = GameMode.Transition;

      // Position the daemon and dragon completely on the title screen
      g_ciUI.m_iTitleAnimationOffset = 0;

      // Clear pre-existing game activity for the new player
      g_ciUI.m_ciChatTextView.Clear();
      g_ciUI.m_ciGameTextView.Clear();

      EndStage( TitleStages.Done );
    }
  }


  function OnListMenuCanceled()
  {
    g_ciCUO.m_bCharacterDelete = false;
    EndStage( TitleStages.MainMenu );
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
