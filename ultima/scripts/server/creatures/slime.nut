class U4_Slime_Creature extends U4_NPC
{
  function Damage( i_iAmount, i_ciSource, i_eWeaponType = rumInvalidAssetID, i_bSendClientEffect = true )
  {
    local bDamaged = base.Damage( i_iAmount, i_ciSource, i_eWeaponType, i_bSendClientEffect );

    // If this is not fire damage, there is a chance for division if the slime fails a strength check
    if( bDamaged && !( IsDead() || ( i_eWeaponType == null ) || i_eWeaponType.s_bFire ) )
    {
      if( !SkillRoll( GetStrength() ) )
      {
        // Slime divides - create a new temporary slime creature
        local ciSlime = ::rumCreate( U4_Slime_CreatureID );

        local bPlaced = false;

        // Start at a random direction and move clockwise until the item can be placed
        local ciMap = GetMap();
        local ciPos = GetPosition();
        local eDir = GetRandomDirection();
        for( local i = 0; i < 8; ++i, ++eDir )
        {
          eDir = eDir % 8;

          local ciNewPos = ciPos + GetDirectionVector( eDir );

          // Add the new slime to the map
          if( ciMap.MovePawn( ciSlime, ciNewPos,
                              rumIgnoreDistanceMoveFlag | rumIgnorePawnCollisionMoveFlag |
                              rumTestMoveFlag ) == rumSuccessMoveResultType )
          {
            if( ciMap.AddPawn( ciSlime, ciNewPos ) )
            {
              ciSlime.m_ciOriginPos = ciNewPos;
              bPlaced = true;
              break;
            }
          }
        }

        if( !bPlaced )
        {
          // Just add the slime at the original slime's location
          if( ciMap.AddPawn( ciSlime, ciPos ) )
          {
            ciSlime.m_ciOriginPos = ciPos;
            bPlaced = true;
          }
        }

        if( bPlaced )
        {
          ciSlime.m_bRespawns = false;
          ciSlime.m_eDefaultPosture = m_eDefaultPosture;
          ciSlime.m_eDefaultNpcType = m_eDefaultNpcType;

          local iHitpoints = min( i_iAmount, GetMaxHitpoints() );
          ciSlime.SetProperty( Hitpoints_PropertyID, iHitpoints );

          ::rumSchedule( ciSlime, ciSlime.AIDetermine, frandVariance( m_fTimeDecideVariance ) );
        }
      }
    }
  }
}
