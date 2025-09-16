function CastCure( i_ciCaster, i_ciTarget, i_ciSpell, i_bAffectVirtue )
{
  if( !( i_ciCaster != null && i_ciTarget != null ) )
  {
    return false;
  }

  local bSuccess = false;

  // Creature must be poisoned
  if( i_ciTarget.IsPoisoned() )
  {
    local ciMap = i_ciCaster.GetMap();
    local iDistance = ciMap.GetTileDistance( i_ciCaster.GetPosition(), i_ciTarget.GetPosition() );

    local iRange = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_Range_PropertyID, 1 ) : 1;
    if( iDistance <= iRange )
    {
      // Cure the creature
      local ciEffect = i_ciTarget.GetEffect( Poison_Effect );
      if( ciEffect )
      {
        ciEffect.Remove();
      }

      i_ciTarget.Cure();
      i_ciCaster.ActionSuccess( msg_cured_client_StringID );

      bSuccess = true;

      if( i_ciTarget instanceof Creature && i_ciTarget != i_ciCaster )
      {
        SendClientEffect( i_ciTarget, ClientEffectType.Cast );
        i_ciTarget.ActionSuccess( msg_cured_client_StringID );
      }
    }
    else
    {
      i_ciCaster.ActionFailed( msg_out_of_range_client_StringID );
    }
  }
  else
  {
    i_ciCaster.ActionFailed( msg_no_effect_client_StringID );
  }

  // Award virtue for helping others (players do not benefit from aiding theirself)
  if( bSuccess && i_bAffectVirtue && ( i_ciTarget instanceof Player ) && i_ciTarget != i_ciCaster )
  {
    // Increment virtue for helping other players
    i_ciCaster.AffectVirtue( VirtueType.Honor, 1, false, true );
  }

  return bSuccess;
}
