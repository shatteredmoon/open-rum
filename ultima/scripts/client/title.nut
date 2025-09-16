function InitTitle()
{
  g_ciCUO.m_fTitleTimer = g_ciCUO.m_fMainTimer;
  g_ciCUO.m_bWaitingForResponse = false;

  InitTitleStage( TitleStages.Splash );
}


function InitTitleStage( i_eStage )
{
  g_ciUI.m_ciInfoLabel.SetText( "" );

  g_ciCUO.m_eTitleStage = i_eStage;
  switch( i_eStage )
  {
    case TitleStages.Splash:              SplashTitleStage.BeginStage();          break;
    case TitleStages.VerifyClientVersion: VerifyClientTitleStage.BeginStage();    break;
    case TitleStages.VerifyUltimaInstall: VerifyUltimaTitleStage.BeginStage();    break;
    case TitleStages.Connect:             ConnectTitleStage.BeginStage();         break;
    case TitleStages.VerifyGameFiles:     VerifyPatchTitleStage.BeginStage();     break;
    case TitleStages.AccountMenu:         AccountLoginTitleStage.BeginStage();    break;
    case TitleStages.Login:               LoginTitleStage.BeginStage();           break;
    case TitleStages.MainMenu:            MainMenuTitleStage.BeginStage();        break;
    case TitleStages.CharSelect:          CharacterSelectTitleStage.BeginStage(); break;
    case TitleStages.DeleteChar:          DeleteCharacterTitleStage.BeginStage(); break;
    case TitleStages.Done:
    default:
      break;
  }
}


function RenderTitleScreen()
{
  ::rumClearScreen();

  local iScreenHeight = ::rumGetScreenHeight();
  local iScreenWidth = ::rumGetScreenWidth();
  local iScreenCenterX = iScreenWidth / 2;

  local ciGraphic = ::rumGetGraphic( U4_Title_GraphicID );
  local iOffsetY = ciGraphic.GetFrameHeight();

  // Title graphic
  local iOffsetX = iScreenCenterX - ( ciGraphic.GetFrameWidth() / 2 );
  ciGraphic.Draw( rumPoint( iOffsetX, 0 ) );

  // Daemon graphic
  ciGraphic = ::rumGetGraphic( U4_Title_Daemon_GraphicID );
  ciGraphic.DrawAnimation( rumPoint( 0, g_ciUI.m_iTitleAnimationOffset ),
                           0, g_ciUI.s_iDaemonFrameIndicesArray[g_ciUI.m_iDaemonAnimFrame] );

  // Dragon graphic
  ciGraphic = ::rumGetGraphic( U4_Title_Dragon_GraphicID );
  ciGraphic.DrawAnimation( rumPoint( iScreenWidth - ciGraphic.GetFrameWidth(), g_ciUI.m_iTitleAnimationOffset ),
                           0, g_ciUI.s_iDragonFrameIndicesArray[g_ciUI.m_iDragonAnimFrame] );

  switch( g_ciCUO.m_eTitleStage )
  {
    case TitleStages.Splash:              SplashTitleStage.Render( iOffsetY );          break;
    case TitleStages.VerifyClientVersion: VerifyClientTitleStage.Render( iOffsetY );    break;
    case TitleStages.VerifyUltimaInstall: VerifyUltimaTitleStage.Render( iOffsetY );    break;
    case TitleStages.Connect:             ConnectTitleStage.Render( iOffsetY );         break;
    case TitleStages.VerifyGameFiles:     VerifyPatchTitleStage.Render( iOffsetY );     break;
    case TitleStages.AccountMenu:         AccountLoginTitleStage.Render( iOffsetY );    break;
    case TitleStages.Login:               LoginTitleStage.Render( iOffsetY );           break;
    case TitleStages.MainMenu:            MainMenuTitleStage.Render( iOffsetY );        break;
    case TitleStages.CharSelect:          CharacterSelectTitleStage.Render( iOffsetY ); break;
    case TitleStages.DeleteChar:          DeleteCharacterTitleStage.Render( iOffsetY ); break;
  }
}


function UpdateTitleAnimations( i_fElapsedTime )
{
  if( i_fElapsedTime - g_ciCUO.m_fTitleTimer > 0.2 )
  {
    g_ciCUO.m_fTitleTimer = i_fElapsedTime;

    if( g_ciUI.m_iTitleAnimationOffset < 0 )
    {
      ++g_ciUI.m_iTitleAnimationOffset;
    }

    ++g_ciUI.m_iDaemonAnimFrame;
    if( g_ciUI.m_iDaemonAnimFrame >= g_ciUI.s_iDaemonFrameIndicesArray.len() )
    {
      g_ciUI.m_iDaemonAnimFrame = 0;
    }

    ++g_ciUI.m_iDragonAnimFrame;
    if( g_ciUI.m_iDragonAnimFrame >= g_ciUI.s_iDragonFrameIndicesArray.len() )
    {
      g_ciUI.m_iDragonAnimFrame = 0;
    }
  }
}
