class GameSingleton
{
  static s_iLOSRadius = 5;
  static s_iLOSRadiusPeering = 10;
  static s_iMaxCharactersPerAccount = 8;

  // Audio
  m_bMusicEnabled = true;
  m_bSFXEnabled = true;

  m_ciPlayingMusic = null;

  m_strGraphicsPack = "tiles_steele2.pkg";
  m_strSoundPack = "sounds_pc.pkg";

  m_uiMasterVolume = 100;
  m_uiMusicVolume = 100;
  m_uiSFXVolume = 100;

  // Configuration
  m_ciGameConfigTable = null;

  // Game states
  m_bAccountCreate = false;
  m_bAloft = false;
  m_bCharacterDelete = false;
  m_bPeering = false;
  m_bReceivingData = false;
  m_bShuttingDown = false;
  m_bWaitingForResponse = false;

  m_eCurrentGameMode = GameMode.Title;
  m_eTimeGatePeriod = U2_TimegatePeriods.Pangea;
  m_eTitleStage = TitleStages.Splash;
  m_eWindDirection = Direction.West;
  m_eVersion = GameType.Ultima4;

  m_ciAttackEffectsTable = null;
  m_ciCurrentMap = null;

  // Moon phases
  m_eFeluccaPhase = MoonPhase.New;
  m_eTrammelPhase = MoonPhase.New;

  // Party
  m_uiPartyLeaderID = rumInvalidGameID;
  m_uiPartyIDTable = null;

  // Talking
  m_ciKingRef = null;
  m_ciMerchantRef = null;
  m_strDialogueTable = null;
  m_uiCurrentTalkID = rumInvalidGameID;

  // Targeting
  m_ciTargetPos = null;
  m_iLockedTargetIndex = -1;
  m_uiLockedTargetID = rumInvalidGameID;
  m_uiTargetRange = 1;
  m_uiVisibleTargetIDsTable = null;

  // Timers
  m_fMainTimer = 0.0;
  m_fTileAnimationTimer = 0.0;
  m_fTitleTimer = 0.0;

  m_funcLastComparator = null;
  m_strCharacterDeleteName = "";

  m_strCurrentDirectory = "";

  m_strPatchFileTable = null;


  constructor()
  {
    m_ciAttackEffectsTable = {};
    m_ciGameConfigTable = {};

    m_strDialogueTable = {};
    m_strPatchFileTable = {};

    m_uiVisibleTargetIDsTable = {};

    m_strCurrentDirectory = ::rumGetCurrentDirectory();
  }


  function ReturnToMainMenu()
  {
    m_eCurrentGameMode = GameMode.Title;
    InitTitleStage( TitleStages.MainMenu );
  }


  function Shutdown()
  {
    m_bShuttingDown = true;

    if( m_ciPlayingMusic != null )
    {
      m_ciPlayingMusic.Stop();
      m_ciPlayingMusic = null;
    }
  }
}


class InterfaceSingleton
{
  static s_ciColorBlackAlpha = rumColor( 0, 0, 0, 127 );
  static s_ciColorDarkBlue = rumColor( 0, 0, 64 );
  static s_ciColorDarkGreen = rumColor( 0, 64, 0 );
  static s_ciColorDarkRed = rumColor( 64, 0, 0 );
  static s_ciColorLightBlue = rumColor( 64, 64, 96 );
  static s_ciColorLightGreen = rumColor( 0, 96, 0 );

  static s_ciColorBlueScrollbar = rumColor( 64, 64, 255 );
  static s_ciColorBlueScrollbarHandle = rumColor( 127, 127, 255 );

  static s_ciColorIce = rumColor( 64, 128, 192, 32 );
  static s_ciColorPoison = rumColor( 64, 192, 64, 92 );

  static s_ciColorMissingHitpoints = rumColor( 255, 127, 127 );
  static s_ciColorMissingMana = rumColor( 127, 127, 255 );

  static s_iBorderPixelWidth = ::rumGetTilePixelWidth() / 2;
  static s_iCommandHistorySize = 5;
  static s_iDefaultLabelHeight = ::rumGetTilePixelWidth() / 2;
  static s_iMenuWidth = 128;
  static s_iTextBoxWidth = 256;
  static s_iTextPixelPadding = 4;
  static s_iTileHalfPixelWidth = ::rumGetTilePixelWidth() / 2;
  static s_iTilePixelWidth = ::rumGetTilePixelWidth();
  static s_iTileQuarterPixelWidth = ::rumGetTilePixelWidth() / 4;
  static s_iVisibleMapTilesStandard = 11;
  static s_iVisibleMapTilesPeering = 21;

  static s_ciCenterTilePos = rumPoint( 5, 5 );
  static s_ciCenterTilePeeringPos = rumPoint( 10, 10 ); 

  static s_iDaemonFrameIndicesArray =
  [
    1, 0, 1, 2, 3, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 5, 6, 5, 6, 4, 7, 8, 9, 10, 9, 8, 7, 8, 9, 10, 11, 12, 11, 12, 13,
    11, 12, 13, 1, 13, 1, 14, 1, 15, 1, 14, 1, 15, 10, 9, 8, 16, 17, 16, 17, 16, 17, 9, 8, 7, 4, 3, 2
  ]

