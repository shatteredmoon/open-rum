function CastGateTravel( i_ciCaster, i_eDir )
{
  // Note: It takes 20 seconds to close a moongate (controlled by Ultima4World.s_fMoonPhaseInterval)
  // Player moongates are held open for 10 seconds, plus the 20 seconds it takes them to close.
  local ciMap = i_ciCaster.GetMap();
  if( ciMap instanceof U4_Britannia_Map )
  {
    local ciPos = i_ciCaster.GetPosition() + GetDirectionVector( i_eDir );
    local ciPosData = ciMap.GetPositionData( ciPos );
    local eTileID = ciPosData.GetTileID();

    // Moongates can only be created on grass tiles
    if( U4_Grass_TileID == eTileID )
    {
      local ciMoongate = ::rumCreate( U4_Moongate_WidgetID );
      if( ciMoongate != null )
      {
        ciMoongate.m_bPlayerCreated = true;
        ciMoongate.SetVisibility( true );
        ciMoongate.OnMoonPhaseChange();

        if( ciMap.AddPawn( ciMoongate, ciPos ) )
        {
          ::rumSchedule( ciMoongate, ciMoongate.Expire, 10.0 );
        }
        else
        {
          i_ciCaster.ActionFailed( msg_not_here_client_StringID );
        }
      }
    }
    else
    {
      i_ciCaster.ActionFailed( msg_not_here_client_StringID );
    }
  }
  else
  {
    i_ciCaster.ActionFailed( msg_outdoors_only_client_StringID );
  }
}
