function CastYUp( i_ciCaster, i_ciSpell )
{
  local ciMap = i_ciCaster.GetMap();

  local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
  if( MapType.Dungeon != eMapType && MapType.Tower != eMapType && MapType.Abyss != eMapType )
  {
    i_ciCaster.ActionFailed( msg_dungeons_only_client_StringID );
    return;
  }

  local bNegated = ciMap.GetProperty( Map_Negates_YUp_Spell_PropertyID, false );
  if( bNegated )
  {
    // The current map does not allow blink/teleportation spells
    i_ciCaster.ActionFailed( msg_hmmm_no_effect_client_StringID );
  }

  local iMapLevel = ciMap.GetProperty( Map_Level_PropertyID, -1 );
  if( ( 1 == iMapLevel ) && ( MapType.Dungeon == eMapType ) )
  {
    // Player is leaving a dungeon from the first level, which is the same functionality as the X-it spell
    CastXit( i_ciCaster, i_ciSpell );
    return;
  }

  // Is there a map above the caster?
  local eUpperMapID = ciMap.GetProperty( Map_ID_Above_PropertyID, rumInvalidAssetID );
  if( rumInvalidAssetID == eUpperMapID )
  {
    // Couldn't find a map
    i_ciCaster.ActionFailed( msg_hmmm_no_effect_client_StringID );
    return;
  }

  local ciTargetMap = GetOrCreateMap( i_ciCaster, eUpperMapID );
  if( null == ciTargetMap )
  {
    i_ciCaster.ActionFailed( msg_hmmm_no_effect_client_StringID );
    return;
  }

  // Is the caster's position valid on the target level?
  local ciPos = i_ciCaster.GetPosition();
  local ciTargetPos = ciPos;

  // Check for collisions and out-of-boundary conditions
  local eResult = ciTargetMap.MovePawn( i_ciCaster, ciPos, rumTestMoveFlag );
  if( ( rumOffMapMoveResultType == eResult ) ||
      ( rumTileCollisionMoveResultType == eResult ) ||
      ( rumErrorMoveResultType == eResult ) )
  {
    // Teleport to the first encountered down ladder
    local ciPawnArray = ciTargetMap.GetAllPawns();
    foreach( ciPawn in ciPawnArray )
    {
      if( ciPawn.IsVisible() && ciPawn.GetProperty( Portal_Down_Ladder_PropertyID, false ) )
      {
        ciTargetPos = ciPawn.GetPosition();
        break;
      }
    }
  }

  // Transfer all grouped players within range of the caster
  local ciParty = i_ciCaster.GetParty();
  if( ciParty != null )
  {
    local iRange = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_Range_PropertyID, 1 ) : 1;

    foreach( uiMemberID in ciParty.m_uiRosterTable )
    {
      local ciMember = ::rumFetchPawn( uiMemberID );
      if( ciMember != null && ciMember != i_ciCaster && ( ciMember.GetMap() == ciMap ) )
      {
        local iDistance = ciMap.GetTileDistance( ciPos, ciMember.GetPosition() );
        if( iDistance <= iRange )
        {
          SendClientEffect( ciMember, ClientEffectType.Cast );
          ciMap.TransferPawn( ciMember, ciTargetMap, ciTargetPos );
          SendClientEffect( ciMember, ClientEffectType.Cast );
        }
      }
    }
  }

  ciMap.TransferPawn( i_ciCaster, ciTargetMap, ciTargetPos );
  SendClientEffect( i_ciCaster, ClientEffectType.Cast );
}