  static s_iDragonFrameIndicesArray =
  [
    1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 1, 2, 3, 4, 1, 2, 5, 6, 7, 8, 5, 6, 7, 8, 5, 6, 7, 8, 5, 6,
    7, 8, 5, 6, 7, 8, 5, 6, 7, 8, 9, 10, 9, 10, 9, 10, 11, 11, 11, 11, 12, 12, 13, 13, 12, 13, 12, 13, 12, 11, 11, 11,
    0, 0, 1, 2, 3, 4, 1, 2, 5, 6, 7, 8, 5, 6, 7, 8, 9, 10, 11, 11, 11, 0, 0, 14, 14, 14, 15, 16, 16, 16, 17, 17, 17,
    16, 16, 16, 17, 17, 17, 16, 16, 16, 15, 14, 14, 0, 0, 11, 11, 11, 1, 0, 1, 2, 3, 4, 3, 2
  ]

  // Controls
  m_ciChatTextView = null;
  m_ciExplorerDriveListView = null;
  m_ciExplorerFileListView = null;
  m_ciFoodLabel = null;
  m_ciGameInputTextBox = null;
  m_ciGameListView = null;
  m_ciGameRegion = null;
  m_ciGameTextView = null;
  m_ciGoldLabel = null;
  m_ciInfoLabel = null;
  m_ciMapNameLabel = null;
  m_ciScreenRegion = null;
  m_ciSplashLabel = null;
  m_ciStatLabel = null;
  m_ciStatListView = null;
  m_ciTextBoxArray = [ null, null, null, null ];
  m_ciTitleLabel = null;
  m_ciTitleListMenu = null;
  m_ciTitleTextView = null;
  m_ciYesNoListView = null;

  m_ciAnkhPos = null;
  m_ciMapRect = null;
  m_ciMoonDisplayPos = null;
  m_ciPeerMapRect = null;
  m_ciPlayerCenterPos = null;
  m_ciPlayerOffsetPos = null;
  m_ciSettingsMenu = null;
  m_ciSettingsMenuSecondary = null;
  m_ciSettingsMenuTertiary = null;
  m_ciSettingsSlider = null;

  // State
  m_bShowCombat = true;
  m_bShowMoves = true;
  m_bShowNames = true;
  m_bShowVitals = true;

  m_iDaemonAnimFrame = 0;
  m_iDragonAnimFrame = 0;

  m_iTitleAnimationOffset = -64;

  m_eCurrentStatPage = U4_StatPage.Main;
  m_eMapLabelType = MapLabelType.MapName;
  m_ePreviousStatPage = U4_StatPage.Main;

  m_eOverlayGraphicID = rumInvalidAssetID;
  m_ciOverlayGraphicOffset = null;

  m_eOverrideGraphicID = rumInvalidAssetID;
  m_ciOverrideGraphicOffset = null;

  m_eVerificationVersion = GameType.Invalid;
  m_bNeedsInstallVerification = false;
  m_iNumVerificationAttempts = 0;

  m_iCommandHistoryDisplayIndex = -1;
  m_iCommandHistoryInsertIndex = 0;
  m_strCommandHistoryArray = null;

  m_bInputBlocked = false;
  m_fBlockTimer = 0.0;


  constructor()
  {
    m_strCommandHistoryArray = array( s_iCommandHistorySize, null );
  }


  function AppendCommandHistory( i_strCommand )
  {
    m_strCommandHistoryArray[m_iCommandHistoryInsertIndex++] = i_strCommand;
    if( s_iCommandHistorySize == m_iCommandHistoryInsertIndex )
    {
      m_iCommandHistoryInsertIndex = 0;
    }

    m_iCommandHistoryDisplayIndex = -1;
  }


