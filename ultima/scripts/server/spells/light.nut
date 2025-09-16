function CastLight( i_ciCaster, i_ciSpell )
{
  local iRange = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_Range_PropertyID, 1 ) : 1;
  local fDuration = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_Duration_PropertyID, 60.0 ) : 0.0;

  local ciMap = i_ciCaster.GetMap();
  if( MapRequiresLight( ciMap ) )
  {
    // Extinguish any previous lights
    i_ciCaster.ExtinguishLight();

    // Create the spell effect
    local ciEffect = Light_Effect( true );
    ciEffect.m_uiTargetID = i_ciCaster.GetID();

    // Put the effect in the target's effect table
    i_ciCaster.m_ciEffectsTable[ciEffect] <- ciEffect;

    // The spell expires after the specified duration
    ::rumSchedule( ciEffect, ciEffect.Expire, fDuration );

    i_ciCaster.SetLightRange( iRange );
  }
  else
  {
    i_ciCaster.ActionFailed( msg_dungeons_only_client_StringID );
  }
}
