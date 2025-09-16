// Received from client
class Player_Portal_Broadcast extends rumBroadcast
{
  var = 0;

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local eUsageType = var;

    local ciMap = i_ciPlayer.GetMap();
    local bFound = false;
    local ciPosData = ciMap.GetPositionData( i_ciPlayer.GetPosition() );
    local ciPortal;
    while( ciPortal = ciPosData.GetNext( rumPortalPawnType ) )
    {
      if( !ciPortal.IsVisible() )
      {
        continue;
      }

      local iUsageFlags = ciPortal.GetProperty( Portal_Usage_Flags_PropertyID, 0 );

      if( PortalUsageType.Descend == eUsageType )
      {
        if( ( iUsageFlags & PortalUsageType.Descend ) != 0 )
        {
          ciPortal.Use( i_ciPlayer, eUsageType );
          bFound = true;
          ciPosData.Stop();
        }
      }
      else if( PortalUsageType.Klimb == eUsageType )
      {
        if( ( iUsageFlags & PortalUsageType.Klimb ) != 0 )
        {
          ciPortal.Use( i_ciPlayer, eUsageType );
          bFound = true;
          ciPosData.Stop();
        }
      }
      else
      {
        // Entering - unlike in Ultima IV - enter can be used to klimb and descend ladders as well
        if( ( iUsageFlags & PortalUsageType.Descend ) != 0 )
        {
          eUsageType = VerticalDirectionType.Down;
        }
        else if( ( iUsageFlags & PortalUsageType.Klimb ) != 0 )
        {
          eUsageType = VerticalDirectionType.Up;
        }

        ciPortal.Use( i_ciPlayer, eUsageType );
        bFound = true;
        ciPosData.Stop();
      }
    }

    local eDelay = ActionDelay.Long;

    if( !bFound )
    {
      // Ultima IV usually says Descend What?, Klimb What?, or
      // Enter What? I think "Not here!" suffices for all three.
      i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
      eDelay = ActionDelay.Short;
    }

    local fDelay = i_ciPlayer.GetActionDelay( eDelay );
    ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
  }
}
