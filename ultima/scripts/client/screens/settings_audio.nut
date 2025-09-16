enum SettingsAudioMenu
{
  MusicEnabled
  SFXEnabled,
  MasterVolume,
  MusicVolume,
  SFXVolume,
  SoundPack,
  Back
}

g_aSoundPacksArray <-
[
  [ "PC", "sounds_pc.pkg" ],
  [ "C64", "sounds_c64.pkg" ]
]


function GetMusicVolume()
{
  return ( g_ciCUO.m_uiMasterVolume / 100.0 ) * ( g_ciCUO.m_uiMusicVolume / 100.0 );
}


function GetSFXVolume()
{
  return ( g_ciCUO.m_uiMasterVolume / 100.0 ) * ( g_ciCUO.m_uiSFXVolume / 100.0 );
}


function InitMasterVolumerSlider()
{
  local ciPos = g_ciUI.m_ciSettingsMenuTertiary.GetPos();
  ciPos.y = g_ciUI.m_ciSettingsMenuSecondary.GetSelectedPos().y;

  g_ciUI.m_ciSettingsSlider.SetPos( ciPos );
  g_ciUI.m_ciSettingsSlider.SetValueRange( 1, 100 );
  g_ciUI.m_ciSettingsSlider.SetValue( g_ciCUO.m_uiMasterVolume );
}


function InitMusicVolumerSlider()
{
  local ciPos = g_ciUI.m_ciSettingsMenuTertiary.GetPos();
  ciPos.y = g_ciUI.m_ciSettingsMenuSecondary.GetSelectedPos().y;

  g_ciUI.m_ciSettingsSlider.SetPos( ciPos );
  g_ciUI.m_ciSettingsSlider.SetValueRange( 1, 100 );
  g_ciUI.m_ciSettingsSlider.SetValue( g_ciCUO.m_uiMusicVolume );
}


function InitSettingsAudioMenu( i_iIndex )
{
  g_ciUI.m_ciSettingsMenuSecondary.Clear();

  local ciPos = g_ciUI.m_ciSettingsMenuSecondary.GetPos();
  ciPos.y = g_ciUI.m_ciSettingsMenu.GetSelectedPos().y;

  g_ciUI.m_ciSettingsMenuSecondary.SetPos( ciPos );

  local strVolume = ::rumGetString( token_volume_client_StringID );

  local strBack = ::rumGetString( token_back_client_StringID );
  local strMasterVolume = format( "%s %s", ::rumGetString( token_master_client_StringID ), strVolume );
  local strMusic = ::rumGetString( token_music_client_StringID );
  local strMusicVolume = format( "%s %s", strMusic, strVolume );
  local strOn = ::rumGetString( token_on_client_StringID );
  local strOff = ::rumGetString( token_off_client_StringID );
  local strSFX = ::rumGetString( token_sfx_client_StringID );
  local strSFXVolume = format( "%s %s", strSFX, strVolume );
  local strSoundPack = ::rumGetString( token_sound_pack_client_StringID );

  g_ciUI.m_ciSettingsMenuSecondary.SetEntry( SettingsAudioMenu.MusicEnabled,
                                             format( " |%s: %s|",
                                               strMusic, g_ciCUO.m_bMusicEnabled ? strOn : strOff ) );
  g_ciUI.m_ciSettingsMenuSecondary.SetEntry( SettingsAudioMenu.SFXEnabled,
                                             format( " |%s: %s|", strSFX, g_ciCUO.m_bSFXEnabled ? strOn : strOff ) );
  g_ciUI.m_ciSettingsMenuSecondary.SetEntry( SettingsAudioMenu.MasterVolume, format( " |%s|>", strMasterVolume ) );
  g_ciUI.m_ciSettingsMenuSecondary.SetEntry( SettingsAudioMenu.MusicVolume, format( " |%s|>", strMusicVolume ) );
  g_ciUI.m_ciSettingsMenuSecondary.SetEntry( SettingsAudioMenu.SFXVolume, format( " |%s|>", strSFXVolume ) );
  g_ciUI.m_ciSettingsMenuSecondary.SetEntry( SettingsAudioMenu.SoundPack, format( " |%s|>", strSoundPack ) );
  g_ciUI.m_ciSettingsMenuSecondary.SetEntry( SettingsAudioMenu.Back, format( "<|%s|", strBack ) );

  g_ciUI.m_ciSettingsMenuSecondary.CalcHeight();
  g_ciUI.m_ciSettingsMenuSecondary.SetCurrentIndex( i_iIndex );

  g_ciUI.m_ciSettingsMenuSecondary.SetBackgroundColor( g_ciUI.s_ciColorDarkBlue );  
}


