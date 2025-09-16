function CastTremor( i_ciCaster, i_ciSpell )
{
  // The tremor spell will damage as many targets as the caster's level
  local iTargets = i_ciCaster.GetProperty( U4_Level_PropertyID, 1 );
  local eAlignmentType = i_ciCaster.GetAlignment();

  // Get all creatures within range
  local ciMap = i_ciCaster.GetMap();
  local iRange = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_Range_PropertyID, 1 ) : 1;
  local ciPawnArray = ciMap.GetPawns( i_ciCaster.GetPosition(), iRange, false );
  ShuffleArray( ciPawnArray );

  SendClientEffect( i_ciCaster, ClientEffectType.ScreenShake );

  foreach( ciPawn in ciPawnArray )
  {
    if( iTargets > 0 )
    {
      // This spell does not require line of sight!
      if( ciPawn.IsVisible() && ( ciPawn instanceof Creature ) && ciPawn != i_ciCaster &&
          ciPawn.GetAlignment() != eAlignmentType )
      {
        // Found a creature to cast the spell on - see if it is able to resist
        if( !ciPawn.SkillRoll( ciPawn.GetDexterity() ) )
        {
          ciPawn.Damage( 75 + rand() % 25, i_ciCaster );
          --iTargets;
        }
      }
    }
  }
}
