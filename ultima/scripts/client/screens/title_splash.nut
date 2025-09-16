class SplashTitleStage
{
  m_ciSettingsTable = { bNeedsRenderInit = true };


  function BeginStage()
  {
    // Hide all game controls
    g_ciUI.m_ciChatTextView.SetActive( false );
    g_ciUI.m_ciExplorerDriveListView.SetActive( false );
    g_ciUI.m_ciExplorerFileListView.SetActive( false );
    g_ciUI.m_ciFoodLabel.SetActive( false );
    g_ciUI.m_ciGameInputTextBox.SetActive( false );
    g_ciUI.m_ciGameListView.SetActive( false );
    g_ciUI.m_ciGameRegion.SetActive( false );
    g_ciUI.m_ciGameTextView.SetActive( false );
    g_ciUI.m_ciGoldLabel.SetActive( false );
    g_ciUI.m_ciMapNameLabel.SetActive( false );
    g_ciUI.m_ciStatLabel.SetActive( false );
    g_ciUI.m_ciStatListView.SetActive( false );

    g_ciUI.m_ciSplashLabel.SetActive( true );

    g_ciUI.m_ciScreenRegion.SetActive( true );
    g_ciUI.m_ciScreenRegion.m_funcKeyPress = OnKeyPressed.bindenv( SplashTitleStage );
    g_ciUI.m_ciScreenRegion.m_funcMouseButtonPress = OnMouseButtonPressed.bindenv( SplashTitleStage );
    g_ciUI.m_ciScreenRegion.Focus();
  }


  function EndStage()
  {
    m_ciSettingsTable.bNeedsRenderInit = true;

    g_ciUI.m_ciSplashLabel.SetActive( false );
    g_ciUI.m_ciScreenRegion.SetActive( false );
    g_ciUI.m_ciScreenRegion.ClearHandlers();

    InitTitleStage( TitleStages.VerifyClientVersion );
  }


  function OnKeyPressed( i_ciKeyInput )
  {
    if( rumKeypress.KeyEscape() != i_ciKeyInput.GetKey() )
    {
      EndStage();
    }
  }


  function OnMouseButtonPressed( i_eButton, i_ciPoint )
  {
    if( g_ciUI.m_ciSettingsMenu.IsActive() )
    {
      return;
    }

    EndStage();
  }


  function Render( i_iOffsetY )
  {
    local iScreenWidth = ::rumGetScreenWidth();
    local iScreenCenterX = iScreenWidth / 2;

    i_iOffsetY += g_ciUI.s_iDefaultLabelHeight;

    // Splash graphic
    local ciGraphic = ::rumGetGraphic( U4_Title_Splash_GraphicID );
    local iSplashOffsetX = iScreenCenterX - ( ciGraphic.GetFrameWidth() / 2 );
    ciGraphic.Draw( rumPoint( iSplashOffsetX, i_iOffsetY ) );

    if( m_ciSettingsTable.bNeedsRenderInit )
    {
      local iScreenHeight = ::rumGetScreenHeight();

      local ciPos = rumPoint( iSplashOffsetX + ciGraphic.GetFrameWidth() - g_ciUI.m_ciSplashLabel.GetWidth(),
                              iScreenHeight - g_ciUI.s_iDefaultLabelHeight - g_ciUI.m_ciSplashLabel.GetHeight() );
      g_ciUI.m_ciSplashLabel.SetPos( ciPos );

      m_ciSettingsTable.bNeedsRenderInit = false;
    }
  }
}