function InitSettingsSoundPackMenu( i_iIndex )
{
  g_ciUI.m_ciSettingsMenuTertiary.Clear();

  local ciPos = g_ciUI.m_ciSettingsMenuTertiary.GetPos();
  ciPos.y = g_ciUI.m_ciSettingsMenuSecondary.GetSelectedPos().y;

  g_ciUI.m_ciSettingsMenuTertiary.SetPos( ciPos );

  foreach( i, aSoundPackArray in g_aSoundPacksArray )
  {
    local bSelected = ( aSoundPackArray[1] == g_ciCUO.m_strSoundPack );
    local strEntry = format( "%s|%s|", ( bSelected ? "+" : " " ), aSoundPackArray[0] );
    g_ciUI.m_ciSettingsMenuTertiary.SetEntry( i, strEntry );
  }

  local strBack = ::rumGetString( token_back_client_StringID );

  g_ciUI.m_ciSettingsMenuTertiary.SetEntry( g_aSoundPacksArray.len(), format( "<|%s|", strBack ) );

  g_ciUI.m_ciSettingsMenuTertiary.CalcHeight();
  g_ciUI.m_ciSettingsMenuTertiary.SetCurrentIndex( i_iIndex );
}


function InitSFXVolumerSlider()
{
  local ciPos = g_ciUI.m_ciSettingsMenuTertiary.GetPos();
  ciPos.y = g_ciUI.m_ciSettingsMenuSecondary.GetSelectedPos().y;

  g_ciUI.m_ciSettingsSlider.SetPos( ciPos );
  g_ciUI.m_ciSettingsSlider.SetValueRange( 1, 100 );
  g_ciUI.m_ciSettingsSlider.SetValue( g_ciCUO.m_uiSFXVolume );
}


function OnAudioMenuAccepted()
{
  g_ciUI.m_ciSettingsMenuTertiary.SetActive( false );
  g_ciUI.m_ciSettingsMenuTertiary.ClearHandlers();
  g_ciUI.m_ciSettingsSlider.SetActive( false );
  g_ciUI.m_ciSettingsSlider.ClearHandlers;

  local iSelectedKey = g_ciUI.m_ciSettingsMenuSecondary.GetSelectedKey();

  if( SettingsAudioMenu.MusicEnabled == iSelectedKey )
  {
    g_ciCUO.m_bMusicEnabled = !g_ciCUO.m_bMusicEnabled;
    if( !g_ciCUO.m_bMusicEnabled )
    {
      StopMusic();
    }
    else
    {
      // TODO - game mode determines what music files are played and when...
      PlayMusic( U4_Towns_SoundID );
    }

    // Save this setting
    g_ciCUO.m_ciGameConfigTable["cuo:music_enabled"] <- g_ciCUO.m_bMusicEnabled;
    ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );

    InitSettingsAudioMenu( g_ciUI.m_ciSettingsMenuSecondary.GetSelectedIndex() );
  }
  else if( SettingsAudioMenu.SFXEnabled == iSelectedKey )
  {
    g_ciCUO.m_bSFXEnabled = !g_ciCUO.m_bSFXEnabled;

    // Save this setting
    g_ciCUO.m_ciGameConfigTable["cuo:sfx_enabled"] <- g_ciCUO.m_bSFXEnabled;
    ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );

    InitSettingsAudioMenu( g_ciUI.m_ciSettingsMenuSecondary.GetSelectedIndex() );
  }
  else if( SettingsAudioMenu.MasterVolume == iSelectedKey )
  {
    OpenMasterVolumeSlider();
  }
  else if( SettingsAudioMenu.MusicVolume == iSelectedKey )
  {
    OpenMusicVolumeSlider();
  }
  else if( SettingsAudioMenu.SFXVolume == iSelectedKey )
  {
    OpenSFXVolumeSlider();
  }
  else if( SettingsAudioMenu.SoundPack == iSelectedKey )
  {
    OpenSettingsMusicPackMenu();
  }
  else if( SettingsAudioMenu.Back == iSelectedKey )
  {
    CloseMenu( g_ciUI.m_ciSettingsMenuSecondary );
  }
}


