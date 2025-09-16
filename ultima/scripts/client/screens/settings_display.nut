enum SettingsDisplayMenu
{
  DisplayMode,
  Resolution,
  Pack,
  Back
}

// Common 4:3 aspect ratios
g_aWindowedResolutionsArray <-
[
  [4096, 3072],
  [3200, 2400],
  [2048, 1536],
  [1920, 1440],
  [1856, 1392],
  [1600, 1200],
  [1400, 1050],
  [1280, 960],
  [1152, 864],
  [1024, 768],
  [800,  600],
  [640,  480]
]

g_aGraphicsPacksArray <-
[
  [ "Apple ][",       "tiles_apple2.pkg" ],
  [ "ASCII",          "tiles_ascii.pkg" ],
  [ "C64",            "tiles_c64.pkg" ],
  [ "C64 Remastered", "tiles_c64_rem.pkg" ],
  [ "FM Towns",       "tiles_fm_towns.pkg" ],
  [ "NES",            "tiles_nes.pkg" ],
  [ "PC EGA",         "tiles_ega.pkg" ],
  [ "PC Steele v1",   "tiles_steele1.pkg" ],
  [ "PC Steele v2",   "tiles_steele2.pkg" ],
  [ "PC Steele v3",   "tiles_steele3.pkg" ],
  [ "SMS",            "tiles_sms.pkg" ],
  [ "XU4",            "tiles_xu4.pkg" ]
]


function InitSettingsDisplayMenu( i_iIndex )
{
  g_ciUI.m_ciSettingsMenuSecondary.Clear();

  local ciPos = g_ciUI.m_ciSettingsMenuSecondary.GetPos();
  ciPos.y = g_ciUI.m_ciSettingsMenu.GetSelectedPos().y;

  g_ciUI.m_ciSettingsMenuSecondary.SetPos( ciPos );

  local strBack = ::rumGetString( token_back_client_StringID );
  local strFullscreen = ::rumGetString( token_fullscreen_client_StringID );
  local strGraphicsPack = ::rumGetString( token_graphics_pack_client_StringID );
  local strOn = ::rumGetString( token_on_client_StringID );
  local strOff = ::rumGetString( token_off_client_StringID );
  local strResolution = ::rumGetString( token_resolution_client_StringID );

  g_ciUI.m_ciSettingsMenuSecondary.SetEntry( SettingsDisplayMenu.DisplayMode,
                                             format( " |%s: %s|",
                                               strFullscreen, ::rumIsFullscreen() ? strOn : strOff ) );
  g_ciUI.m_ciSettingsMenuSecondary.SetEntry( SettingsDisplayMenu.Resolution, format( " |%s|>", strResolution ) );
  g_ciUI.m_ciSettingsMenuSecondary.SetEntry( SettingsDisplayMenu.Pack, format( " |%s|>", strGraphicsPack ) );
  g_ciUI.m_ciSettingsMenuSecondary.SetEntry( SettingsDisplayMenu.Back, format( "<|%s|", strBack ) );

  g_ciUI.m_ciSettingsMenuSecondary.CalcHeight();
  g_ciUI.m_ciSettingsMenuSecondary.SetCurrentIndex( i_iIndex );

  g_ciUI.m_ciSettingsMenuSecondary.SetBackgroundColor( g_ciUI.s_ciColorDarkBlue );
}


function InitSettingsGraphicsPackMenu( i_iIndex )
{
  g_ciUI.m_ciSettingsMenuTertiary.Clear();

  local ciPos = g_ciUI.m_ciSettingsMenuTertiary.GetPos();
  ciPos.y = g_ciUI.m_ciSettingsMenuSecondary.GetSelectedPos().y;

  g_ciUI.m_ciSettingsMenuTertiary.SetPos( ciPos );

  foreach( i, aGraphicsPackArray in g_aGraphicsPacksArray )
  {
    local bSelected = ( aGraphicsPackArray[1] == g_ciCUO.m_strGraphicsPack );
    local strEntry =  format( "%s|%s|", ( bSelected ? "+" : " " ), aGraphicsPackArray[0] );
    g_ciUI.m_ciSettingsMenuTertiary.SetEntry( i, strEntry );
  }

  local strBack = ::rumGetString( token_back_client_StringID );

  g_ciUI.m_ciSettingsMenuTertiary.SetEntry( g_aGraphicsPacksArray.len(), format( "<|%s|", strBack ) );

  g_ciUI.m_ciSettingsMenuTertiary.CalcHeight();
  g_ciUI.m_ciSettingsMenuTertiary.SetCurrentIndex( i_iIndex );
}


