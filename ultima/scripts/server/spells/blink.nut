function CastBlink( i_ciCaster, i_ciSpell, i_eDir )
{
  local ciMap = i_ciCaster.GetMap();

  local bNegated = ciMap.GetProperty( Map_Negates_Blink_Spell_PropertyID, false );
  if( bNegated )
  {
    // The current map does not allow blink/teleportation spells
    i_ciCaster.ActionFailed( msg_hmmm_no_effect_client_StringID );
  }

  if( i_ciCaster.GetTransportID() != rumInvalidGameID )
  {
    // Players can't blink when on a transport of any kind
    i_ciCaster.ActionFailed( msg_only_on_foot_client_StringID );
    return;
  }

  if( !( i_eDir >= Direction.West && i_eDir <= Direction.Southwest ) )
  {
    // No movement or invalid movement specified
    i_ciCaster.ActionFailed( msg_no_effect_client_StringID );
    return;
  }

  local bObstacle = false;
  local ciPos = i_ciCaster.GetPosition();
  local ciOriginalPos = ciPos;
  local ciLastValidPos = ciPos;

  // Temporarily modify the move type for the spell
  // TODO - This is kind of gross since a message will be sent from server->client every time the move flag is changed
  local eSavedMoveType = i_ciCaster.GetMoveType();

  // Required proximity of party members
  local iRange = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_Range_PropertyID, 1 ) : 1;

  // The maximum distance a player can travel
  // TODO - Consider making this a property of the spell
  local iMaxDistance = 11;

  for( local i = 0; !bObstacle && i < iMaxDistance; ++i )
  {
    i_ciCaster.SetMoveType( MoveType.Blinks );

    ciPos = ciPos + GetDirectionVector( i_eDir );

    local eResult = ciMap.MovePawn( i_ciCaster, ciPos, rumTestMoveFlag, iMaxDistance );
    if( eResult != rumOffMapMoveResultType )
    {
      if( ( rumTileCollisionMoveResultType == eResult ) || ( rumPawnCollisionMoveResultType == eResult ) )
      {
        // The player hit an obstacle
        bObstacle = true;
      }
      else
      {
        // The player can blink through this position, but can it be used as a stopping point?
        i_ciCaster.SetMoveType( eSavedMoveType );

        eResult = ciMap.MovePawn( i_ciCaster, ciPos, rumTestMoveFlag, iMaxDistance );
        if( rumTileCollisionMoveResultType != eResult && rumPawnCollisionMoveResultType != eResult )
        {
          // The player can blink to this location
          ciLastValidPos = ciPos;
        }
      }
    }
    else
    {
      bObstacle = true;
    }
  }

  // Restore the original move type
  i_ciCaster.SetMoveType( eSavedMoveType );

  if( !ciLastValidPos.Equals( i_ciCaster.GetPosition() ) )
  {
    ciMap.MovePawn( i_ciCaster, ciLastValidPos,
                    rumIgnoreTileCollisionMoveFlag | rumIgnorePawnCollisionMoveFlag |
                    rumIgnoreDistanceMoveFlag );

    local ciParty = i_ciCaster.GetParty();
    if( ciParty != null )
    {
      foreach( uiMemberID in ciParty.m_uiRosterTable )
      {
        local ciMember = ::rumFetchPawn( uiMemberID );
        if( ciMember != null && ciMember != i_ciCaster && ( ciMember.GetMap() == ciMap ) )
        {
          local iDistance = ciMap.GetTileDistance( ciOriginalPos, ciMember.GetPosition() );
          if( iDistance <= iRange )
          {
            if( ciMember.GetTransportID() == rumInvalidGameID )
            {
              SendClientEffect( ciMember, ClientEffectType.Cast );

              // Don't blink the player if they're interacting with something else
              if( rumInvalidGameID == ciMember.m_uiInteractID )
              {
                ciMap.MovePawn( ciMember, ciLastValidPos,
                                rumIgnoreTileCollisionMoveFlag | rumIgnorePawnCollisionMoveFlag |
                                rumIgnoreDistanceMoveFlag );
              }
            }
            else
            {
              ciMember.ActionFailed( msg_only_on_foot_client_StringID );
            }
          }
        }
      }
    }
  }
  else
  {
    // No movement at all occurred
    i_ciCaster.ActionFailed( msg_blocked_client_StringID );
  }
}
