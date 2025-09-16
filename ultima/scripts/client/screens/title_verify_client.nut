class VerifyClientTitleStage
{
  m_ciSettingsTable = { bNeedsRenderInit = true };


  function BeginStage()
  {
    local iRequiredMajorVersion = 1;
    local iRequiredMinorVersion = 0;

    local iMajorVersion = rumProgramMajorVersion;
    local iMinorVersion = rumProgramMinorVersion;

    ::rumLog( "Using client version " + iMajorVersion + "." + iMinorVersion + "\n" );

    if( ( iRequiredMajorVersion == iMajorVersion ) && ( iRequiredMinorVersion == iMinorVersion ) )
    {
      EndStage( TitleStages.VerifyUltimaInstall );
      return;
    }

    local strError = format( ::rumGetString( msg_client_version_invalid_long_client_StringID ),
                             iRequiredMajorVersion,
                             iRequiredMinorVersion,
                             iMajorVersion,
                             iMinorVersion );

    g_ciUI.m_ciTitleLabel.SetActive( true );

    g_ciUI.m_ciTitleTextView.SetActive( true );
    g_ciUI.m_ciTitleTextView.Clear();
    g_ciUI.m_ciTitleTextView.PushText( strError );
    g_ciUI.m_ciTitleListMenu.m_funcAccept = OnAccepted.bindenv( VerifyClientTitleStage );
    g_ciUI.m_ciTitleListMenu.m_funcCancel = OnCanceled.bindenv( VerifyClientTitleStage );
    g_ciUI.m_ciTitleListMenu.m_funcKeyPress = OnKeyPressed.bindenv( VerifyClientTitleStage );

    g_ciUI.m_ciInfoLabel.SetActive( true );
    g_ciUI.m_ciInfoLabel.SetText( ::rumGetString( msg_client_version_invalid_short_client_StringID ) );
  }


  function EndStage( i_eNextStage )
  {
    m_ciSettingsTable.bNeedsRenderInit = true;

    g_ciUI.m_ciTitleLabel.SetActive( false );
    g_ciUI.m_ciTitleTextView.SetActive( false );
    g_ciUI.m_ciTitleTextView.ClearHandlers();
    g_ciUI.m_ciInfoLabel.SetActive( false );

    InitTitleStage( i_eNextStage );
  }


  function OnAccepted()
  {
    ::rumShutdown();
  }


  function OnCanceled()
  {
    EndStage( TitleStages.Splash );
  }


  function OnKeyPressed( i_ciKeyInput )
  {
    ::rumShutdown();
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

    g_ciUI.m_ciTitleLabel.SetPos( rumPoint( iScreenCenterX - ( g_ciUI.m_ciTitleLabel.GetWidth() / 2 ), i_iOffsetY ) );
    g_ciUI.m_ciTitleLabel.SetActive( true );

    i_iOffsetY += g_ciUI.s_iDefaultLabelHeight * 2;

    g_ciUI.m_ciTitleTextView.Clear();
    g_ciUI.m_ciTitleTextView.SetPos( rumPoint( iScreenCenterX - ( g_ciUI.m_ciTitleTextView.GetWidth() / 2 ),
                                               i_iOffsetY ) );
    g_ciUI.m_ciTitleTextView.SetActive( true );

    i_iOffsetY += g_ciUI.m_ciTitleTextView.GetHeight() + g_ciUI.s_iDefaultLabelHeight;

    g_ciUI.m_ciInfoLabel.SetPos( rumPoint( iScreenCenterX - ( g_ciUI.m_ciInfoLabel.GetWidth() / 2 ), i_iOffsetY ) );
    g_ciUI.m_ciInfoLabel.SetActive( true );

    m_ciSettingsTable.bNeedsRenderInit = false;
  }
}