function InitSettingsResolutionMenu( i_iIndex )
{
  g_ciUI.m_ciSettingsMenuTertiary.Clear();

  local ciPos = g_ciUI.m_ciSettingsMenuTertiary.GetPos();
  ciPos.y = g_ciUI.m_ciSettingsMenuSecondary.GetSelectedPos().y;

  g_ciUI.m_ciSettingsMenuTertiary.SetPos( ciPos );

  local strBack = ::rumGetString( token_back_client_StringID );

  if( ::rumIsFullscreen() )
  {
    local iCurrentHeight = ::rumGetFullScreenHeight();
    local iCurrentWidth = ::rumGetFullScreenWidth();

    // Get the available resolutions and sort from largest to smallest
    local iResolutionsArray = ::rumGetResolutions();
    iResolutionsArray.reverse();

    local iResolutionKeyTable = {};

    for( local i = 0; i < iResolutionsArray.len(); ++i )
    {
      // Create a key based on the product of the width & height so that we can avoid listing redundant resolutions
      local iKey = iResolutionsArray[i][0] * iResolutionsArray[i][1];
      if( !( iKey in iResolutionKeyTable ) )
      {
        local bMatched = ( iCurrentWidth == iResolutionsArray[i][0] ) && ( iCurrentHeight == iResolutionsArray[i][1] );
        local strEntry = format( "%s|%d x %d|",
                               ( bMatched ? "+" : " " ),
                               iResolutionsArray[i][0],
                               iResolutionsArray[i][1] );
        g_ciUI.m_ciSettingsMenuTertiary.SetEntry( i, strEntry );
        iResolutionKeyTable[iKey] <- i;
      }
    }

    g_ciUI.m_ciSettingsMenuTertiary.SetEntry( iResolutionsArray.len(), format( "<|%s|", strBack ) );
  }
  else
  {
    local iCurrentHeight = ::rumGetWindowedScreenHeight();
    local iCurrentWidth = ::rumGetWindowedScreenWidth();

    local ciRes = ::rumGetMaxScreenResolution();

    foreach( i, iResolutionArray in g_aWindowedResolutionsArray )
    {
      if( ciRes.x >= iResolutionArray[0] && ciRes.y >= iResolutionArray[1] )
      {
        // Add each resolution entry
        local bMatched = ( iCurrentWidth == iResolutionArray[0] ) && ( iCurrentHeight == iResolutionArray[1] );
        local strEntry =  format( "%s|%d x %d|", ( bMatched ? "+" : " " ), iResolutionArray[0], iResolutionArray[1] );
        g_ciUI.m_ciSettingsMenuTertiary.SetEntry( i, strEntry );
      }
    }

    g_ciUI.m_ciSettingsMenuTertiary.SetEntry( g_aWindowedResolutionsArray.len(), format( "<|%s|", strBack ) );
  }

  g_ciUI.m_ciSettingsMenuTertiary.CalcHeight();
  g_ciUI.m_ciSettingsMenuTertiary.SetCurrentIndex( i_iIndex );
}


function OnDisplayMenuAccepted()
{
  g_ciUI.m_ciSettingsMenuTertiary.SetActive( false );
  g_ciUI.m_ciSettingsMenuTertiary.ClearHandlers();
  g_ciUI.m_ciSettingsSlider.SetActive( false );
  g_ciUI.m_ciSettingsSlider.ClearHandlers;

  local iSelectedKey = g_ciUI.m_ciSettingsMenuSecondary.GetSelectedKey();

  if( SettingsDisplayMenu.DisplayMode == iSelectedKey )
  {
      if( ::rumIsFullscreen() )
      {
        // Switch to windowed
        ::rumSetWindowedMode( ::rumGetWindowedScreenWidth(), ::rumGetWindowedScreenHeight() );
      }
      else
      {
        // Switch to full screen
        ::rumSetFullscreenMode( ::rumGetFullScreenWidth(), ::rumGetFullScreenHeight() );
      }

      InitSettingsDisplayMenu( g_ciUI.m_ciSettingsMenuSecondary.GetSelectedIndex() );
  }
  else if( SettingsDisplayMenu.Resolution == iSelectedKey )
  {
    OpenSettingsResolutionMenu();
  }
  else if( SettingsDisplayMenu.Pack == iSelectedKey )
  {
    OpenSettingsGraphicsPackMenu();
  }
  else if( SettingsDisplayMenu.Back )
  {
    CloseMenu( g_ciUI.m_ciSettingsMenuSecondary );
  }
}


function OnDisplayMenuCanceled()
{
  CloseMenu( g_ciUI.m_ciSettingsMenuSecondary );
}


function OnDisplayMenuKeyPressed( i_ciKeyInput )
{
  local eKey = i_ciKeyInput.GetKey();

  if( rumKeypress.KeyRight() == eKey )
  {
    local iSelectedKey = g_ciUI.m_ciSettingsMenuSecondary.GetSelectedKey();

    if( SettingsDisplayMenu.Resolution == iSelectedKey )
    {
      OpenSettingsResolutionMenu();
    }
    else if( SettingsDisplayMenu.Pack == iSelectedKey )
    {
      OpenSettingsGraphicsPackMenu();
    }
  }
  else if( rumKeypress.KeyLeft() == eKey )
  {
    CloseMenu( g_ciUI.m_ciSettingsMenuSecondary );
  }
}


