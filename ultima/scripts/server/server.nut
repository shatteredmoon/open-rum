/*----------------------------------------------------------------------------

ooooo  oooo o888    o8    o88                                ooooo ooooo  oooo
 888    88   888  o888oo  oooo  oo ooo oooo     ooooooo       888   888    88
 888    88   888   888     888   888 888 888    ooooo888      888    888  88
 888    88   888   888     888   888 888 888  888    888      888     88888
  888oo88   o888o   888o  o888o o888o888o888o  88ooo88 8o    o888o     888
                                     _              _
      _/  /  _   _     _   _ _/     (_  _/  /  _   /_|     _ _/  _  _
      /  /) (-  (/ (/ (- _)  /   () /   /  /) (-  (  | \/ (/ /  (/ /
                /
----------------------------------------------------------------------------*/


class GameServer
{
  // Config
  m_strGameConfigTable = null;

  // Maps
  //m_ciPrivateMapTable = null;
  m_ciPublicMapTable = null;
  m_ciSoloMapTable = null;

  // Parties
  m_uiPartyTable = null;
  m_uiPartyIndex = 0;

  // Timers
  m_fServerTime = 0.0;

  // Worlds
  m_ciUltima1World = null;
  m_ciUltima2World = null;
  m_ciUltima3World = null;
  m_ciUltima4World = null;


  constructor()
  {
    m_ciUltima1World = Ultima1World();
    m_ciUltima2World = Ultima2World();
    m_ciUltima3World = Ultima3World();
    m_ciUltima4World = Ultima4World();

    m_uiPartyTable = {};

    //m_ciPrivateMapTable = {};
    m_ciPublicMapTable = {};
    m_ciSoloMapTable = {};
  }


  function Update( i_fElapsedTime )
  {
    m_fServerTime = i_fElapsedTime;

    m_ciUltima1World.Update();
    m_ciUltima2World.Update();
    m_ciUltima3World.Update();
    m_ciUltima4World.Update();
  }
}


// Create the server instance
g_ciServer <- GameServer();


function OnFrameStart( i_fElapsedTime )
{
  g_ciServer.Update( i_fElapsedTime );
}


function OnGameInit( i_fProgramTime )
{
  print( "Server initialization\n" );

  g_ciServer.m_fServerTime = i_fProgramTime;

  // Seed the random number generator
  srand( time() );

  g_ciServer.m_strGameConfigTable = ::rumReadConfig();
  PrintContainer( g_ciServer.m_strGameConfigTable );

  // Initialize the game
  U1_Init();
  U2_Init();
  U3_Init();
  U4_Init();

  return true;
}


function OnGameShutdown()
{
  print( "Server initiating script shutdown...\n" );

  ReleaseAllMaps();

  g_ciServer = null;

  print( "Script shutdown complete.\n" );
}
