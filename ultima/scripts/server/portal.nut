class Portal extends rumPortal
{
  function Use( i_ciPlayer, i_eUsageType )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return false;
    }

    local bNoTransports = ( Portal_Prohibits_Transports_PropertyID, false );
    local uiTransportID = i_ciPlayer.GetTransportID();
    if( bNoTransports && ( uiTransportID != rumInvalidGameID ) )
    {
      // Player cannot use this portal while on a transport - this should've been caught by the client
      i_ciPlayer.IncrementHackAttempts();
      return false;
    }

    local iDestMapID = GetProperty( Map_ID_PropertyID, rumInvalidAssetID );
    local ciDestMap = GetOrCreateMap( i_ciPlayer, iDestMapID );
    if( null == ciDestMap )
    {
      return false;
    }

    local ciSourceMap = i_ciPlayer.GetMap();
    if( null == ciSourceMap )
    {
      return false;
    }

    local ciSrcPos = i_ciPlayer.GetPosition();

    local ciDestPos = rumPos( 0, 0 );
    ciDestPos.x = GetProperty( Map_PosX_PropertyID, 0 );
    ciDestPos.y = GetProperty( Map_PosY_PropertyID, 0 );

    local bSuccess = false;

    if( uiTransportID != rumInvalidGameID )
    {
      local ciTransport = i_ciPlayer.GetTransport();
      if( ciTransport != null )
      {
        // Transfer the transport ahead of the player
        bSuccess = ciTransport.Portal( iDestMapID, ciDestPos );
      }
    }
    else
    {
      ciDestMap = GetOrCreateMap( i_ciPlayer, iDestMapID );
      bSuccess = ciSourceMap.TransferPawn( i_ciPlayer, ciDestMap, ciDestPos );
    }

    // Remove player lighting
    if( bSuccess && !MapRequiresLight( ciDestMap ) )
    {
      i_ciPlayer.ExtinguishLight();
    }

    return bSuccess;
  }
}
