class Portal extends rumPortal
{
  m_ciEffectsTable = null;


  constructor()
  {
    base.constructor();

    m_ciEffectsTable = {};
  }


  function AddClientEffect( i_ciClientEffect )
  {
    // A weakref is not used here because this is the only thing that keeps the object from being garbage collected
    m_ciEffectsTable[i_ciClientEffect] <- i_ciClientEffect;
  }


  function CanBeUsedBy( i_ciPlayer, i_eUsageType )
  {
    local eUsageFlags = GetProperty( Portal_Usage_Flags_PropertyID, PortalUsageType.Undef );
    if( !::rumBitAnyOn( eUsageFlags, i_eUsageType ) )
    {
      return PortalUsageResultType.Invalid;
    }

    local bBlockTransports = GetProperty( Portal_Prohibits_Transports_PropertyID, false );
    if( bBlockTransports && i_ciPlayer.GetTransportID() != rumInvalidGameID )
    {
      // Player must be on foot
      return PortalUsageResultType.NoTransports;
    }

    return PortalUsageResultType.Success;
  }


  function GetDescription()
  {
    return ::rumGetStringByName( GetName() + "_Portal_client_StringID" );
  }


  function OnAnimate( i_uiCurrentFrame )
  {
    return ( rand()%2 == 0 ? i_uiCurrentFrame + 1 : i_uiCurrentFrame );
  }


  function OnObjectReleased()
  {
    // Release all affects since they potentially hold a reference to the creature
    m_ciEffectsTable.clear();
  }


  function RemoveClientEffect( i_ciClientEffect )
  {
    delete m_ciEffectsTable[i_ciClientEffect];
  }


  function UpdateEffects()
  {}
}
