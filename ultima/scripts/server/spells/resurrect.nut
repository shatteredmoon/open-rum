function CastResurrect( i_ciCaster, i_eDir, i_eStatID, i_eRemainsType, i_bAffectVirtue )
{
  local bResurrected = false;

  local ciPos = i_ciCaster.GetPosition() + GetDirectionVector( i_eDir );
  local ciMap = i_ciCaster.GetMap();

  local ciRemains;
  local ciPosData = ciMap.GetPositionData( ciPos );
  while( ciRemains = ciPosData.GetNext( rumWidgetPawnType, i_eRemainsType ) )
  {
    if( ciRemains.IsVisible() )
    {
      if( i_eStatID != null && ( i_eRemainsType == U4_Body_Dead_WidgetID ) )
      {
        // There is a chance of failure and reducing the body to ashes
        if( !i_ciCaster.SkillRoll( i_ciCaster.GetWisdom() ) )
        {
          i_ciCaster.ActionFailed( msg_failed_client_StringID );

          // The remains turn to ashes
          local ciAshes = ::rumCreate( U3_Ashes_WidgetID );
          if( ciAshes != null )
          {
            if( ciMap.AddPawn( ciAshes, ciPos ) )
            {
              // Set the corpse ID to the player's unique ID
              local uiGameID = ciRemains.GetProperty( Player_ID_PropertyID, rumInvalidGameID );
              ciAshes.SetProperty( Player_ID_PropertyID, uiGameID );

              // The player has a limited time to resurrect to the corpse if able
              ::rumSchedule( ciAshes, ciAshes.Expire, Player.s_fDeathInterval );

              // Reset the time the player will automatically resurrect
              local uiPlayerID = ciRemains.GetProperty( Player_ID_PropertyID, rumInvalidGameID );
              if( uiPlayerID != rumInvalidGameID )
              {
                local ciPlayer = ::rumFetchPawn( uiPlayerID );

                ciPlayer.m_iDeathIndex += 1;
                ::rumSchedule( ciPlayer, ciPlayer.OnDeathTimeout, Player.s_fDeathInterval,
                               ciPlayer.m_iDeathIndex );
              }
            }
          }

          ciMap.RemovePawn( ciRemains );

          return false;
        }
      }

      // Get the associated player
      local uiPlayerID = ciRemains.GetProperty( Player_ID_PropertyID, rumInvalidGameID );
      if( uiPlayerID != rumInvalidGameID )
      {
        local ciPlayer = ::rumFetchPawn( uiPlayerID );
        if( ciPlayer != null && ciPlayer.IsDead() )
        {
          SendClientEffect( ciPlayer, ClientEffectType.Cast );

          ciPlayer.Resurrect( ResurrectionType.Spell );
          ciMap.RemovePawn( ciRemains );

          bResurrected = true;

          // Award virtue for helping others (players do not benefit from aiding theirself)
          if( bResurrected && i_bAffectVirtue && ( ciPlayer instanceof Player ) && ciPlayer != i_ciCaster )
          {
            // Increment virtue for helping other players
            i_ciCaster.AffectVirtue( VirtueType.Honor, 1, false, true );
          }

          ciPosData.Stop();
        }
      }
    }
  }

  if( !bResurrected )
  {
    i_ciCaster.ActionFailed( msg_not_here_client_StringID );
  }

  return bResurrected;
}
