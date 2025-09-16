function CastQuickness( i_ciCaster, i_ciSpell, i_iNumTargets )
{
  if( i_ciCaster.Quicken() )
  {
    --i_iNumTargets;
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
      if( ciMember != null && ciMember != i_ciCaster && ciMember.GetMap() == ciMap )
      {
        local iDistance = ciMap.GetTileDistance( ciPos, ciMember.GetPosition() );
        if( iDistance <= iRange )
        {
          if( i_iNumTargets > 0 )
          {
            // This spell does not require line of sight!
            if( ciMember.Quicken() )
            {
              SendClientEffect( ciMember, ClientEffectType.Cast );
              --i_iNumTargets;
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
