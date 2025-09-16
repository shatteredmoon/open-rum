class VerifyPatchTitleStage
{
  m_ciSettingsTable = { bNeedsRenderInit = true };


  function BeginStage()
  {
    ::rumLog( "Verifying game files...\n" );

    // Cache for later because databases will be closed during patching
    ::rumGetString( msg_client_restarting_client_StringID );

    g_ciUI.m_ciInfoLabel.SetActive( true );
    g_ciUI.m_ciInfoLabel.SetText( ::rumGetString( msg_client_patching_client_StringID ) );

    g_ciUI.m_ciTitleLabel.SetActive( true );

    g_ciUI.m_ciTitleTextView.SetActive( true );
    g_ciUI.m_ciTitleTextView.Clear();

    ::rumStartPatch();
  }


  function EndStage()
  {
    m_ciSettingsTable.bNeedsRenderInit = true;

    g_ciCUO.m_strPatchFileTable.clear();

    g_ciUI.m_ciInfoLabel.SetActive( false );
    g_ciUI.m_ciTitleLabel.SetActive( false );
    g_ciUI.m_ciTitleTextView.SetActive( false );

    InitTitleStage( TitleStages.AccountMenu );
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

    g_ciUI.m_ciTitleTextView.SetPos( rumPoint( iScreenCenterX - ( g_ciUI.m_ciTitleTextView.GetWidth() / 2 ), i_iOffsetY ) );
    i_iOffsetY += g_ciUI.m_ciTitleTextView.GetHeight() + g_ciUI.s_iDefaultLabelHeight;

    g_ciUI.m_ciInfoLabel.SetPos( rumPoint( iScreenCenterX - ( g_ciUI.m_ciInfoLabel.GetWidth() / 2 ), i_iOffsetY ) );

    m_ciSettingsTable.bNeedsRenderInit = true;
  }


  function Update( i_strFile, i_uiPercent )
  {
    g_ciCUO.m_strPatchFileTable[i_strFile] <- i_uiPercent;

    g_ciUI.m_ciTitleTextView.Clear();

    foreach( strFile in g_ciCUO.m_strPatchFileTable )
    {
      local strEntry = format( "%s: %d%%", i_strFile, i_uiPercent );
      g_ciUI.m_ciTitleTextView.PushText( strEntry );
    }
  }
}
