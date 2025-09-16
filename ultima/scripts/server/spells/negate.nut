function CastNegateMagic( i_ciCaster, i_ciSpell )
{
  // Get all creatures within range
  local ciMap = i_ciCaster.GetMap();
  local iRange = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_Range_PropertyID, 1 ) : 1;
  local ciPawnArray = ciMap.GetPawns( i_ciCaster.GetPosition(), iRange, false );
  foreach( ciPawn in ciPawnArray )
  {
    // This spell does not require line of sight!
    if( ciPawn.IsVisible() && ( ciPawn instanceof Creature ) )
    {
      SendClientEffect( ciPawn, ClientEffectType.Cast );

      // Found a creature to cast the spell on - this spell affects even the caster and there is no skill
      // roll to avoid the effects of the spell
      ciPawn.Negate();
    }
  }
}
