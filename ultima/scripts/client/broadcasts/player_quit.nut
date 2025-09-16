// Sent from client
class Player_Quit_Broadcast extends rumBroadcast
{
  function OnRecv()
  {
    g_ciCUO.m_eCurrentGameMode = GameMode.Title;
    g_ciCUO.m_bReceivingData = false;

    Ultima_ListSelectionEnd();

    InitTitleStage( TitleStages.Splash );
  }
}