  function CalculateOffsets()
  {
    // 352
    local iMapPixelWidth = s_iVisibleMapTilesStandard * s_iTilePixelWidth;

    // 336
    local iPeerMapPixelWidth = s_iVisibleMapTilesPeering * s_iBorderPixelWidth;

    // 24
    local iPeerMapOffset = ( iMapPixelWidth - iPeerMapPixelWidth ) / 2 + s_iBorderPixelWidth;

    // 192
    local iMapCenter = iMapPixelWidth / 2 + s_iBorderPixelWidth;

    // 16, 16, 368, 368
    local ciMapPos = rumPoint( s_iBorderPixelWidth, s_iBorderPixelWidth );
    m_ciMapRect = rumRect( ciMapPos, iMapPixelWidth, iMapPixelWidth );

    // 24, 24, 360, 360
    m_ciPeerMapRect = rumRect( rumPoint( iPeerMapOffset, iPeerMapOffset ), iPeerMapPixelWidth, iPeerMapPixelWidth );

    // 192, 192
    m_ciPlayerCenterPos = rumPoint( iMapCenter, iMapCenter );

    // 160, 160
    local iPlayerOffset = iMapPixelWidth / 2 - s_iBorderPixelWidth;
    m_ciPlayerOffsetPos = rumPoint( iPlayerOffset, iPlayerOffset );

    // 176, 0
    m_ciMoonDisplayPos = rumPoint( iMapCenter - s_iBorderPixelWidth, 0 );

    // 104, 368
    local ciMapNameLabelPos = rumPoint( iMapCenter - ( m_ciMapNameLabel.GetWidth() / 2 ),
                                        s_iBorderPixelWidth + iMapPixelWidth );

    // 16, 400
    local ciChatListPos = rumPoint( s_iBorderPixelWidth, ciMapNameLabelPos.y + ( s_iBorderPixelWidth * 2 ) );

    // 384, 16
    local ciStatListPos = rumPoint( iMapPixelWidth + ( s_iBorderPixelWidth * 2 ), s_iBorderPixelWidth );

    local iStatListCenter = ( ciStatListPos.x + ( m_ciStatListView.GetWidth() / 2 ) );

    // 424, 0
    local ciStatLabelPos = rumPoint( iStatListCenter - ( m_ciStatLabel.GetWidth() / 2 ), 0 );

    // 384, 160
    local ciFoodLabelPos = rumPoint( ciStatListPos.x,
                                     ciStatListPos.y + m_ciStatListView.GetHeight() + s_iBorderPixelWidth );

    // 496, 160
    local ciGoldLabelPos = rumPoint( ciStatListPos.x + m_ciStatListView.GetWidth() - m_ciGoldLabel.GetWidth(),
                                     ciFoodLabelPos.y );

    // 496, 160
    m_ciAnkhPos = rumPoint( iStatListCenter - ( s_iBorderPixelWidth / 2 ), ciFoodLabelPos.y );

    // 384, 192
    local ciTextViewPos = rumPoint( ciStatListPos.x,
                                    ciFoodLabelPos.y + m_ciFoodLabel.GetHeight() + s_iBorderPixelWidth );

    // 384, 368
    local ciGameInputPos = rumPoint( ciStatListPos.x, ciMapNameLabelPos.y );

    // 176, 400
    local ciFileListPos = rumPoint( ciChatListPos.x + m_ciExplorerDriveListView.GetWidth(), ciChatListPos.y );

    // Set control positions
    m_ciChatTextView.SetPos( ciChatListPos );
    m_ciExplorerDriveListView.SetPos( ciChatListPos );
    m_ciExplorerFileListView.SetPos( ciFileListPos );
    m_ciFoodLabel.SetPos( ciFoodLabelPos );
    m_ciGoldLabel.SetPos( ciGoldLabelPos );
    m_ciGameInputTextBox.SetPos( ciGameInputPos );
    m_ciGameListView.SetPos( ciGameInputPos );
    m_ciGameRegion.SetPos( ciMapPos );
    m_ciGameTextView.SetPos( ciTextViewPos );
    m_ciMapNameLabel.SetPos( ciMapNameLabelPos );
    m_ciStatLabel.SetPos( ciStatLabelPos );
    m_ciStatListView.SetPos( ciStatListPos );
    m_ciYesNoListView.SetPos( ciGameInputPos );
  }


