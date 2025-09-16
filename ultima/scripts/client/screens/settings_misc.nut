enum SettingsMiscMenu
{
  ShowCombat,
  ShowMoves,
  ShowNames,
  ShowVitals,
  Back
}


function InitSettingsMiscMenu( i_iIndex )
{
  g_ciUI.m_ciSettingsMenuSecondary.Clear();

  local ciPos = g_ciUI.m_ciSettingsMenuSecondary.GetPos();
  ciPos.y = g_ciUI.m_ciSettingsMenu.GetSelectedPos().y;

  g_ciUI.m_ciSettingsMenuSecondary.SetPos( ciPos );

  local strBack = ::rumGetString( token_back_client_StringID );
  local strOn = ::rumGetString( token_on_client_StringID );
  local strOff = ::rumGetString( token_off_client_StringID );
  local strShowCombat = ::rumGetString( token_show_combat_client_StringID );
  local strShowMoves = ::rumGetString( token_show_moves_client_StringID );
  local strShowNames = ::rumGetString( token_show_names_client_StringID );
  local strShowVitals = ::rumGetString( token_show_vitals_client_StringID );

  g_ciUI.m_ciSettingsMenuSecondary.SetEntry( SettingsMiscMenu.ShowCombat,
                                             format( " |%s: %s|", strShowCombat,
                                               g_ciUI.m_bShowCombat ? strOn : strOff ) );
  g_ciUI.m_ciSettingsMenuSecondary.SetEntry( SettingsMiscMenu.ShowMoves,
                                             format( " |%s: %s|", strShowMoves,
                                               g_ciUI.m_bShowMoves ? strOn : strOff ) );
  g_ciUI.m_ciSettingsMenuSecondary.SetEntry( SettingsMiscMenu.ShowNames,
                                             format( " |%s: %s|", strShowNames,
                                               g_ciUI.m_bShowNames ? strOn : strOff ) );
  g_ciUI.m_ciSettingsMenuSecondary.SetEntry( SettingsMiscMenu.ShowVitals,
                                             format( " |%s: %s|", strShowVitals,
                                               g_ciUI.m_bShowVitals ? strOn : strOff ) );
  g_ciUI.m_ciSettingsMenuSecondary.SetEntry( SettingsMiscMenu.Back, format( "<|%s|", strBack ) );

  g_ciUI.m_ciSettingsMenuSecondary.CalcHeight();
  g_ciUI.m_ciSettingsMenuSecondary.SetCurrentIndex( i_iIndex );

  g_ciUI.m_ciSettingsMenuSecondary.SetBackgroundColor( g_ciUI.s_ciColorDarkBlue );  
}


function OnMiscMenuAccepted()
{
  g_ciUI.m_ciSettingsMenuTertiary.SetActive( false );
  g_ciUI.m_ciSettingsMenuTertiary.ClearHandlers();
  g_ciUI.m_ciSettingsSlider.SetActive( false );
  g_ciUI.m_ciSettingsSlider.ClearHandlers;

  local iSelectedKey = g_ciUI.m_ciSettingsMenuSecondary.GetSelectedKey();

  if( SettingsMiscMenu.ShowCombat == iSelectedKey )
  {
    g_ciUI.m_bShowCombat = !g_ciUI.m_bShowCombat;

    // Save this setting
    g_ciCUO.m_ciGameConfigTable["cuo:show_combat"] <- g_ciUI.m_bShowCombat;
    ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );

    InitSettingsMiscMenu( g_ciUI.m_ciSettingsMenuSecondary.GetSelectedIndex() );
  }
  else if( SettingsMiscMenu.ShowMoves == iSelectedKey )
  {
    g_ciUI.m_bShowMoves = !g_ciUI.m_bShowMoves;

    // Save this setting
    g_ciCUO.m_ciGameConfigTable["cuo:show_moves"] <- g_ciUI.m_bShowMoves;
    ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );

    InitSettingsMiscMenu( g_ciUI.m_ciSettingsMenuSecondary.GetSelectedIndex() );
  }
  else if( SettingsMiscMenu.ShowNames == iSelectedKey )
  {
    g_ciUI.m_bShowNames = !g_ciUI.m_bShowNames;

    // Save this setting
    g_ciCUO.m_ciGameConfigTable["cuo:show_names"] <- g_ciUI.m_bShowNames;
    ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );

    InitSettingsMiscMenu( g_ciUI.m_ciSettingsMenuSecondary.GetSelectedIndex() );
  }
  else if( SettingsMiscMenu.ShowVitals == iSelectedKey )
  {
    g_ciUI.m_bShowVitals = !g_ciUI.m_bShowVitals;

    // Save this setting
    g_ciCUO.m_ciGameConfigTable["cuo:show_vitals"] <- g_ciUI.m_bShowVitals;
    ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );

    InitSettingsMiscMenu( g_ciUI.m_ciSettingsMenuSecondary.GetSelectedIndex() );
  }
  else if( SettingsMiscMenu.Back == iSelectedKey )
  {
    CloseMenu( g_ciUI.m_ciSettingsMenuSecondary );
  }
}


function OnMiscMenuCanceled()
{
  CloseMenu( g_ciUI.m_ciSettingsMenuSecondary );
}


function OnMiscMenuKeyPressed( i_ciKeyInput )
{
  local eKey = i_ciKeyInput.GetKey();

  if( rumKeypress.KeyLeft() == eKey )
  {
    CloseMenu( g_ciUI.m_ciSettingsMenuSecondary );
  }
}


function OpenSettingsMiscMenu()
{
  g_ciUI.m_ciSettingsMenu.SetBackgroundColor( g_ciUI.s_ciColorLightBlue );

  InitSettingsMiscMenu( 0 );

  g_ciUI.m_ciSettingsMenuSecondary.SetActive( true );
  g_ciUI.m_ciSettingsMenuSecondary.m_funcAccept = OnMiscMenuAccepted;
  g_ciUI.m_ciSettingsMenuSecondary.m_funcCancel = OnMiscMenuCanceled;
  g_ciUI.m_ciSettingsMenuSecondary.m_funcKeyPress = OnMiscMenuKeyPressed;
  g_ciUI.m_ciSettingsMenuSecondary.Focus();
}
