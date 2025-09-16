class Ladder_Up_Down_Portal extends Portal
{
  // Override the default behavior - players cannot use ladders when on a transport
  function Use( i_ciPlayer, i_eUsageType )
  {
    if( i_ciPlayer.GetTransportID() != rumInvalidGameID )
    {
      // Player cannot use this portal while on a transport - this should've been caught by the client
      i_ciPlayer.IncrementHackAttempts();
      return false;
    }

    local bSuccess = false;

    local ciMap = i_ciPlayer.GetMap();
    if( ciMap != null )
    {
      if( PortalUsageType.Klimb == i_eUsageType )
      {
        // Get the destination map and location
        local iDestMapID = GetProperty( Map_ID_PropertyID, rumInvalidAssetID );
        local ciDestMap = GetOrCreateMap( i_ciPlayer, iDestMapID );
        if( null == ciDestMap )
        {
          return false;
        }

        local ciDestPos = GetPosition();
        ciDestPos.x = GetProperty( Map_PosX_PropertyID, ciDestPos.x );
        ciDestPos.y = GetProperty( Map_PosY_PropertyID, ciDestPos.y );

        bSuccess = ciMap.TransferPawn( i_ciPlayer, ciDestMap, ciDestPos );
      }
      else if( PortalUsageType.Descend == i_eUsageType )
      {
        // Get the destination map and location
        local iDestMapID = GetProperty( Map_Alt_Destination_ID_PropertyID, rumInvalidAssetID );
        local ciDestMap = GetOrCreateMap( i_ciPlayer, iDestMapID );
        if( null == ciDestMap )
        {
          return false;
        }

        local ciDestPos = GetPosition();
        ciDestPos.x = GetProperty( Map_Alt_Destination_PosX_PropertyID, ciDestPos.x );
        ciDestPos.y = GetProperty( Map_Alt_Destination_PosY_PropertyID, ciDestPos.y );

        bSuccess = ciMap.TransferPawn( i_ciPlayer, ciDestMap, ciDestPos );
      }
      else
      {
        // Not an ascend or descend method
        i_ciPlayer.IncrementHackAttempts();
      }
    }

    return bSuccess;
  }
}
