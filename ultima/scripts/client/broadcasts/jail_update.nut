// Received from server when notoriety changes on a jailed player
class Jail_Update_Broadcast extends rumBroadcast
{
  var = 0; // Notoriety

  function OnRecv()
  {
    local fMinutes = var * ( Player.s_fNotorietyReductionInterval / 60.0 );
    if( fMinutes > 0.0 )
    {
      local strDesc = format( ::rumGetString( msg_jail_time_client_StringID ), fMinutes );
      ShowString( strDesc, g_strColorTagArray.Yellow );
    }
  }
}
