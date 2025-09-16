class U1_Sosaria_Portal extends Portal
{
  function Use( i_ciPlayer, i_eUsageType )
  {
    local bSuccess = false;

    // TODO - transport commander did this, right?
    local ciTransport = i_ciPlayer.GetTransport();
    if( ciTransport != null )
    {
      if( !ciTransport.IsCommander( i_ciPlayer ) )
      {
        i_ciPlayer.ActionFailed( msg_not_commander_client_StringID );
        return false;
      }

      local bKillPassengers = false;

      // If the player is on either of the space fighters, entering Sosaria's atmosphere means death
      local eAssetID = ciTransport.GetAssetID();
      if( ( U1_Space_Fighter1_WidgetID == eAssetID ) || ( U1_Space_Fighter2_WidgetID == eAssetID ) )
      {
        bKillPassengers = true;
      }

      local ciPos = ciTransport.GetPosition();
      local uiCommanderID = i_ciPlayer.GetID();

      // Remove all players from the transport
      foreach( uiPassengerID in ciTransport.m_uiPassengerTable )
      {
        if( uiPassengerID != uiCommanderID )
        {
          local ciPassenger = ::rumGetPlayer( uiPassengerID );
          if( ciPassenger != null )
          {
            ciTransport.Exit( ciPassenger, ciPos, MoveType.Celestial );
            base.Use( ciPassenger, i_eUsageType );

            if( bKillPassengers )
            {
              ciPassenger.Kill( null );
              ciPassenger.ActionFailed( msg_died_entering_atmos_client_StringID );
            }
          }
        }
      }

      // Remove the commander last
      ciTransport.Exit( i_ciPlayer, ciPos, MoveType.Celestial );
      bSuccess = base.Use( i_ciPlayer, i_eUsageType );

      if( bKillPassengers )
      {
        i_ciPlayer.Kill( null );
        i_ciPlayer.ActionFailed( msg_died_entering_atmos_client_StringID );
      }

      // Remove the transport from the game
      local ciMap = ciTransport.GetMap();
      ciMap.RemovePawn( ciTransport );
    }
    else
    {
      bSuccess = base.Use( i_ciPlayer, i_eUsageType );
      if( bSuccess )
      {
        if( !i_ciPlayer.IsDead() )
        {
          // Player entering atmosphere without a spacecraft!
          i_ciPlayer.Kill( null );
          i_ciPlayer.ActionFailed( msg_died_entering_atmos_client_StringID );
        }
      }
    }

    return bSuccess;
  }
}