function OnGraphicsPackMenuAccepted()
{
  local iSelectedKey = g_ciUI.m_ciSettingsMenuTertiary.GetSelectedKey();

  if( iSelectedKey >= g_aGraphicsPacksArray.len() )
  {
    CloseMenu( g_ciUI.m_ciSettingsMenuTertiary );
  }
  else if( g_aGraphicsPacksArray[iSelectedKey][1] != g_ciCUO.m_strGraphicsPack )
  {
    ::rumLoadGraphicArchive( g_aGraphicsPacksArray[iSelectedKey][1] );
    g_ciCUO.m_strGraphicsPack = g_aGraphicsPacksArray[iSelectedKey][1];

    // Update game.ini
    g_ciCUO.m_ciGameConfigTable["cuo:graphics_pack"] <- g_ciCUO.m_strGraphicsPack;
    ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );

    // Update the menu
    InitSettingsGraphicsPackMenu( g_ciUI.m_ciSettingsMenuTertiary.GetSelectedIndex() );
  }
}


function OnGraphicsPackMenuCanceled()
{
  CloseMenu( g_ciUI.m_ciSettingsMenuTertiary );
}


function OnGraphicsPackMenuKeyPressed( i_ciKeyInput )
{
  local eKey = i_ciKeyInput.GetKey();

  if( rumKeypress.KeyLeft() == eKey )
  {
    CloseMenu( g_ciUI.m_ciSettingsMenuTertiary );
  }
}


function OnResolutionMenuAccepted()
{
  local iSelectedKey = g_ciUI.m_ciSettingsMenuTertiary.GetSelectedKey();

  if( ::rumIsFullscreen() )
  {
    local iResolutionsArray = ::rumGetResolutions();
    if( iSelectedKey >= iResolutionsArray.len() )
    {
      CloseMenu( g_ciUI.m_ciSettingsMenuTertiary );
    }
    else
    {
      local ciRes = ::rumGetMaxScreenResolution();
      iResolutionsArray.reverse();

      ::rumSetFullscreenMode( iResolutionsArray[iSelectedKey][0], iResolutionsArray[iSelectedKey][1] );
    }
  }
  else
  {
    if( iSelectedKey >= g_aWindowedResolutionsArray.len() )
    {
      CloseMenu( g_ciUI.m_ciSettingsMenuTertiary );
    }
    else
    {
      // Note that we don't save the change or reinitialize the menu here as those are handled by other callbacks
      ::rumSetWindowedMode( g_aWindowedResolutionsArray[iSelectedKey][0],
                            g_aWindowedResolutionsArray[iSelectedKey][1] );
    }
  }
}


function OnResolutionMenuCanceled()
{
  CloseMenu( g_ciUI.m_ciSettingsMenuTertiary );
}


function OnResolutionMenuKeyPressed( i_ciKeyInput )
{
  local eKey = i_ciKeyInput.GetKey();

  if( rumKeypress.KeyLeft() == eKey )
  {
    CloseMenu( g_ciUI.m_ciSettingsMenuTertiary );
  }
}


function OpenSettingsDisplayMenu()
{
  g_ciUI.m_ciSettingsMenu.SetBackgroundColor( g_ciUI.s_ciColorLightBlue );

  InitSettingsDisplayMenu( 0 );

  g_ciUI.m_ciSettingsMenuSecondary.SetActive( true );
  g_ciUI.m_ciSettingsMenuSecondary.m_funcAccept = OnDisplayMenuAccepted;
  g_ciUI.m_ciSettingsMenuSecondary.m_funcCancel = OnDisplayMenuCanceled;
  g_ciUI.m_ciSettingsMenuSecondary.m_funcKeyPress = OnDisplayMenuKeyPressed;
  g_ciUI.m_ciSettingsMenuSecondary.Focus();
}


function OpenSettingsGraphicsPackMenu()
{
  g_ciUI.m_ciSettingsMenuSecondary.SetBackgroundColor( g_ciUI.s_ciColorLightBlue );

  InitSettingsGraphicsPackMenu( 0 );

  g_ciUI.m_ciSettingsMenuTertiary.SetActive( true );
  g_ciUI.m_ciSettingsMenuTertiary.m_funcAccept = OnGraphicsPackMenuAccepted;
  g_ciUI.m_ciSettingsMenuTertiary.m_funcCancel = OnGraphicsPackMenuCanceled;
  g_ciUI.m_ciSettingsMenuTertiary.m_funcKeyPress = OnGraphicsPackMenuKeyPressed;
  g_ciUI.m_ciSettingsMenuTertiary.Focus();
}


function OpenSettingsResolutionMenu()
{
  g_ciUI.m_ciSettingsMenuSecondary.SetBackgroundColor( g_ciUI.s_ciColorLightBlue );

  InitSettingsResolutionMenu( 0 );

  g_ciUI.m_ciSettingsMenuTertiary.SetActive( true );
  g_ciUI.m_ciSettingsMenuTertiary.m_funcAccept = OnResolutionMenuAccepted;
  g_ciUI.m_ciSettingsMenuTertiary.m_funcCancel = OnResolutionMenuCanceled;
  g_ciUI.m_ciSettingsMenuTertiary.m_funcKeyPress = OnResolutionMenuKeyPressed;
  g_ciUI.m_ciSettingsMenuTertiary.Focus();
}
