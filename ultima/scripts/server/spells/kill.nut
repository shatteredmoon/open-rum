function CastKill( i_ciCaster, i_ciTarget, i_ciSpell )
{
  if( !( i_ciCaster != null && i_ciTarget != null && i_ciSpell != null ) )
  {
    return false;
  }

  if( ( i_ciTarget instanceof NPC ) && i_ciTarget.IsVisible() && !i_ciTarget.IsUndead() )
  {
    SendClientEffect( i_ciTarget, ClientEffectType.Cast );

    if( i_ciTarget.GetAssetID() == U1_Mondain_CreatureID )
    {
      i_ciTarget.KillFailed( i_ciCaster );
    }
    else
    {
      local ePropertyType = i_ciSpell.GetProperty( Spell_Stat_ID_PropertyID, rumInvalidAssetID );
      if( rumInvalidAssetID == ePropertyType )
      {
        return;
      }

      local iCasterRoll = rand() % 100 + i_ciCaster.GetProperty( ePropertyType, 1 );
      local iTargetRoll = rand() % 100 + i_ciTarget.GetProperty( ePropertyType, 1 );

      if( iCasterRoll > iTargetRoll )
      {
        local iDamage = i_ciTarget.GetMaxHitpoints();
        if( ( iCasterRoll - iTargetRoll ) < 75 )
        {
          // Target is greatly damaged (might still die)
          iDamage = rand() % 100 + 100;
        }

        i_ciTarget.Damage( iDamage, i_ciCaster );
      }
      else
      {
        i_ciCaster.ActionFailed( msg_no_effect_client_StringID );
      }
    }
  }
  else
  {
    i_ciCaster.ActionFailed( msg_hmmm_no_effect_client_StringID );
  }

  return true;
}


function CastKillGroup( i_ciCaster, i_ciSpell, i_iNumTargets )
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

    local iCasterRoll = rand() % 100 + i_ciCaster.GetProperty( ePropertyType, 1 );
    local eAlignmentType = i_ciCaster.GetAlignment();

    local eSpellAssetID = i_ciSpell.GetAssetID();

    ShuffleArray( ciPawnArray );
    foreach( ciTarget in ciPawnArray )
    {
      if( i_iNumTargets > 0 )
      {
        if( ( i_ciTarget instanceof NPC ) && i_ciTarget.IsVisible() && !i_ciTarget.IsUndead() &&
            i_ciTarget.GetAlignment() != eAlignmentType )
        {
          local eTargetAssetID = i_ciTarget.GetAssetID();
          local bAffectsTarget = false;

          if( U3_Repond_Spell_CustomID == eSpellAssetID )
          {
            if( ( U3_Goblin_CreatureID == eTargetAssetID ) ||
                ( U3_Orc_CreatureID == eTargetAssetID )    ||
                ( U3_Troll_CreatureID == eTargetAssetID ) )
            {
              // TODO - cooldown so that this spell can't be cast often
              bAffectsTarget = true;
            }
          }
          else if( U3_Pontori_Spell_CustomID == eSpellAssetID )
          {
            if( ( U3_Ghoul_CreatureID == eTargetAssetID )    ||
                ( U3_Skeleton_CreatureID == eTargetAssetID ) ||
                ( U3_Zombie_CreatureID == eTargetAssetID ) )
            {
              // TODO - cooldown so that this spell can't be cast often
              bAffectsTarget = true;
            }
          }
          else
          {
            bAffectsTarget = true;
          }

          if( bAffectsTarget );
          {
            SendClientEffect( i_ciTarget, ClientEffectType.Cast );

            local iTargetRoll = rand() % 100 + ciTarget.GetProperty( ePropertyType, 1 );
            if( iCasterRoll > iTargetRoll )
            {
              local iDamage = ciTarget.GetMaxHitpoints();
              if( ( iCasterRoll - iTargetRoll ) < 75 )
              {
                // Target is greatly damaged (might still die)
                iDamage = rand() % 100 + 100;
              }

              ciTarget.Damage( iDamage, i_ciCaster );
            }

            --i_iNumTargets;
          }
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
