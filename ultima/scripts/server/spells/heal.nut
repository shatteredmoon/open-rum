function CastHeal( i_ciCaster, i_ciTarget, i_ciSpell, iAmount, i_bAffectVirtue = false )
{
  if( !( i_ciCaster && i_ciTarget ) )
  {
    return false;
  }

  local bSuccess = false;

  local iHitpoints = i_ciTarget.GetHitpoints();
  local iMaxHitpoints = i_ciTarget.GetMaxHitpoints();

  // Creature must be alive, but wounded
  if( !i_ciTarget.IsDead() && iHitpoints < iMaxHitpoints )
  {
    local ciMap = i_ciCaster.GetMap();
    local iRange = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_Range_PropertyID, 1 ) : 1;
    local iDistance = ciMap.GetTileDistance( i_ciCaster.GetPosition(), i_ciTarget.GetPosition() );
    if( iDistance <= iRange )
    {
      // Heal
      i_ciTarget.AffectHitpoints( iAmount );
      i_ciCaster.ActionSuccess( msg_healed_client_StringID );

      if( i_ciTarget instanceof Creature )
      {
        if( i_ciTarget != i_ciCaster )
        {
          SendClientEffect( i_ciTarget, ClientEffectType.Cast );
          i_ciTarget.ActionSuccess( msg_healed_client_StringID );
        }

        bSuccess = true;
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
