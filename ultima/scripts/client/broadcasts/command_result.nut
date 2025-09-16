// Received from server to show results of a player action or game event
class Command_Result_Broadcast extends rumBroadcast
{
  var1 = ""; // Database string identifier
  var2 = g_strColorTagArray.White; // Optional color tag
  var3 = true; // Whether received string should be fetch from db on client

  function OnRecv()
  {
    if( msg_turning_client_StringID == var1 )
    {
      local ciPlayer = ::rumGetMainPlayer();
      local ciTransport = ciPlayer.GetTransport();
      if( ciTransport != null )
      {
        local strName = ::rumGetStringByName( ciTransport.GetName() + "_client_StringID" );
        local strDesc = format( "%s %s", ::rumGetString( msg_turning_client_StringID ), strName );
        ShowString( strDesc, var2 );
      }
    }
    else
    {
      local strDesc = var1;
      if( true == var3 )
      {
        strDesc = ::rumGetString( var1 );
      }

      // Utility function
      ShowString( strDesc, var2 );

      if( msg_blocked_client_StringID == var1 )
      {
        PlaySound( Blocked_SoundID );
      }
      else if( ( msg_gold_pilfered_client_StringID == var1 ) ||
               ( msg_food_pilfered_client_StringID == var1 ) ||
               ( msg_evaded_client_StringID == var1 ) )
      {
        PlaySound( Player_Miss_SoundID );
      }

      if( msg_whirlpool_ambrosia_client_StringID == var1 )
      {
        local ciPlayer = ::rumGetMainPlayer();
        ::rumSchedule( ciPlayer, ciPlayer.AmbrosiaWelcome, 4.0 );
      }
    }
  }
}
