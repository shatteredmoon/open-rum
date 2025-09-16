// Client
class Moon_Phases_Broadcast extends rumBroadcast
{
  var1 = 0; // Trammel phase
  var2 = 0; // Felucca phase

  function OnRecv()
  {
    g_ciCUO.m_eTrammelPhase = var1;
    g_ciCUO.m_eFeluccaPhase = var2;
  }
}