function OnAudioMenuCanceled()
{
  CloseMenu( g_ciUI.m_ciSettingsMenuSecondary );
}


function OnAudioMenuKeyPressed( i_ciKeyInput )
{
  local eKey = i_ciKeyInput.GetKey();

  if( rumKeypress.KeyRight() == eKey )
  {
    local iSelectedKey = g_ciUI.m_ciSettingsMenuSecondary.GetSelectedKey();

    if( SettingsAudioMenu.MasterVolume == iSelectedKey )
    {
      OpenMasterVolumeSlider();
    }
    else if( SettingsAudioMenu.MusicVolume == iSelectedKey )
    {
      OpenMusicVolumeSlider();
    }
    else if( SettingsAudioMenu.SFXVolume == iSelectedKey )
    {
      OpenSFXVolumeSlider();
    }
    else if( SettingsAudioMenu.SoundPack == iSelectedKey )
    {
      OpenSettingsMusicPackMenu();
    }
  }
  else if( rumKeypress.KeyLeft() == eKey )
  {
    CloseMenu( g_ciUI.m_ciSettingsMenuSecondary );
  }
}


function OnMasterVolumeSliderChanged( i_iValue )
{
  g_ciCUO.m_uiMasterVolume = i_iValue;

  if( g_ciCUO.m_ciPlayingMusic != null && g_ciCUO.m_ciPlayingMusic.IsPlaying() )
  {
    g_ciCUO.m_ciPlayingMusic.SetVolume( GetMusicVolume() );
  }

  // TODO - Hold off saving until the player "accepts" the change?
  // Update game.ini
  g_ciCUO.m_ciGameConfigTable["cuo:master_volume"] <- format( "%d", g_ciCUO.m_uiMasterVolume );
  ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );
}


function OnMusicVolumeSliderChanged( i_iValue )
{
  g_ciCUO.m_uiMusicVolume = i_iValue;

  if( g_ciCUO.m_ciPlayingMusic != null && g_ciCUO.m_ciPlayingMusic.IsPlaying() )
  {
    g_ciCUO.m_ciPlayingMusic.SetVolume( GetMusicVolume() );
  }

  // Update game.ini
  g_ciCUO.m_ciGameConfigTable["cuo:music_volume"] <- format( "%d", g_ciCUO.m_uiMusicVolume );
  ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );
}


function OnSFXVolumeSliderChanged( i_iValue )
{
  g_ciCUO.m_uiSFXVolume = i_iValue;

  // Update game.ini
  g_ciCUO.m_ciGameConfigTable["cuo:sfx_volume"] <- format( "%d", g_ciCUO.m_uiSFXVolume );
  ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );
}


function OnSoundPackMenuAccepted()
{
  local iSelectedKey = g_ciUI.m_ciSettingsMenuTertiary.GetSelectedKey();

  if( iSelectedKey >= g_aSoundPacksArray.len() )
  {
    CloseMenu( g_ciUI.m_ciSettingsMenuTertiary );
  }
  else
  {
    if( g_aSoundPacksArray[iSelectedKey][1] != g_ciCUO.m_strSoundPack )
    {
      ::rumLoadSoundArchive( g_aSoundPacksArray[iSelectedKey][1] );
      g_ciCUO.m_strSoundPack = g_aSoundPacksArray[iSelectedKey][1];

      // Update game.ini
      g_ciCUO.m_ciGameConfigTable["cuo:sound_pack"] <- g_ciCUO.m_strSoundPack;
      ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );

      // Update the menu
      InitSettingsSoundPackMenu( g_ciUI.m_ciSettingsMenuTertiary.GetSelectedIndex() );
    }
  }
}


