function CastRangedDamage( i_ciCaster, i_ciSpell, i_iNumTargets )
{
  // Get all creatures within range
  local ciMap = i_ciCaster.GetMap();
  local iRange = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_Range_PropertyID, 1 ) : 1;
  local ciPawnArray = ciMap.GetPawns( i_ciCaster.GetPosition(), iRange, true );
  if( ciPawnArray.len() > 0 )
  {
    // Fetch the stat that will be used for the skill check
    local ePropertyType = i_ciSpell.GetProperty( Spell_Stat_ID_PropertyID, rumInvalidAssetID );
    if( rumInvalidAssetID == ePropertyType )
    {
      return;
    }

    local eAlignmentType = i_ciCaster.GetAlignment();
    local eSpellAssetID = i_ciSpell.GetAssetID();

    // Determine the caster's skill roll to check against each affected target
    local iCasterRoll = rand() % 100 + i_ciCaster.GetProperty( ePropertyType, 1 );

    ShuffleArray( ciPawnArray );
    foreach( ciPawn in ciPawnArray )
    {
      if( i_iNumTargets > 0 )
      {
        if( ciPawn.IsVisible() && ( ciPawn instanceof NPC ) && ciPawn.GetAlignment() != eAlignmentType )
        {
          local iTargetRoll = rand() % 100 + ciPawn.GetProperty( ePropertyType, 1 );
          if( iCasterRoll > iTargetRoll )
          {
            local iBaseDamage = i_ciSpell.GetProperty( Spell_Damage_PropertyID, 0 );
            local iExtraDamage = i_ciSpell.GetProperty( Spell_Secondary_Damage_PropertyID, 0 );
            local iTotalDamage = iBaseDamage + rand() % iExtraDamage;

            if( U3_Necorp_Spell_CustomID == eSpellAssetID )
            {
              local iHitpoints = ciPawn.GetHitpoints();
              if( iTotalDamage > 0 && iTotalDamage >= iHitpoints )
              {
                iTotalDamage = iHitpoints - 1;
              }
            }

            ciPawn.Damage( iTotalDamage, i_ciCaster );
            if( !ciPawn.IsDead() )
            {
              local ePostureID = i_ciSpell.GetProperty( Spell_Apply_Effect_ID_PropertyID, -1 );
              if( ePostureID != -1 )
              {
                // Update the pawn's posture
                if( PostureType.Flee == ePostureID )
                {
                  m_bFleeing = true;
                }
              }
            }
          }

          --i_iNumTargets;
        }
      }
    }
  }
  else
  {
    // Nothing to damage
    i_ciCaster.ActionFailed( msg_no_effect_client_StringID );
  }
}
