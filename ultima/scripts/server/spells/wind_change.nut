function CastWindChange( i_ciCaster, i_ciSpell, i_eDir )
{
  local ciMap = i_ciCaster.GetMap();
  local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
  if( MapType.World != eMapType )
  {
    i_ciCaster.ActionFailed( msg_outdoors_only_client_StringID );
    return;
  }

  if( i_eDir >= Direction.West && i_eDir <= Direction.Southwest )
  {
    local iRange = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_Range_PropertyID, 1 ) : 1;
    local ciPos = i_ciCaster.GetPosition();
    local ciParty = i_ciCaster.GetParty();
    if( null == ciParty )
    {
      // The player is not in the party, so create a temporary table with just the caster
      ciParty = { slot = i_ciCaster };
    }

    foreach( uiMemberID in ciParty.m_uiRosterTable )
    {
      local ciMember = ::rumFetchPawn( uiMemberID );
      if( ciMember != null && ciPlayer.GetMap() == ciMap )
      {
        local iDistance = ciMap.GetTileDistance( ciPos, ciMember.GetPosition() );
        if( iDistance <= iRange )
        {
          local fDuration = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_Duration_PropertyID, 10.0 ) : 0.0;
          ciPlayer.SetProperty( U4_Wind_Expiration_Time_PropertyID, g_ciServer.m_fServerTime + fDuration );
          ciPlayer.SetProperty( U4_Wind_Direction_Override_PropertyID, i_eDir );
          ::rumSchedule( ciMember, ciMember.WindOverrideExpire, fDuration );
        }
      }
    }
  }
  else
  {
    i_ciCaster.ActionFailed( msg_no_effect_client_StringID );
  }
}
