// Client
class Wind_Direction_Broadcast extends rumBroadcast
{
  var = 0; // Direction

  function OnRecv()
  {
    if( g_ciCUO.m_eWindDirection != var )
    {
      g_ciCUO.m_eWindDirection = var;
      Ultima_UpdateMapLabel();
    }
  }
}