  function InitControls()
  {
    local iScreenHeight = ::rumGetScreenHeight();
    local iScreenWidth = ::rumGetScreenWidth();
    local ciGraphic = ::rumGetGraphic( U4_Title_GraphicID );
    local iTitleWidth = ciGraphic.GetFrameWidth();

    m_ciChatTextView = ::rumCreateControl( TextView );
    m_ciChatTextView.SetBackgroundColor( s_ciColorDarkBlue );
    m_ciChatTextView.SetWidth( s_iBorderPixelWidth * 38 );
    m_ciChatTextView.SetHeight( s_iBorderPixelWidth * 4 );
    m_ciChatTextView.SetBufferHeight( s_iBorderPixelWidth * 64 );
    m_ciChatTextView.SetScrollbarWidth( 2 );
    m_ciChatTextView.SetScrollbarColors( s_ciColorBlueScrollbarHandle, s_ciColorBlueScrollbar );
    m_ciChatTextView.SetHandlesInput( false );
    //m_ciChatTextView.SetCursor( Cursor_GraphicID );
    //m_ciChatTextView.ShowCursor(true);
    //m_ciChatTextView.SetNewestFirst(true);

    m_ciExplorerDriveListView = ::rumCreateControl( ListView );
    m_ciExplorerDriveListView.SetBackgroundColor( s_ciColorDarkBlue );
    m_ciExplorerDriveListView.SetWidth( 160 );
    m_ciExplorerDriveListView.SetHeight( 160 );
    m_ciExplorerDriveListView.ShowScrollbar( true );
    m_ciExplorerDriveListView.SetFormat( "0.0|0.45" );
    m_ciExplorerDriveListView.SetScrollbarWidth( 2 );
    m_ciExplorerDriveListView.SetScrollbarColors( s_ciColorBlueScrollbarHandle, s_ciColorBlueScrollbar );

    m_ciExplorerFileListView = ::rumCreateControl( ListView );
    m_ciExplorerFileListView.SetBackgroundColor( s_ciColorDarkBlue );
    m_ciExplorerFileListView.SetWidth( s_iTextBoxWidth );
    m_ciExplorerFileListView.SetHeight( 160 );
    m_ciExplorerFileListView.ShowScrollbar( true );
    m_ciExplorerFileListView.SetFormat( "0.0|0.05" );
    m_ciExplorerFileListView.SetScrollbarWidth( 2 );
    m_ciExplorerFileListView.SetScrollbarColors( s_ciColorBlueScrollbarHandle, s_ciColorBlueScrollbar );

    m_ciFoodLabel = ::rumCreateControl( Label );
    m_ciFoodLabel.SetBackgroundColor( s_ciColorDarkBlue );
    m_ciFoodLabel.SetWidth( s_iBorderPixelWidth * 7 );
    m_ciFoodLabel.SetHeight( s_iBorderPixelWidth );
    m_ciFoodLabel.SetHandlesInput( false );
    m_ciFoodLabel.AlignCenter();

    m_ciGameInputTextBox = ::rumCreateControl( GameInput );
    m_ciGameInputTextBox.SetBackgroundColor( s_ciColorDarkBlue );
    m_ciGameInputTextBox.SetWidth( s_iBorderPixelWidth * 16 );
    m_ciGameInputTextBox.SetHeight( s_iBorderPixelWidth );
    m_ciGameInputTextBox.SetPrompt( Prompt_GraphicID );
    m_ciGameInputTextBox.SetCursor( Cursor_GraphicID );
    m_ciGameInputTextBox.ShowPrompt( true );
    m_ciGameInputTextBox.ShowCursor( true );

    m_ciGameListView = ::rumCreateControl( ListView );
    m_ciGameListView.SetBackgroundColor( s_ciColorDarkGreen );
    m_ciGameListView.SetWidth( s_iTextBoxWidth );
    m_ciGameListView.SetHeight( 128 );
    m_ciGameListView.SetDelimiter( "|" );
    m_ciGameListView.SetFormat( "0.05|0.5" );
    m_ciGameListView.SetScrollbarWidth( 2 );
    m_ciGameListView.SetScrollbarColors( s_ciColorBlueScrollbarHandle, s_ciColorBlueScrollbar );
    m_ciGameListView.SetPrompt( Prompt_GraphicID );
    m_ciGameListView.m_funcIndexChanged = OnGameListViewIndexChanged;

    m_ciGameRegion = ::rumCreateControl( Region );
    m_ciGameRegion.SetWidth( s_iVisibleMapTilesStandard * s_iTilePixelWidth );
    m_ciGameRegion.SetHeight( s_iVisibleMapTilesStandard * s_iTilePixelWidth );

    m_ciGameTextView = ::rumCreateControl( TextView );
    m_ciGameTextView.SetBackgroundColor( s_ciColorDarkBlue );
    m_ciGameTextView.SetWidth( s_iBorderPixelWidth * 16 );
    m_ciGameTextView.SetHeight( s_iBorderPixelWidth * 11 );
    m_ciGameTextView.SetBufferHeight( m_ciGameTextView.GetHeight() );
    m_ciGameTextView.ShowScrollbar( false );

    m_ciGoldLabel = ::rumCreateControl( Label );
    m_ciGoldLabel.SetBackgroundColor( s_ciColorDarkBlue );
    m_ciGoldLabel.SetWidth( s_iBorderPixelWidth * 7 );
    m_ciGoldLabel.SetHeight( s_iBorderPixelWidth );
    m_ciGoldLabel.SetHandlesInput( false );
    m_ciGoldLabel.AlignCenter();

    m_ciInfoLabel = ::rumCreateControl( Label );
    m_ciInfoLabel.SetBackgroundColor( ::rumColorBlack );
    m_ciInfoLabel.SetWidth( iTitleWidth );
    m_ciInfoLabel.SetHeight( s_iDefaultLabelHeight );
    m_ciInfoLabel.SetHandlesInput( false );
    m_ciInfoLabel.AlignCenter();

    m_ciMapNameLabel = ::rumCreateControl( Label );
    m_ciMapNameLabel.SetBackgroundColor( s_ciColorDarkBlue );
    m_ciMapNameLabel.SetWidth( s_iBorderPixelWidth * 10 );
    m_ciMapNameLabel.SetHeight( s_iBorderPixelWidth );
    m_ciMapNameLabel.SetHandlesInput( false );
    m_ciMapNameLabel.AlignCenter();

    m_ciScreenRegion = ::rumCreateControl( Region );
    m_ciScreenRegion.SetWidth( iScreenWidth );
    m_ciScreenRegion.SetHeight( iScreenHeight );

    m_ciSplashLabel = ::rumCreateControl( Label );
    m_ciSplashLabel.SetBackgroundColor( s_ciColorLightGreen );
    m_ciSplashLabel.SetWidth( s_iMenuWidth );
    m_ciSplashLabel.SetHeight( s_iDefaultLabelHeight );
    m_ciSplashLabel.SetCursor( Cursor_GraphicID );
    m_ciSplashLabel.ShowCursor( true );
    m_ciSplashLabel.SetPersistentCursor( true );
    m_ciSplashLabel.SetHandlesInput( false );
    m_ciSplashLabel.AlignCenter();
    m_ciSplashLabel.SetText( ::rumGetString( token_press_any_key_client_StringID ) );

    m_ciStatLabel = ::rumCreateControl( Label );
    m_ciStatLabel.SetBackgroundColor( s_ciColorDarkBlue );
    m_ciStatLabel.SetWidth( s_iBorderPixelWidth * 10 );
    m_ciStatLabel.SetHeight( s_iBorderPixelWidth );
    m_ciStatLabel.SetHandlesInput( false );
    m_ciStatLabel.AlignCenter();

    m_ciStatListView = ::rumCreateControl( ListView );
    m_ciStatListView.SetBackgroundColor( s_ciColorDarkBlue );
    m_ciStatListView.SetWidth( s_iBorderPixelWidth * 15 );
    m_ciStatListView.SetHeight( s_iBorderPixelWidth * 8 );
    m_ciStatListView.SetDelimiter( "|" );
    m_ciStatListView.SetFormat( "0.05|0.5" );
    m_ciStatListView.ShowScrollbar( true );
    m_ciStatListView.SetScrollbarWidth( 2 );
    m_ciStatListView.SetScrollbarColors( s_ciColorBlueScrollbarHandle, s_ciColorBlueScrollbar );
    m_ciStatListView.SetHandlesInput( false );
    m_ciStatListView.HighlightCurrentIndex( false );

    foreach( i, ciItem in m_ciTextBoxArray )
    {
      m_ciTextBoxArray[i] = ::rumCreateControl( TextBox );
      m_ciTextBoxArray[i].SetBackgroundColor( s_ciColorDarkBlue );
      m_ciTextBoxArray[i].SetWidth( s_iTextBoxWidth );
      m_ciTextBoxArray[i].SetHeight( s_iDefaultLabelHeight );
      m_ciTextBoxArray[i].SetPrompt( Prompt_GraphicID );
      m_ciTextBoxArray[i].ShowPrompt( true );
      m_ciTextBoxArray[i].SetCursor( Cursor_GraphicID );
      m_ciTextBoxArray[i].ShowCursor( true );
      m_ciTextBoxArray[i].AlignLeft();
    }

    m_ciTitleLabel = ::rumCreateControl( Label );
    m_ciTitleLabel.SetBackgroundColor( ::rumColorBlack );
    m_ciTitleLabel.SetWidth( iScreenWidth );
    m_ciTitleLabel.SetHeight( s_iDefaultLabelHeight );
    m_ciTitleLabel.SetHandlesInput( false );
    m_ciTitleLabel.AlignCenter();
    m_ciTitleLabel.SetText( ::rumGetString( title_introduction_client_StringID ) );

    m_ciTitleListMenu = ::rumCreateControl( ListView );
    m_ciTitleListMenu.SetBackgroundColor( ::rumColorBlack );
    m_ciTitleListMenu.SetWidth( g_ciUI.s_iMenuWidth );
    m_ciTitleListMenu.SetHeight( 128 );
    m_ciTitleListMenu.ShowScrollbar( true );
    m_ciTitleListMenu.SetFormat( "1.0" );
    m_ciTitleListMenu.AlignCenter();
    m_ciTitleListMenu.SetScrollbarWidth( 2 );
    m_ciTitleListMenu.SetScrollbarColors( s_ciColorBlueScrollbarHandle, s_ciColorBlueScrollbar );

    m_ciTitleTextView = ::rumCreateControl( TextView );
    m_ciTitleTextView.SetBackgroundColor( s_ciColorDarkBlue );
    m_ciTitleTextView.SetWidth( iTitleWidth );
    m_ciTitleTextView.SetHeight( 96 );
    m_ciTitleTextView.SetBufferHeight( 1024 );
    m_ciTitleTextView.ShowScrollbar( false );
    m_ciTitleTextView.SetNewestFirst( true );

    m_ciYesNoListView = ::rumCreateControl( ListView );
    m_ciYesNoListView.SetBackgroundColor( s_ciColorDarkGreen );
    m_ciYesNoListView.SetWidth( s_iTextBoxWidth );
    m_ciYesNoListView.SetHeight( g_ciUI.s_iDefaultLabelHeight * 2 );
    m_ciYesNoListView.ShowScrollbar( false );
    m_ciYesNoListView.SetFormat( "1.0" );
    m_ciYesNoListView.AlignCenter();
    m_ciYesNoListView.SetScrollbarWidth( 2 );
    m_ciYesNoListView.SetScrollbarColors( s_ciColorBlueScrollbarHandle, s_ciColorBlueScrollbar );
    m_ciYesNoListView.SetPrompt( Prompt_GraphicID );
    m_ciYesNoListView.m_funcIndexChanged = OnYesNoListViewIndexChanged;
  }


