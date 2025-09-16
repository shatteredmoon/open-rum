// Client
class TimeGate_Period_Broadcast extends rumBroadcast
{
  var = 0;

  function OnRecv()
  {
    g_ciCUO.m_eTimeGatePeriod = var;
  }
}
