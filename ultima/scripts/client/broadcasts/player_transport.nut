// Sent from client when player klimbs or descends on various transports (such as the balloon)
class Player_Transport_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;

  constructor( ... )
  {
    base.constructor();

    if( vargv.len() >= 1 )
    {
      var1 = vargv[0];
      if( vargv.len() >= 2 )
      {
        var2 = vargv[1];
      }
    }
  }


  function OnRecv()
  {
    local ciPlayer = ::rumGetMainPlayer();
    if( null == ciPlayer )
    {
      return;
    }

    local ciTransport = ciPlayer.GetTransport();
    local eCommandType = var1;

    if( TransportCommandType.ShuttleLaunch == eCommandType )
    {
      if( ciTransport.GetAssetID() == U1_Shuttle_WidgetID )
      {
        ciTransport.BeginLaunchSequence();
      }
    }
    else if( TransportCommandType.ShuttleLaunchAbort == eCommandType )
    {
      if( ciTransport.GetAssetID() == U1_Shuttle_WidgetID )
      {
        ciTransport.AbortLaunchSequence();
      }
    }
    else if( TransportCommandType.TimeMachineLaunchFailed == eCommandType )
    {
      local strDesc = format( "<b>%s", var2[0] );
      ShowString( strDesc, g_strColorTagArray.White );

      strDesc = format( "<b>%s", ::rumGetString( msg_time_machine_failed_usage_client_StringID ) );
      ShowString( strDesc, g_strColorTagArray.Red );
    }
    else if( ( TransportCommandType.TimeMachineLaunchMondain == eCommandType ) ||
             ( TransportCommandType.TimeMachineLaunchEnd == eCommandType ) )
    {
      BlockInput( RAND_MAX );
      ciPlayer.LaunchTimeMachine( var2 );
    }
  }
}
