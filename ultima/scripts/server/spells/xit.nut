function CastXit( i_ciCaster, i_ciSpell )
{
  local ciMap = i_ciCaster.GetMap();
  local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
  if( MapType.Dungeon != eMapType && MapType.Tower != eMapType )
  {
    i_ciCaster.ActionFailed( msg_dungeons_only_client_StringID );
    return;
  }

  local eExitMapID = ciMap.GetExitMapID();
  if( rumInvalidAssetID == eExitMapID )
  {
    i_ciCaster.ActionFailed( msg_hmmm_no_effect_client_StringID );
    return;
  }

  local ciExitMap = GetOrCreateMap( this, eExitMapID );
  if( null == ciExitMap )
  {
    i_ciCaster.ActionFailed( msg_hmmm_no_effect_client_StringID );
    return;
  }

  local ciParty = i_ciCaster.GetParty();
  if( ciParty != null )
  {
    local iRange = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_Range_PropertyID, 1 ) : 1;
    local ciPos = i_ciCaster.GetPosition();

    // Visit each member of the party
    foreach( uiMemberID in ciParty.m_uiRosterTable )
    {
      local ciMember = ::rumFetchPawn( uiMemberID );
      if( ciMember != null && ciMember != i_ciCaster && ( ciMember.GetMap() == ciMap ) )
      {
        local iDistance = ciMap.GetTileDistance( ciPos, ciMember.GetPosition() );
        if( iDistance <= iRange )
        {
          SendClientEffect( ciMember, ClientEffectType.Cast );

          ciMap.Exit( ciMember, ciExitMap );
          ciMember.ExtinguishLight();

          SendClientEffect( ciMember, ClientEffectType.Cast );
        }
      }
    }
  }

  ciMap.Exit( i_ciCaster, ciExitMap );
  i_ciCaster.ExtinguishLight();

  SendClientEffect( i_ciCaster, ClientEffectType.Cast );
}
