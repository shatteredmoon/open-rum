function CastProtection( i_ciCaster, i_ciSpell )
{
  // The protection spell protects as many targets as the caster's level
  local iTargets = i_ciCaster.GetProperty( U4_Level_PropertyID, 1 );

  if( i_ciCaster.Protect() )
  {
    --iTargets;
  }

  local ciMap = i_ciCaster.GetMap();
  local ciPos = i_ciCaster.GetPosition();
  local iRange = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_Range_PropertyID, 1 ) : 1;

  local ciParty = i_ciCaster.GetParty();
  if( ciParty != null )
  {
    foreach( uiMemberID in ciParty.m_uiRosterTable )
    {
      local ciMember = ::rumFetchPawn( uiMemberID );
      if( ciMember != null && ciMember.GetMap() == ciMap )
      {
        local iDistance = ciMap.GetTileDistance( ciPos, ciMember.GetPosition() );
        if( iDistance <= iRange )
        {
          if( iTargets > 0 )
          {
            // This spell does not require line of sight!
            if( ciMember.Protect() )
            {
              SendClientEffect( ciMember, ClientEffectType.Cast );
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
    }
  }
}
