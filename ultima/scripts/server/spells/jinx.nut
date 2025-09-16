function CastJinx( i_ciCaster, i_ciSpell )
{
  // The jinx spell will confuse as many targets as the caster's level
  local iTargets = i_ciCaster.GetProperty( U4_Level_PropertyID, 1 );
  local eAlignmentType = i_ciCaster.GetAlignment();

  local iRange = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_Range_PropertyID, 1 ) : 1;

  // Get all creatures within range
  local ciMap = i_ciCaster.GetMap();
  local ciPawnArray = ciMap.GetPawns( i_ciCaster.GetPosition(), iRange, false );
  ShuffleArray( ciPawnArray );
  foreach( ciPawn in ciPawnArray )
  {
    if( iTargets > 0 )
    {
      // This spell does not require line of sight!
      if( ciPawn.IsVisible() && ( ciPawn instanceof Creature ) && ciPawn != i_ciCaster &&
          ciPawn.GetAlignment() != eAlignmentType )
      {
        // Found a creature to cast the spell on - see if it is able to resist
        if( ciPawn.Jinx() )
        {
          SendClientEffect( ciPawn, ClientEffectType.Cast );
          --iTargets;
        }
      }
    }
    else
    {
      // Can't affect anymore targets
      break;
    }
  }
}
