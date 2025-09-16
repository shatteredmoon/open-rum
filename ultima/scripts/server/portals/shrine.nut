class U4_Shrine_Portal extends Portal
{
  // Players cannot use ladders when on a transport
  function Use( i_ciPlayer, i_eUsageType )
  {
    local iDestMapID = GetProperty( Map_ID_PropertyID, rumInvalidAssetID );
    if( rumInvalidAssetID == iDestMapID )
    {
      return false;
    }

    if( i_ciPlayer.GetTransportID() != rumInvalidGameID )
    {
      // Player cannot use this portal while on a transport - this should've been caught by the client
      i_ciPlayer.IncrementHackAttempts();
      return false;
    }

    local bSuccess = false;

    local ciMap = GetOrCreateMap( i_ciPlayer, iDestMapID );
    if( ciMap != null )
    {
      local eVirtue = ciMap.GetProperty( U4_Virtue_PropertyID, VirtueType.Honesty );
      if( i_ciPlayer.HasRune( eVirtue ) )
      {
        bSuccess = base.Use( i_ciPlayer, i_eUsageType );
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_missing_rune_client_StringID );
      }
    }

    return bSuccess;
  }
}
