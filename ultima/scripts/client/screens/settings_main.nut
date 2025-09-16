enum SettingsMenu
{
  Display,
  Audio,
  Misc,
  Close,
  Quit
}


function CloseMenu( i_ciMenu )
{
  if( i_ciMenu != null )
  {
    i_ciMenu.SetActive( false );
    i_ciMenu.ClearHandlers();
  }

  CloseSettingsSlider();

  if( i_ciMenu == g_ciUI.m_ciSettingsMenuTertiary )
  {
    g_ciUI.m_ciSettingsMenuSecondary.SetBackgroundColor( g_ciUI.s_ciColorDarkBlue );
    g_ciUI.m_ciSettingsMenuSecondary.Focus();
  }
  else if( i_ciMenu == g_ciUI.m_ciSettingsMenuSecondary )
  {
    CloseMenu( g_ciUI.m_ciSettingsMenuTertiary );

    g_ciUI.m_ciSettingsMenu.SetBackgroundColor( g_ciUI.s_ciColorDarkBlue );
    g_ciUI.m_ciSettingsMenu.Focus();
  }
  else if( i_ciMenu == g_ciUI.m_ciSettingsMenu )
  {
    CloseMenu( g_ciUI.m_ciSettingsMenuTertiary );
    CloseMenu( g_ciUI.m_ciSettingsMenuSecondary );

    ::rumFocusNextControl();
  }
}


function CloseSettingsMenu()
{
  CloseMenu( g_ciUI.m_ciSettingsMenu );
}


function CloseSettingsSlider()
{
  g_ciUI.m_ciSettingsSlider.SetActive( false );
  g_ciUI.m_ciSettingsSlider.ClearHandlers();
  g_ciUI.m_ciSettingsMenuSecondary.SetBackgroundColor( g_ciUI.s_ciColorDarkBlue );
  g_ciUI.m_ciSettingsMenuTertiary.SetBackgroundColor( g_ciUI.s_ciColorDarkBlue );
  g_ciUI.m_ciSettingsMenuSecondary.Focus();
}


function InitSettings()
{
  g_ciUI.m_ciSettingsMenu = ::rumCreateControl( ListView );
  g_ciUI.m_ciSettingsMenu.SetBackgroundColor( g_ciUI.s_ciColorDarkBlue );
  g_ciUI.m_ciSettingsMenu.SetWidth( 128 );
  g_ciUI.m_ciSettingsMenu.ShowScrollbar( false );
  g_ciUI.m_ciSettingsMenu.SetFormat( "0.05|0.15|2.95" );
  g_ciUI.m_ciSettingsMenu.SetActive( false );

  local strAudio = ::rumGetString( token_audio_client_StringID );
  local strClose = ::rumGetString( token_close_client_StringID );
  local strExit = ::rumGetString( title_exit_game_client_StringID )
  local strDisplay = ::rumGetString( token_display_client_StringID );
  local strMisc = ::rumGetString( token_miscellaneous_client_StringID );

  g_ciUI.m_ciSettingsMenu.SetEntry( SettingsMenu.Display, format( " |%s|>", strDisplay ) );
  g_ciUI.m_ciSettingsMenu.SetEntry( SettingsMenu.Audio, format( " |%s|>", strAudio ) );
  g_ciUI.m_ciSettingsMenu.SetEntry( SettingsMenu.Misc, format( " |%s|>", strMisc ) );
  g_ciUI.m_ciSettingsMenu.SetEntry( SettingsMenu.Close, format( "<|%s|", strClose ) );
  g_ciUI.m_ciSettingsMenu.SetEntry( SettingsMenu.Quit, format( " |%s|", strExit ) );

  g_ciUI.m_ciSettingsMenu.CalcHeight();

  g_ciUI.m_ciSettingsMenuSecondary = ::rumCreateControl( ListView );
  g_ciUI.m_ciSettingsMenuSecondary.SetBackgroundColor( g_ciUI.s_ciColorDarkBlue );
  g_ciUI.m_ciSettingsMenuSecondary.SetWidth( 160 );
  g_ciUI.m_ciSettingsMenuSecondary.ShowScrollbar( false );
  g_ciUI.m_ciSettingsMenuSecondary.SetFormat( "0.05|0.15|2.95" );
  g_ciUI.m_ciSettingsMenuSecondary.SetActive( false );

  local ciPos = g_ciUI.m_ciSettingsMenu.GetPos();
  ciPos.x += g_ciUI.m_ciSettingsMenu.GetWidth();
  g_ciUI.m_ciSettingsMenuSecondary.SetPos( ciPos );

  g_ciUI.m_ciSettingsMenuTertiary = ::rumCreateControl( ListView );
  g_ciUI.m_ciSettingsMenuTertiary.SetBackgroundColor( g_ciUI.s_ciColorDarkBlue );
  g_ciUI.m_ciSettingsMenuTertiary.SetWidth( 128 );
  g_ciUI.m_ciSettingsMenuTertiary.ShowScrollbar( false );
  g_ciUI.m_ciSettingsMenuTertiary.SetFormat( "0.05|0.15|2.95" );
  g_ciUI.m_ciSettingsMenuTertiary.SetActive( false );

  ciPos = g_ciUI.m_ciSettingsMenuSecondary.GetPos();
  ciPos.x += g_ciUI.m_ciSettingsMenuSecondary.GetWidth();
  g_ciUI.m_ciSettingsMenuTertiary.SetPos( ciPos );

  g_ciUI.m_ciSettingsSlider = ::rumCreateControl( Slider );
  g_ciUI.m_ciSettingsSlider.SetBackgroundColor( g_ciUI.s_ciColorDarkBlue );
  g_ciUI.m_ciSettingsSlider.SetWidth( 128 );
  g_ciUI.m_ciSettingsSlider.SetHeight( g_ciUI.s_iDefaultLabelHeight );
  g_ciUI.m_ciSettingsSlider.AlignCenter();
  g_ciUI.m_ciSettingsSlider.SetActive( false );
}


