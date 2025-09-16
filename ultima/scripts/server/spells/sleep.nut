function CastSleep( i_ciCaster, i_ciSpell )
{
  // The sleep spell will put as many targets to sleep as the caster's level
  local iTargets = i_ciCaster.GetProperty( U4_Level_PropertyID, 1 );
  local eAlignmentType = i_ciCaster.GetAlignment();

  // Get all creatures within range
  local ciMap = i_ciCaster.GetMap();
  local iRange = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_Range_PropertyID, 1 ) : 1;
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
        SendClientEffect( ciPawn, ClientEffectType.Cast );

        if( !ciPawn.ResistsSleep() )
        {
          ciPawn.Incapacitate();
        }

        --iTargets;
      }
    }
    else
    {
      // Can't affect anymore targets
      break;
    }
  }
}