  function Shutdown()
  {
    print("Releasing client controls...");

    m_ciChatTextView = null;
    m_ciExplorerDriveListView = null;
    m_ciExplorerFileListView = null;
    m_ciFoodLabel = null;
    m_ciGameInputTextBox = null;
    m_ciGameListView = null;
    m_ciGameRegion = null;
    m_ciGameTextView = null;
    m_ciGoldLabel = null;
    m_ciInfoLabel = null;
    m_ciMapNameLabel = null;
    m_ciScreenRegion = null;
    m_ciSplashLabel = null;
    m_ciStatLabel = null;
    m_ciStatListView = null;
    m_ciTextBoxArray.clear();
    m_ciTextBoxArray = null;
    m_ciTitleLabel = null;
    m_ciTitleListMenu = null;
    m_ciTitleTextView = null;
    m_ciYesNoListView = null;

    print(" done.\n");
  }
}


g_ciCUO <- GameSingleton();
g_ciUI <- InterfaceSingleton();


function OnConnectionComplete()
{
  g_ciCUO.m_bWaitingForResponse = false;
  InitTitleStage( TitleStages.VerifyGameFiles );
}


function OnConnectionFailed()
{
  g_ciCUO.m_bWaitingForResponse = false;
  g_ciUI.m_ciInfoLabel.SetText( ::rumGetString( msg_connect_fail_client_StringID ) );
  InitTitleStage( TitleStages.Connect );
}


