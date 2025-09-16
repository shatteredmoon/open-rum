class U4_Wisp_Creature extends U4_NPC
{
  constructor()
  {
    base.constructor();
    SetLightRange( 3 );
  }


  function AIAttack( i_ciAttackTarget )
  {
    TryTeleport();
    return base.AIAttack( i_ciAttackTarget );
  }


  function AIFlee( i_ciAttackTarget )
  {
    TryTeleport();
    return base.AIFlee( i_ciAttackTarget );
  }


  function AIMove( i_eDir )
  {
    TryTeleport();
    return base.AIMove( i_eDir );
  }


  function TryTeleport()
  {
    local bTeleported = false;

    // 25% chance of a teleport
    if( rand() % 100 < 25 )
    {
      // Teleport up to 5 spaces away, but within leash range of the original position
      local iOffsetX = rand() % 4 + 2;
      local iOffsetY = rand() % 4 + 2;

      if( rand() % 2 == 0 )
      {
        iOffsetX = -iOffsetX;
      }

      if( rand() % 2 == 0 )
      {
        iOffsetY = -iOffsetY;
      }

      local ciVector = rumVector( iOffsetX, iOffsetY );
      local ciPos = GetPosition();
      local ciNewPos = ciPos + ciVector;

      local ciMap = GetMap();
      if( ciMap.IsPositionWithinRadius( ciNewPos, m_ciOriginPos, s_fMaxWanderLeashRange ) )
      {
        if( ciMap.MovePawn( this, ciNewPos, rumIgnoreDistanceMoveFlag ) == rumSuccessMoveResultType )
        {
          SendClientEffect( this, ClientEffectType.Cast );
          bTeleported = true;
        }
      }
    }

    return bTeleported;
  }
}
