function CastProjectile( i_ciCaster, i_ciTarget, i_eWeaponID )
{
  if( !( i_ciCaster && i_ciTarget ) )
  {
    return false;
  }

  local eResult = i_ciCaster.Attack( i_ciTarget, i_eWeaponID );
  if( eResult != AttackReturnType.Success && !( i_ciCaster instanceof Player ) )
  {
    AIForgetTarget();
  }

  return true;
}


function CastProjectileGroup( i_ciCaster, i_ciSpell, i_eWeaponID, i_iNumTargets )
{
  // Get all creatures within range
  local ciMap = i_ciCaster.GetMap();
  local ciWeapon = ::rumGetAsset( i_eWeaponID );
  local iRange = ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Attack_Range_PropertyID, 1 ) : 1;
  local ciPawnArray = ciMap.GetPawns( i_ciCaster.GetPosition(), iRange, true );
  if( ciPawnArray.len() > 0 )
  {
    local eAlignmentType = i_ciCaster.GetAlignment();
    local eSpellAssetID = i_ciSpell.GetAssetID();

    ShuffleArray( ciPawnArray );
    foreach( ciPawn in ciPawnArray )
    {
      if( i_iNumTargets > 0 && ( ciPawn instanceof NPC ) && ciPawn.GetAlignment() != eAlignmentType )
      {
        if( i_ciCaster.Attack( ciPawn, i_eWeaponID ) == AttackReturnType.Success )
        {
          if( U3_Noxum_Spell_CustomID == eSpellAssetID )
          {
            // Also potentially poison the target
            ciPawn.Poison();
          }
        }

        --i_iNumTargets;
      }
    }
  }
  else
  {
    // Nothing to damage
    i_ciCaster.ActionFailed( msg_no_effect_client_StringID );
  }

  return true;
}
