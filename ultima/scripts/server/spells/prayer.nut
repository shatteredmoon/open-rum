function CastPrayer( i_ciCaster, i_iRange, i_cStat )
{
  // Note: Changed somewhat from the original U1 and U2 spells. This version uses overland rules (even in dungeons),
  // but to a greater effect.

  local bApplied = false;

  // First of all, a skill roll is made to see if the spell even succeeds
  if( i_ciCaster.SkillRoll( i_ciCaster.GetWisdom() ) )
  {
    local ciMap = i_ciCaster.GetMap();
    local ciPos = i_ciCaster.GetPosition();

    // Is there an enemy nearby within LOS? If so, cast kill on it
    local ciPawnArray = ciMap.GetPawns( ciPos, i_iRange, true );
    ShuffleArray( ciPawnArray );
    foreach( ciPawn in ciPawnArray )
    {
      if( ( ciPawn instanceof NPC ) && AlignmentType.Good != ciPawn.GetAlignment() )
      {
        CastKill( i_ciCaster, ciPawn, i_cStat );
        bApplied = true;
        break;
      }
    }

    if( !bApplied )
    {
      // Is caster low on health?
      local iHitpoints = i_ciCaster.GetHitpoints();
      if( iHitpoints < 100 )
      {
        i_ciCaster.AffectHitpoints( 150 );
        bApplied = true;
      }
      else
      {
        // Is player low on food?
        local iFood = i_ciCaster.GetVersionedProperty( g_eFoodPropertyVersionArray );
        if( iFood < 50 )
        {
          i_ciCaster.AdjustVersionedProperty( g_eFoodPropertyVersionArray, 50 );
          bApplied = true;
        }
      }
    }
  }

  if( !bApplied )
  {
    // Spell failed or nothing to do
    i_ciCaster.ActionFailed( msg_hmmm_no_effect_client_StringID );
  }
}
