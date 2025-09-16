class VerifyUltimaTitleStage
{
  m_ciSettingsTable = { bNeedsRenderInit = true };


  function BeginStage()
  {
    g_ciUI.m_eVerificationVersion = GameType.Ultima4;

    if( VerifyUltima( g_ciUI.m_eVerificationVersion ) )
    {
      EndStage( TitleStages.Connect );
      return;
    }

    g_ciUI.m_ciTitleLabel.SetActive( true );

    // Explain to the player that an original copy of Ultima 4 is required
    g_ciUI.m_ciTitleTextView.SetActive( true );
    g_ciUI.m_ciTitleTextView.Clear();
    g_ciUI.m_ciTitleTextView.PushText( ::rumGetString( u4_install_req_client_StringID ) );

    // An error showing that the last checked location for Ultima 4 is invalid
    g_ciUI.m_ciInfoLabel.SetActive( true );
    g_ciUI.m_ciInfoLabel.SetText( format( ::rumGetString( msg_location_invalid_client_StringID ), GameType.Ultima4 ) );

    local iTitleWidth = g_ciUI.m_ciTitleTextView.GetWidth();
    local iFileListWidth = ( iTitleWidth * 0.9 ).tointeger();
    local iDriveListWidth = iTitleWidth - iFileListWidth;

    g_ciUI.m_ciExplorerDriveListView.SetActive( false );
    g_ciUI.m_ciExplorerDriveListView.Clear();
    g_ciUI.m_ciExplorerDriveListView.SetWidth( iDriveListWidth );
    g_ciUI.m_ciExplorerDriveListView.SetHeight( 160 );
    g_ciUI.m_ciExplorerDriveListView.SetFormat( "0.0|0.45" );

    g_ciUI.m_ciExplorerFileListView.SetActive( false );
    g_ciUI.m_ciExplorerFileListView.Clear();
    g_ciUI.m_ciExplorerFileListView.SetWidth( iFileListWidth );
    g_ciUI.m_ciExplorerFileListView.SetHeight( 160 );
    g_ciUI.m_ciExplorerFileListView.SetFormat( "0.0|0.05" );

    g_ciUI.m_ciTitleListMenu.SetActive( true );
    g_ciUI.m_ciTitleListMenu.Focus();
    g_ciUI.m_ciTitleListMenu.Clear();
    g_ciUI.m_ciTitleListMenu.SetEntry( VerifyUltimaMenu.Path, ::rumGetString( token_path_client_StringID ) );
    g_ciUI.m_ciTitleListMenu.SetEntry( VerifyUltimaMenu.Browse, ::rumGetString( token_browse_client_StringID ) );
    g_ciUI.m_ciTitleListMenu.SetEntry( VerifyUltimaMenu.Exit, ::rumGetString( title_exit_game_client_StringID ) );
    g_ciUI.m_ciTitleListMenu.SetWidth( g_ciUI.s_iMenuWidth );
    g_ciUI.m_ciTitleListMenu.m_funcAccept = OnListMenuAccepted.bindenv( VerifyUltimaTitleStage );
    g_ciUI.m_ciTitleListMenu.m_funcCancel = OnListMenuCanceled.bindenv( VerifyUltimaTitleStage );
    g_ciUI.m_ciTitleListMenu.m_funcKeyPress = OnListMenuKeyPressed.bindenv( VerifyUltimaTitleStage );

    CalculateListViewSize( g_ciUI.m_ciTitleListMenu, "default", 3 );

    // Set the text box entry to the last known location (or guess) of where Ultima 4 is installed
    local ciTextBox = g_ciUI.m_ciTextBoxArray[VerifyUltimaMenu.Path];
    ciTextBox.SetText( g_ciCUO.m_ciGameConfigTable["cuo:ultima4"] );
    ciTextBox.ObscureText( false );
    ciTextBox.SetActive( true );
    ciTextBox.m_funcAccept = OnTextBoxAccepted.bindenv( VerifyUltimaTitleStage );
    ciTextBox.m_funcCancel = OnTextBoxCanceled.bindenv( VerifyUltimaTitleStage );
  }


  function CancelBrowse()
  {
    g_ciUI.m_ciExplorerDriveListView.SetActive( false );
    g_ciUI.m_ciExplorerFileListView.SetActive( false );

    g_ciUI.m_ciTitleTextView.SetActive( true );
    g_ciUI.m_ciTitleListMenu.SetActive( true );
    g_ciUI.m_ciTitleListMenu.Focus();

    g_ciUI.m_ciTextBoxArray[VerifyUltimaMenu.Path].SetActive( true );

    m_ciSettingsTable.bNeedsRenderInit = true;
  }


  function EndStage( i_eNextStage )
  {
    m_ciSettingsTable.bNeedsRenderInit = true;

    g_ciUI.m_ciInfoLabel.SetActive( false );
    g_ciUI.m_ciInfoLabel.SetText( "" );

    g_ciUI.m_ciExplorerDriveListView.SetActive( false );
    g_ciUI.m_ciExplorerDriveListView.ClearHandlers();
    g_ciUI.m_ciExplorerFileListView.SetActive( false );
    g_ciUI.m_ciExplorerFileListView.ClearHandlers();

    g_ciUI.m_ciTextBoxArray[VerifyUltimaMenu.Path].SetActive( false );
    g_ciUI.m_ciTextBoxArray[VerifyUltimaMenu.Path].ClearHandlers();
    g_ciUI.m_ciTitleLabel.SetActive( false );
    g_ciUI.m_ciTitleListMenu.SetActive( false );
    g_ciUI.m_ciTitleListMenu.ClearHandlers();
    g_ciUI.m_ciTitleTextView.SetActive( false );

    InitTitleStage( i_eNextStage );
  }


  function OnDriveListMenuCanceled()
  {
    ::OnDriveListMenuCanceled();
    CancelBrowse();
  }


  function OnFileListMenuCanceled()
  {
    ::OnFileListMenuCanceled();
    CancelBrowse();
  }


  function OnListMenuAccepted()
  {
    g_ciUI.m_ciInfoLabel.SetText( "" );

    local iSelectedKey = g_ciUI.m_ciTitleListMenu.GetSelectedKey();

    if( VerifyUltimaMenu.Path == iSelectedKey )
    {
      g_ciUI.m_ciTextBoxArray[VerifyUltimaMenu.Path].Focus();
    }
    else if( VerifyUltimaMenu.Browse == iSelectedKey )
    {
      OpenFileExplorer();
    }
    else if( VerifyUltimaMenu.Exit == iSelectedKey )
    {
      ::rumShutdown();
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

    if( ( rumKeypress.KeyRight() == eKey ) || ( rumKeypress.KeyTab() == eKey ) )
    {
      g_ciUI.m_ciTextBoxArray[VerifyUltimaMenu.Path].Focus();
    }
  }


  function OnTextBoxAccepted()
  {
    g_ciUI.m_ciTitleListMenu.Focus();
    SetUltimaPath( g_ciUI.m_eVerificationVersion, g_ciUI.m_ciTextBoxArray[VerifyUltimaMenu.Path].GetText() );
  }


  function OnTextBoxCanceled()
  {
    g_ciUI.m_ciTitleListMenu.Focus();
  }


  function OpenFileExplorer()
  {
    g_ciUI.m_ciTextBoxArray[VerifyUltimaMenu.Path].SetActive( false );
    g_ciUI.m_ciTitleTextView.SetActive( false );
    g_ciUI.m_ciTitleListMenu.SetActive( false );
    g_ciUI.m_ciExplorerDriveListView.SetActive( true );
    g_ciUI.m_ciExplorerFileListView.SetActive( true );
    g_ciUI.m_ciExplorerFileListView.Focus();

    // Add special handling for canceling selection
    UpdateDriveListView();
    g_ciUI.m_ciExplorerDriveListView.m_funcCancel = OnDriveListMenuCanceled.bindenv( VerifyUltimaTitleStage );

    // Add special handling for canceling selection
    UpdateFileListView();
    g_ciUI.m_ciExplorerFileListView.m_funcCancel = OnFileListMenuCanceled.bindenv( VerifyUltimaTitleStage );

    m_ciSettingsTable.bNeedsRenderInit = true;

    local strUltima = format( "cuo:ultima%d", g_ciCUO.m_eVersion );
    g_ciUI.m_ciInfoLabel.SetText( g_ciCUO.m_ciGameConfigTable[strUltima] );
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
    i_iOffsetY += g_ciUI.s_iDefaultLabelHeight * 2;

    local ciPos = rumPoint( iScreenCenterX - ( g_ciUI.m_ciTitleTextView.GetWidth() / 2 ), i_iOffsetY )

    if( g_ciUI.m_ciExplorerDriveListView.IsActive() && g_ciUI.m_ciExplorerFileListView.IsActive() )
    {
      g_ciUI.m_ciExplorerDriveListView.SetPos( ciPos );

      ciPos = rumPoint( g_ciUI.m_ciExplorerDriveListView.GetPosX() + g_ciUI.m_ciExplorerDriveListView.GetWidth(),
                        i_iOffsetY );
      g_ciUI.m_ciExplorerFileListView.SetPos( ciPos );

      i_iOffsetY += g_ciUI.m_ciExplorerDriveListView.GetHeight() + g_ciUI.s_iDefaultLabelHeight;
    }
    else
    {
      // Show that the original game executable could not be found and explain that the path to the original
      // installation must be provided
      g_ciUI.m_ciTitleTextView.SetPos( ciPos );
      i_iOffsetY += g_ciUI.m_ciTitleTextView.GetHeight() + g_ciUI.s_iDefaultLabelHeight;

      local iWidth = g_ciUI.s_iTextBoxWidth + g_ciUI.s_iMenuWidth;
      local iMenuOffsetX = iScreenCenterX - iWidth / 2;

      // Display menu
      g_ciUI.m_ciTitleListMenu.SetPos( rumPoint( iMenuOffsetX, i_iOffsetY ) );

      local iTextOffsetX = iMenuOffsetX + g_ciUI.m_ciTitleListMenu.GetWidth();

      // Text box for input
      g_ciUI.m_ciTextBoxArray[VerifyUltimaMenu.Path].SetPos( rumPoint( iTextOffsetX, i_iOffsetY ) );
      i_iOffsetY += g_ciUI.m_ciTextBoxArray[VerifyUltimaMenu.Path].GetHeight();

      i_iOffsetY += g_ciUI.s_iDefaultLabelHeight * 3;
    }

    g_ciUI.m_ciInfoLabel.SetPos( rumPoint( iScreenCenterX - ( g_ciUI.m_ciInfoLabel.GetWidth() / 2 ), i_iOffsetY ) );

    m_ciSettingsTable.bNeedsRenderInit = false;
  }
}
