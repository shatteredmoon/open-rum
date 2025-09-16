class U4_Volcano_Portal extends Portal
{
  // TODO - handle PortalUsageResultType.EightPartAvatar
  function CanBeUsedBy( i_ciPlayer, i_eUsageType )
  {
    if( PortalUsageType.Enter != i_eUsageType )
    {
      return PortalUsageResultType.Invalid;
    }
    else if( i_ciPlayer.GetTransportID() != rumInvalidGameID )
    {
      // Player must be on foot
      return PortalUsageResultType.NoTransports;
    }

    return PortalUsageResultType.Success;
  }
}