function OnSettingsMenuAccepted()
{
  g_ciUI.m_ciSettingsMenuSecondary.SetActive( false );
  g_ciUI.m_ciSettingsMenuSecondary.ClearHandlers();
  g_ciUI.m_ciSettingsMenuTertiary.SetActive( false );
  g_ciUI.m_ciSettingsMenuTertiary.ClearHandlers();
  g_ciUI.m_ciSettingsSlider.SetActive( false );
  g_ciUI.m_ciSettingsSlider.ClearHandlers;

  local iSelectedKey = g_ciUI.m_ciSettingsMenu.GetSelectedKey();

  if( SettingsMenu.Display == iSelectedKey )
  {
    OpenSettingsDisplayMenu();
  }
  else if( SettingsMenu.Audio == iSelectedKey )
  {
    OpenSettingsAudioMenu();
  }
  else if( SettingsMenu.Misc == iSelectedKey )
  {
    OpenSettingsMiscMenu();
  }
  else if( SettingsMenu.Close == iSelectedKey )
  {
    CloseMenu( g_ciUI.m_ciSettingsMenu );
  }
  else if( SettingsMenu.Quit == iSelectedKey )
  {
    ::rumShutdown();
  }
}


function OnSettingsMenuCanceled()
{
  CloseMenu( g_ciUI.m_ciSettingsMenu );
}


function OnSettingsMenuKeyPressed( i_ciKeyInput )
{
  local eKey = i_ciKeyInput.GetKey();

  if( rumKeypress.KeyRight() == eKey )
  {
    local iSelectedKey = g_ciUI.m_ciSettingsMenu.GetSelectedKey();

    if( SettingsMenu.Display == iSelectedKey )
    {
      OpenSettingsDisplayMenu();
    }
    else if( SettingsMenu.Audio == iSelectedKey )
    {
      OpenSettingsAudioMenu();
    }
    else if( SettingsMenu.Misc == iSelectedKey )
    {
      OpenSettingsMiscMenu();
    }
  }
  else if( rumKeypress.KeyLeft() == eKey )
  {
    CloseMenu( g_ciUI.m_ciSettingsMenu );
  }
}


function OpenSettingsMenu()
{
  g_ciUI.m_ciSettingsMenu.SetActive( true );
  g_ciUI.m_ciSettingsMenu.SetBackgroundColor( g_ciUI.s_ciColorDarkBlue );
  g_ciUI.m_ciSettingsMenu.m_funcAccept = OnSettingsMenuAccepted;
  g_ciUI.m_ciSettingsMenu.m_funcCancel = OnSettingsMenuCanceled;
  g_ciUI.m_ciSettingsMenu.m_funcKeyPress = OnSettingsMenuKeyPressed;
  g_ciUI.m_ciSettingsMenu.Focus();
}
