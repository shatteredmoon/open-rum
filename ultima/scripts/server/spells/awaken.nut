function CastAwaken( i_ciCaster, i_ciTarget, i_ciSpell )
{
  if( !( i_ciCaster && i_ciTarget ) )
  {
    return false;
  }

  local bSuccess = false;

  // Creature must be asleep
  if( i_ciTarget.IsUnconscious() )
  {
    // Players should not be able to awaken themselves
    if( i_ciTarget != i_ciCaster )
    {
      local ciMap = i_ciCaster.GetMap();
      local iRange = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_Range_PropertyID, 1 ) : 1;
      local iDistance = ciMap.GetTileDistance( i_ciCaster.GetPosition(), i_ciTarget.GetPosition() );
      if( iDistance <= iRange )
      {
        // Wake the creature
        i_ciTarget.RemoveProperty( Unconscious_PropertyID );
        i_ciCaster.ActionSuccess( msg_awakened_client_StringID );

        if( i_ciTarget instanceof Player )
        {
          SendClientEffect( i_ciTarget, ClientEffectType.Cast );
          i_ciTarget.ActionSuccess( msg_awakened_client_StringID );
          bSuccess = true;
        }
      }
      else
      {
        i_ciCaster.ActionFailed( msg_out_of_range_client_StringID );
      }
    }
  }
  else
  {
    i_ciCaster.ActionFailed( msg_no_effect_client_StringID );
  }

  return bSuccess;
}
