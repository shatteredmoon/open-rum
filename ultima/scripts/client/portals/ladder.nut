class Ladder_Up_Down_Portal extends Portal
{
}


class U1_Ladder_Down_Portal extends Portal
{
  function CanBeUsedBy( i_ciPlayer, i_eUsageType )
  {
    if( PortalUsageType.Descend != i_eUsageType )
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


class U1_Ladder_Up_Portal extends Portal
{
  function CanBeUsedBy( i_ciPlayer, i_eUsageType )
  {
    if( PortalUsageType.Klimb != i_eUsageType )
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


class U4_Ladder_Down_Portal extends Portal
{
  function CanBeUsedBy( i_ciPlayer, i_eUsageType )
  {
    if( PortalUsageType.Descend != i_eUsageType )
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


class U4_Ladder_Up_Down_Portal extends Ladder_Up_Down_Portal
{
  function CanBeUsedBy( i_ciPlayer, i_eUsageType )
  {
    if( PortalUsageType.Enter == i_eUsageType )
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


class U4_Ladder_Up_Portal extends Portal
{
  function CanBeUsedBy( i_ciPlayer, i_eUsageType )
  {
    if( PortalUsageType.Klimb != i_eUsageType )
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
