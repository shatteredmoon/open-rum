// Sent from client when a new character has finished the gypsy introduction
// Received from server with character initialization results
class Player_Initialize_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;
  var3 = 0;

  constructor( ... )
  {
    base.constructor();

    if( 3 == vargv.len() )
    {
      var1 = vargv[0]; // Name
      var2 = vargv[1]; // Gender
      var3 = vargv[2]; // Cards/Bead selections
    }
  }

  function OnRecv()
  {
    g_ciCUO.m_bWaitingForResponse = false;

    local bInitialized = var1;
    local strCharacterName = var2;

    if( bInitialized )
    {
      // The player is ready to start the game
      local ciPlayer = ::rumGetMainPlayer();
      if( ciPlayer != null && ( ciPlayer instanceof Player ) )
      {
        ciPlayer.StartPlaying();
      }
      else
      {
        g_ciUI.m_ciChatTextView.Clear();
        g_ciUI.m_ciChatTextView.PushText( "Failed to start the game" );
      }
    }
    else
    {
      // The character has not been initialized, so switch to CharGen mode
      g_ciCUO.m_eCurrentGameMode = GameMode.CharGen;
      InitCharGen();
      g_ciCharGen.m_strCharacterName = strCharacterName;
      g_ciCharGen.InitStage( CharGen.Gender );
    }
  }
}
