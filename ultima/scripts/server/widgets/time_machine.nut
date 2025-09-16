class U1_Time_Machine_Widget extends Transport_Widget
{
  static s_bIdleCleanup = false;


  function Board( i_ciPlayer, i_bForce = false )
  {
    local ciMap = GetMap();
    local eMapAssetID = ciMap.GetAssetID();

    local bBoarded = false;

    if( U1_Time_Machine_MapID == eMapAssetID )
    {
      // Check prerequisites for using the time machine
      local iExp = i_ciPlayer.GetProperty( U1_Experience_PropertyID, 0 );
      local iLvl = iExp / 1000 + 1;

      local bSpaceAce = i_ciPlayer.GetProperty( U1_Space_Enemies_Killed_PropertyID, 0 ) >= 20;

      local iMinValue = ::rumGetMaxPropertyValue( U1_Gems_PropertyID );
      local iMaxValue = ::rumGetMaxPropertyValue( U1_Gems_PropertyID );
      local iGemFlags = i_ciPlayer.GetProperty( U1_Gems_PropertyID, iMinValue );
      if( !bSpaceAce || iLvl < 8 || iGemFlags < iMaxValue )
      {
        local strArray = [::rumGetString( u1_time_machine_1_server_StringID, i_ciPlayer.m_iLanguageID )];

        local ciBroadcast = ::rumCreate( Player_Transport_BroadcastID, TransportCommandType.TimeMachineLaunchFailed,
                                         strArray );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else if( base.Board( i_ciPlayer, i_bForce ) )
      {
        local strArray =
          [
            ::rumGetString( u1_time_machine_1_server_StringID, i_ciPlayer.m_iLanguageID ),
            ::rumGetString( u1_time_machine_2_server_StringID, i_ciPlayer.m_iLanguageID ),
            ::rumGetString( u1_time_machine_3_server_StringID, i_ciPlayer.m_iLanguageID ),
            ::rumGetString( u1_time_machine_4_server_StringID, i_ciPlayer.m_iLanguageID ),
            ::rumGetString( u1_time_machine_5_server_StringID, i_ciPlayer.m_iLanguageID ),
            ::rumGetString( u1_time_machine_6_server_StringID, i_ciPlayer.m_iLanguageID ),
            ::rumGetString( u1_time_machine_7_server_StringID, i_ciPlayer.m_iLanguageID ),
            ::rumGetString( u1_time_machine_8_server_StringID, i_ciPlayer.m_iLanguageID )
          ];

        local ciBroadcast = ::rumCreate( Player_Transport_BroadcastID, TransportCommandType.TimeMachineLaunchMondain,
                                         strArray );
        i_ciPlayer.SendBroadcast( ciBroadcast );

        bBoarded = true;
      }
    }
    else if( U1_Lair_Mondain_MapID == eMapAssetID )
    {
      local iFlags = i_ciPlayer.GetProperty( Ultima_Completions_PropertyID, 0 );
      local bKilledMondain = ::rumBitOn( iFlags, UltimaCompletions.KilledMondain );
      if( bKilledMondain && base.Board( i_ciPlayer, i_bForce ) )
      {
        local strArray =
          [
            ::rumGetString( u1_endgame_1_server_StringID, i_ciPlayer.m_iLanguageID ),
            ::rumGetString( u1_endgame_2_server_StringID, i_ciPlayer.m_iLanguageID ),
            ::rumGetString( u1_endgame_3_server_StringID, i_ciPlayer.m_iLanguageID ),
            ::rumGetString( u1_endgame_4_server_StringID, i_ciPlayer.m_iLanguageID ),
            ::rumGetString( u1_endgame_5_server_StringID, i_ciPlayer.m_iLanguageID ),
            ::rumGetString( u1_endgame_6_server_StringID, i_ciPlayer.m_iLanguageID ),
            ::rumGetString( u1_endgame_7_server_StringID, i_ciPlayer.m_iLanguageID ),
            ::rumGetString( u1_endgame_8_server_StringID, i_ciPlayer.m_iLanguageID )
          ];

        local ciBroadcast = ::rumCreate( Player_Transport_BroadcastID, TransportCommandType.TimeMachineLaunchEnd,
                                         strArray );
        i_ciPlayer.SendBroadcast( ciBroadcast );

        bBoarded = true;
      }
    }

    if( !bBoarded )
    {
      i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
    }

    return bBoarded;
  }
}