function OnSoundPackMenuCanceled()
{
  CloseMenu( g_ciUI.m_ciSettingsMenuTertiary );
}


function OnSoundPackMenuKeyPressed( i_ciKeyInput )
{
  local eKey = i_ciKeyInput.GetKey();

  if( rumKeypress.KeyLeft() == eKey )
  {
    CloseMenu( g_ciUI.m_ciSettingsMenuTertiary );
  }
}


function OpenMasterVolumeSlider()
{
  g_ciUI.m_ciSettingsMenuSecondary.SetBackgroundColor( g_ciUI.s_ciColorLightBlue );

  InitMasterVolumerSlider();

  g_ciUI.m_ciSettingsSlider.SetActive( true );
  g_ciUI.m_ciSettingsSlider.m_funcAccept = CloseSettingsSlider;
  g_ciUI.m_ciSettingsSlider.m_funcCancel = CloseSettingsSlider;
  g_ciUI.m_ciSettingsSlider.m_funcValueChanged = OnMasterVolumeSliderChanged;
  g_ciUI.m_ciSettingsSlider.Focus();
}


function OpenMusicVolumeSlider()
{
  g_ciUI.m_ciSettingsMenuSecondary.SetBackgroundColor( g_ciUI.s_ciColorLightBlue );

  InitMusicVolumerSlider();

  g_ciUI.m_ciSettingsSlider.SetActive( true );
  g_ciUI.m_ciSettingsSlider.m_funcAccept = CloseSettingsSlider;
  g_ciUI.m_ciSettingsSlider.m_funcCancel = CloseSettingsSlider;
  g_ciUI.m_ciSettingsSlider.m_funcValueChanged = OnMusicVolumeSliderChanged;
  g_ciUI.m_ciSettingsSlider.Focus();
}


function OpenSettingsAudioMenu()
{
  g_ciUI.m_ciSettingsMenu.SetBackgroundColor( g_ciUI.s_ciColorLightBlue );

  InitSettingsAudioMenu( 0 );

  g_ciUI.m_ciSettingsMenuSecondary.SetActive( true );
  g_ciUI.m_ciSettingsMenuSecondary.m_funcAccept = OnAudioMenuAccepted;
  g_ciUI.m_ciSettingsMenuSecondary.m_funcCancel = OnAudioMenuCanceled;
  g_ciUI.m_ciSettingsMenuSecondary.m_funcKeyPress = OnAudioMenuKeyPressed;
  g_ciUI.m_ciSettingsMenuSecondary.Focus();
}


function OpenSettingsMusicPackMenu()
{
  g_ciUI.m_ciSettingsMenuSecondary.SetBackgroundColor( g_ciUI.s_ciColorLightBlue );

  InitSettingsSoundPackMenu( 0 );

  g_ciUI.m_ciSettingsMenuTertiary.SetActive( true );
  g_ciUI.m_ciSettingsMenuTertiary.m_funcAccept = OnSoundPackMenuAccepted;
  g_ciUI.m_ciSettingsMenuTertiary.m_funcCancel = OnSoundPackMenuCanceled;
  g_ciUI.m_ciSettingsMenuTertiary.m_funcKeyPress = OnSoundPackMenuKeyPressed;
  g_ciUI.m_ciSettingsMenuTertiary.Focus();
}


function OpenSFXVolumeSlider()
{
  g_ciUI.m_ciSettingsMenuSecondary.SetBackgroundColor( g_ciUI.s_ciColorLightBlue );

  InitSFXVolumerSlider();

  g_ciUI.m_ciSettingsSlider.SetActive( true );
  g_ciUI.m_ciSettingsSlider.m_funcAccept = CloseSettingsSlider;
  g_ciUI.m_ciSettingsSlider.m_funcCancel = CloseSettingsSlider;
  g_ciUI.m_ciSettingsSlider.m_funcValueChanged = OnSFXVolumeSliderChanged;
  g_ciUI.m_ciSettingsSlider.Focus();
}