function OnGameInit( i_fClientTime, i_strServer, i_strPort, i_strAccountName, i_strLanguage )
{
  print( "Client initialization\n" );

  ::rumSetWindowTitle( "Classic Ultima Online" );

  // Set the sound attenuation distance, slightly better than half the number of tiles that can be seen
  ::rumSetMinMaxSoundDistance( 0.0, g_ciUI.s_iVisibleMapTilesStandard / 1.5 );

  g_ciCUO.m_eWindDirection = Direction.West;

  // Seed the random number generator
  srand( time() );

  g_ciCUO.m_fTileAnimationTimer = g_ciCUO.m_fMainTimer = i_fClientTime;

  g_ciCUO.m_ciGameConfigTable = ::rumReadConfig();

  local bUpdateConfig = false;

  // Provide reasonable defaults for any missing values in the config
  if( !( "cuo:username" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:username"] <- i_strAccountName;
    bUpdateConfig = true;
  }

  if( !( "cuo:server" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:server"] <- i_strServer;
    bUpdateConfig = true;
  }

  if( !( "cuo:port" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:port"] <- i_strPort;
    bUpdateConfig = true;
  }

  if( !( "cuo:ultima4" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:ultima4"] <- "C:/GOG Games/Ultima 4/";
    bUpdateConfig = true;
  }

  if( !( "cuo:ultima3" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:ultima3"] <- "C:/GOG Games/Ultima 3/";
    bUpdateConfig = true;
  }

  if( !( "cuo:ultima2" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:ultima2"] <- "C:/GOG Games/Ultima 2/";
    bUpdateConfig = true;
  }

  if( !( "cuo:ultima1" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:ultima1"] <- "C:/GOG Games/Ultima 1/";
    bUpdateConfig = true;
  }

  if( !( "cuo:ultima" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:ultima"] <- "4";
    bUpdateConfig = true;
  }

  if( !( "cuo:graphics_pack" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:graphics_pack"] <- g_aGraphicsPacksArray[0][1];
    bUpdateConfig = true;
  }

  if( !( "cuo:music_enabled" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:music_enabled"] <- "Yes";
    bUpdateConfig = true;
  }

  if( !( "cuo:sfx_enabled" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:sfx_enabled"] <- "Yes";
    bUpdateConfig = true;
  }

  if( !( "cuo:master_volume" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:master_volume"] <- "100";
    bUpdateConfig = true;
  }

  if( !( "cuo:music_volume" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:music_volume"] <- "100";
    bUpdateConfig = true;
  }

  if( !( "cuo:sfx_volume" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:sfx_volume"] <- "100";
    bUpdateConfig = true;
  }

  if( !( "cuo:sound_pack" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:sound_pack"] <- g_aSoundPacksArray[0][1];
    bUpdateConfig = true;
  }

  if( !( "cuo:show_combat" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:show_combat"] <- "Yes";
    bUpdateConfig = true;
  }

  if( !( "cuo:show_moves" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:show_moves"] <- "Yes";
    bUpdateConfig = true;
  }

  if( !( "cuo:show_names" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:show_names"] <- "Yes";
    bUpdateConfig = true;
  }

  if( !( "cuo:show_vitals" in g_ciCUO.m_ciGameConfigTable ) )
  {
    g_ciCUO.m_ciGameConfigTable["cuo:show_vitals"] <- "Yes";
    bUpdateConfig = true;
  }

  if( bUpdateConfig )
  {
    ::rumWriteConfig( g_ciCUO.m_ciGameConfigTable );
  }

  g_ciUI.m_bShowCombat = ( g_ciCUO.m_ciGameConfigTable["cuo:show_combat"] == "Yes" ? true : false );
  g_ciUI.m_bShowMoves = ( g_ciCUO.m_ciGameConfigTable["cuo:show_moves"] == "Yes" ? true : false );
  g_ciUI.m_bShowNames = ( g_ciCUO.m_ciGameConfigTable["cuo:show_names"] == "Yes" ? true : false );
  g_ciUI.m_bShowVitals = ( g_ciCUO.m_ciGameConfigTable["cuo:show_vitals"] == "Yes" ? true : false );

  g_ciCUO.m_strGraphicsPack = g_ciCUO.m_ciGameConfigTable["cuo:graphics_pack"];

  // Verify that the graphics pack is valid
  local bPackFound = false;
  foreach( strGraphicsPackArray in g_aGraphicsPacksArray )
  {
    if( strGraphicsPackArray[1] == g_ciCUO.m_strGraphicsPack )
    {
      bPackFound = true;
    }
  }

  if( !bPackFound )
  {
    // Use the first pack in the array as the default
    g_ciCUO.m_strGraphicsPack = g_aGraphicsPacksArray[0][1];
  }

  g_ciCUO.m_bMusicEnabled = ( g_ciCUO.m_ciGameConfigTable["cuo:music_enabled"] == "Yes" ? true : false );
  g_ciCUO.m_bSFXEnabled = ( g_ciCUO.m_ciGameConfigTable["cuo:sfx_enabled"] == "Yes" ? true : false );

  g_ciCUO.m_uiMasterVolume = g_ciCUO.m_ciGameConfigTable["cuo:master_volume"].tointeger();
  g_ciCUO.m_uiMusicVolume = g_ciCUO.m_ciGameConfigTable["cuo:music_volume"].tointeger();
  g_ciCUO.m_uiSFXVolume = g_ciCUO.m_ciGameConfigTable["cuo:sfx_volume"].tointeger();

  g_ciCUO.m_strSoundPack = g_ciCUO.m_ciGameConfigTable["cuo:sound_pack"];

  // Verify that the sound pack is valid
  bPackFound = false;
  foreach( strSoundPackArray in g_aSoundPacksArray )
  {
    if( strSoundPackArray[1] == g_ciCUO.m_strSoundPack )
    {
      bPackFound = true;
    }
  }

  if( !bPackFound )
  {
    // Use the first pack in the array as the default
    g_ciCUO.m_strSoundPack = g_aSoundPacksArray[0][1];
  }

  // Some things to remember about fonts
  // 1. If you want dynamic color blitting, the face should be white
  // 2. Outlines in any color will always change to black during dynamic
  //    color changes
  // 3. Use attrib.blendFace = false for very cheap pseudo-outline
  // 4. If you use a single color most of the time and it is not white,
  //    consider making that color its own separate font. Use the white
  //    version for color changes.

  local fp = rumFontProps();
  fp.PixelHeight = 13;
  fp.FaceColor = ::rumColorWhite;
  fp.BlendFace = false;
  ::rumCreateFont( "fonts/tahoma.ttf", "default", fp );
  fp.PixelHeight = 14;
  ::rumCreateFont( "fonts/runes.ttf", "runes", fp );

  fp.PixelHeight = 10;
  fp.FaceColor = ::rumColorWhite;
  ::rumCreateFont( "fonts/candarab.ttf", "small", fp );

  // How to extract an archive (for testing)
  //::rumExtractArchive(rumGamePath + "/graphics/tiles_steele2.pkg");

  print( "Creating tiles...\n" );

  ::rumLoadGraphic( U4_Map_GraphicID );
  ::rumLoadGraphic( U4_Title_Splash_GraphicID );
  ::rumLoadGraphic( U3_Map_GraphicID );
  ::rumLoadGraphic( U2_Map_GraphicID );
  ::rumLoadGraphic( U2_Time_Periods_GraphicID );
  ::rumLoadGraphic( U1_Map_Danger_Despair_GraphicID );
  ::rumLoadGraphic( U1_Map_Dark_Unknown_GraphicID );
  ::rumLoadGraphic( U1_Map_Feudal_Lords_GraphicID );
  ::rumLoadGraphic( U1_Map_Lord_British_GraphicID );
  ::rumLoadGraphic( File_GraphicID );
  ::rumLoadGraphic( Folder_GraphicID );
  ::rumLoadGraphic( Mouse_Cursor_GraphicID );
  ::rumLoadGraphic( Target_GraphicID );
  ::rumLoadGraphic( Volume_GraphicID );

  ::rumLoadGraphicArchive( g_ciCUO.m_strGraphicsPack );

  ::rumAddMapArchive( "maps_u4.pkg" );
  ::rumAddMapArchive( "maps_u3.pkg" );
  ::rumAddMapArchive( "maps_u2.pkg" );
  ::rumAddMapArchive( "maps_u1.pkg" );

  ::rumLoadSoundArchive( g_ciCUO.m_strSoundPack );

  ::rumLoadSound( Blocked_SoundID );
  ::rumLoadSound( Creature_Hit_SoundID );
  ::rumLoadSound( Creature_Miss_SoundID );
  ::rumLoadSound( Creature_Spell_SoundID );
  ::rumLoadSound( Player_Hit_SoundID );
  ::rumLoadSound( Player_Miss_SoundID );
  ::rumLoadSound( Player_Spell_SoundID );
  ::rumLoadSound( Tremor_SoundID );
  ::rumLoadSound( Step_SoundID );

  g_ciUI.InitControls();
  g_ciUI.CalculateOffsets();

  g_ciUI.m_ciScreenRegion.SetActive( true );
  g_ciUI.m_ciScreenRegion.Focus();

  InitTitle();
  Ultima_Init();
  InitSettings();

  PlayMusic( U4_Towns_SoundID );

  // Test strings
  for( local eTileID = 0x40000000; eTileID <= 0x4000ffff; ++eTileID )
  {
    local ciTile = ::rumGetTileAsset( eTileID );
    if( ciTile != null )
    {
      local strDesc = ::rumGetStringByName( ciTile.GetName() + "_Tile_client_StringID" );
      if( "" == strDesc )
      {
        print( "No description for tile: " + ciTile.GetName() + "\n");
      }
    }
  }

  for( local eCreatureID = 0x00000000; eCreatureID <= 0x0000ffff; ++eCreatureID )
  {
    local ciCreature = ::rumGetCreatureAsset( eCreatureID );
    if( ciCreature != null )
    {
      local strDesc = ::rumGetStringByName( ciCreature.GetName() + "_Creature_client_StringID" );
      if( "" == strDesc )
      {
        print( "No description for creature: " + ciCreature.GetName() + "\n");
      }
    }
  }

  for( local ePortalID = 0x10000000; ePortalID <= 0x1000ffff; ++ePortalID )
  {
    local ciPortal = ::rumGetPortalAsset( ePortalID );
    if( ciPortal != null )
    {
      local strDesc = ::rumGetStringByName( ciPortal.GetName() + "_Portal_client_StringID" );
      if( "" == strDesc )
      {
        print( "No description for portal: " + ciPortal.GetName() + "\n");
      }
    }
  }

  for( local eWidgetID = 0x20000000; eWidgetID <= 0x2000ffff; ++eWidgetID )
  {
    local ciWidget = ::rumGetWidgetAsset( eWidgetID );
    if( ciWidget != null )
    {
      local strDesc = ::rumGetStringByName( ciWidget.GetName() + "_Widget_client_StringID" );
      if( "" == strDesc )
      {
        print( "No description for widget: " + ciWidget.GetName() + "\n");
      }
    }
  }

  return true;
}


function OnAccountCreationFailed( i_strReason )
{
  g_ciCUO.m_bWaitingForResponse = false;
  g_ciUI.m_ciInfoLabel.SetText( ::rumGetStringByName( i_strReason + "_shared_StringID" ) );
}


function OnAccountCreationSuccess()
{
  g_ciCUO.m_bWaitingForResponse = false;

  if( ::rumAccountLogin( g_ciUI.m_ciTextBoxArray[NewAccountMenu.Name].GetText(),
                         g_ciUI.m_ciTextBoxArray[NewAccountMenu.Password].GetText() ) == 0 )
  {
    g_ciCUO.m_bWaitingForResponse = true;
  }
  else
  {
    InitTitleStage( TitleStages.AccountMenu );

    local strError = ::rumGetLastErrorString();
    if( strError != null )
    {
      g_ciUI.m_ciInfoLabel.SetText( ::rumGetStringByName( strError + "_shared_StringID" ) );
    }
  }
}


function OnAccountLoginSuccess()
{
  g_ciCUO.m_bWaitingForResponse = false;
  LoginTitleStage.EndStage( TitleStages.MainMenu );
}


function OnAccountLoginFailed( i_strReason )
{
  g_ciCUO.m_bWaitingForResponse = false;
  g_ciUI.m_ciInfoLabel.SetText( ::rumGetStringByName( i_strReason + "_shared_StringID" ) );
}


function OnGameShutdown()
{
  print( "Ultima is shutting down...\n" );

  if( g_ciCUO != null )
  {
    g_ciCUO.Shutdown();
    g_ciCUO = null;
  }

  if( g_ciCharGen != null )
  {
    g_ciCharGen.Shutdown();
    g_ciCharGen = null;
  }

  if( g_ciUI != null )
  {
    g_ciUI.Shutdown()
    g_ciUI = null;
  }

  print( "Shutdown complete.\n" );
}


function OnGameListViewIndexChanged( i_iIndex )
{
  local strText = g_ciUI.m_ciGameListView.GetCurrentEntry();
  if( strText )
  {
    g_ciUI.m_ciGameInputTextBox.SetText( strText );
  }
  else
  {
    g_ciUI.m_ciGameInputTextBox.Clear();
  }
}


function OnPatchComplete( i_bRestart )
{
  if( i_bRestart )
  {
    g_ciUI.m_ciInfoLabel.SetText( ::rumGetString( msg_client_restarting_client_StringID ) );
    ::rumRestartClient();
    return;
  }

  VerifyPatchTitleStage.EndStage();
}


function OnPatchFile( i_strFile, i_uiPercent )
{
  VerifyPatchTitleStage.Update( i_strFile, i_uiPercent );
}


function OnWindowSizeChanged( i_iWidth, i_iHeight )
{
  // If the resolution menu is open, we'll need to update it
  if( g_ciUI.m_ciSettingsMenuTertiary != null && g_ciUI.m_ciSettingsMenuTertiary.IsActive() )
  {
    // Technically, the tertiary menu can be used by anything, but we can determine if the resolution menu is open by
    // taking a look at the listview handler
    if( g_ciUI.m_ciSettingsMenuTertiary.m_funcAccept == OnResolutionMenuAccepted )
    {
      // The resolution menu is open, so we should update it
      InitSettingsResolutionMenu( g_ciUI.m_ciSettingsMenuTertiary.GetSelectedIndex() );
    }
  }
}


function OnYesNoListViewIndexChanged( i_iIndex )
{
  local strText = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
  if( strText )
  {
    g_ciUI.m_ciGameInputTextBox.SetText( strText );
  }
  else
  {
    g_ciUI.m_ciGameInputTextBox.Clear();
  }
}
