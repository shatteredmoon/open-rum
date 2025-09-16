//g_uiLastAttackedID <- rumInvalidGameID;

class NPC extends Creature
{
  // The max distance an NPC can move from its origin when attacking
  static s_fMaxAttackLeashRange = 13.0;

  // The max distance an NPC can move from its origin when wandering
  static s_fMaxWanderLeashRange = 7.0;

  // The max distance an NPC can path under any circumstances
  static s_iMaxPathRangeForLeashing = 64;
  static s_iMaxPathRangeForAttacking = 16;
  static s_iMaxRehomeAttempts = 5;

  // The max distance an NPC will aggro from its current position
  static s_iMaxAggroRange = 5;

  static m_fTimeDecideVariance = 0.05;

  static s_fStopFleeTime = 20.0;

  static m_fCamoCooldownTime = 7.0;

  m_bSpecialWeaponReady = false;
  m_bSpellReady = false;

  m_eSpecialWeaponArray = null;

  m_eDefaultPosture = PostureType.Idle;

  m_eDefaultNpcType = 0;

  m_iDialogueID = 0;

  m_ciAttackTargetTable = null;
  m_ciIgnoreTargetIDTable = null;
  m_uiFleeTargetID = rumInvalidGameID;

  m_ciOriginPos = null;

  m_bCamouflages = false;
  m_bRespawns = true;

  m_iActionIndex = 0;

  m_uiInteractionTable = null;

  m_uiPrimaryWeaponRange = 1;
  m_uiSpecialWeaponRange = 0;

  m_bPathing = false;
  m_iRehomeAttempts = 0;


  constructor()
  {
    base.constructor();

    m_ciAttackTargetTable = {};
    m_ciIgnoreTargetIDTable = {};
  }


  function AddAttackTarget( i_ciTarget, i_uiHateLevel )
  {
    if( null == i_ciTarget )
    {
      return;
    }

    local uiTargetID = i_ciTarget.GetID();
    if( rumInvalidGameID == uiTargetID )
    {
      return;
    }

    //if( g_uiLastAttackedID == GetID() )
    //print( format( "## %s[%x] has new attack target: %x %s\n", GetName(), GetID(), uiTargetID, i_ciTarget.GetName() ) );

    local ciMap = GetMap();
    if( ciMap != i_ciTarget.GetMap() )
    {
      return;
    }

    local ciPosition = GetPosition();
    local ciTargetPosition = i_ciTarget.GetPosition();

    local fOriginDistance = ciMap.GetDistance( m_ciOriginPos, ciTargetPosition );
    local uiTileDistance = ciMap.GetTileDistance( ciPosition, ciTargetPosition );

    local bAdjacent = ( uiTileDistance <= 1 );

    local ciAttackTarget = null;
    if( m_ciAttackTargetTable.rawin( uiTargetID ) )
    {
      // Access the existing attack target
      ciAttackTarget = m_ciAttackTargetTable[uiTargetID];
    }
    else
    {
      // Add the new attack target to the table
      ciAttackTarget = AttackTarget();
      ciAttackTarget.m_uiTargetID = i_ciTarget.GetID();
      ciAttackTarget.m_uiHateLevel = 100;

      m_ciAttackTargetTable[uiTargetID] <- ciAttackTarget;
    }

    //if( g_uiLastAttackedID == GetID() )
    //print( format( "## %s[%x] last target pos set to: %d %d\n", GetName(), GetID(), ciTargetPosition.x, ciTargetPosition.y ) );

    ciAttackTarget.m_ciLastPosition = ciTargetPosition;
    ciAttackTarget.m_uiTileDistance = uiTileDistance;
    ciAttackTarget.m_uiHateLevel += i_uiHateLevel;
    ciAttackTarget.m_bInAggroRange = ( uiTileDistance <= GetMaxAggroRange() );
    ciAttackTarget.m_bInLeashRange = ( fOriginDistance <= s_fMaxAttackLeashRange );
    ciAttackTarget.m_bInLOS = ( bAdjacent || ciMap.TestLOS( ciPosition, ciTargetPosition ) );
    ciAttackTarget.m_bInPrimaryRange = ( uiTileDistance <= m_uiPrimaryWeaponRange );
    ciAttackTarget.m_bInSpecialRange = ( uiTileDistance <= m_uiSpecialWeaponRange );
  }


  function AIAbandonPath()
  {
    if( HasPath() )
    {
      //if( g_uiLastAttackedID == GetID() )
      //print( format( "## %s[%x] abandoned path\n", GetName(), GetID() ) );
      ForgetPath();
    }
  }


  function AIAttack( i_ciAttackTarget )
  {
    local ciMap = GetMap();
    if( ( null == ciMap ) || ( null == i_ciAttackTarget ) )
    {
      return ActionDelay.Short;
    }

    local ciTarget = ::rumFetchPawn( i_ciAttackTarget.m_uiTargetID );
    if( null == ciTarget )
    {
      return ActionDelay.Short;
    }

    local ciPosition = GetPosition();
    local ciTargetPosition = i_ciAttackTarget.m_ciLastPosition;

    // Determine NPC attack based on distance to target
    if( i_ciAttackTarget.m_bInLOS && HasSpellReady() && SkillRoll( GetIntelligence() ) )
    {
      if( AICastSpell( ciTarget, i_ciAttackTarget.m_uiTileDistance ) )
      {
        OnSpellCast();
        return ActionDelay.Short;
      }
    }

    local eDelay = ActionDelay.Short;

    if( i_ciAttackTarget.m_uiTileDistance <= 1 )
    {
      //if( g_uiLastAttackedID == GetID() )
      //print( format( "## %s[%x] within melee range of %x\n", GetName(), GetID(), i_ciAttackTarget.m_uiTargetID ) );

      local bTargetIsPlayer = ( ciTarget instanceof Player );
      local bGuard = GetProperty( Creature_Guard_PropertyID, false );
      if( bGuard && bTargetIsPlayer && ciTarget.IsCriminal() &&
          ( ciTarget.GetHitpoints() < 200 || ciTarget.IsIncapacitated() ) )
      {
        ciTarget.Jail();
        return eDelay;
      }

      if( m_bCamouflages && IsCamouflaged() )
      {
        // Come out of hiding
        SetProperty( Camouflaged_PropertyID, false );
        SetProperty( Last_Camouflage_Time_PropertyID, g_ciServer.m_fServerTime );
      }

      local bStealsFood = GetProperty( Creature_Steals_Food_PropertyID, false );
      local bStealsGold = GetProperty( Creature_Steals_Gold_PropertyID, false );

      // There is a 5% chance that a creature that steals will attempt to do so from an adjacent target
      if( ( bStealsFood || bStealsGold ) && bTargetIsPlayer && rand() % 100 < 5 )
      {
        if( bStealsGold && ciTarget.GetVersionedProperty( g_eGoldPropertyVersionArray ) > 0 )
        {
          if( !ciTarget.SkillRoll( ciTarget.GetDexterity() ) )
          {
            // Deduct small amount of gold
            local iAmount = rand() % 32 + 1;
            ciTarget.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iAmount );
            ciTarget.ActionWarning( msg_gold_pilfered_client_StringID );
          }
        }
        else if( bStealsFood && ciTarget.GetVersionedProperty( g_eFoodPropertyVersionArray ) > 0 )
        {
          if( !ciTarget.SkillRoll( ciTarget.GetDexterity() ) )
          {
            // Deduct small amount of food
            local iAmount = rand() % 16 + 1;
            ciTarget.AdjustVersionedProperty( g_eFoodPropertyVersionArray, -iAmount );
            ciTarget.ActionWarning( msg_food_pilfered_client_StringID );
          }
        }
      }
      else
      {
        local eWeaponType = ChooseWeapon( i_ciAttackTarget.m_uiTileDistance );
        Attack( ciTarget, eWeaponType );
      }
    }
    else if( !m_bCamouflages || !IsCamouflaged() )
    {
      // The enemy isn't adjacent, so determine if the NPC should attack from range or move closer
      local eWeaponType = ChooseWeapon( i_ciAttackTarget.m_uiTileDistance );
      if( ( rumInvalidAssetID == eWeaponType ) || !ciTarget.SkillRoll( ciTarget.GetIntelligence() ) )
      {
        //if( g_uiLastAttackedID == GetID() )
        //print( format( "## %s[%x] wants to get closer to %x\n", GetName(), GetID(), i_ciAttackTarget.m_uiTargetID ) );

        // Attempt to move closer, but only if the NPC is capable of moving and the move falls within
        // the leash range of the origin position
        if( GetMoveType() != MoveType.Stationary )
        {
          if( i_ciAttackTarget.m_bInLeashRange )
          {
            local ciPathTargetPos = GetPathTargetPosition();
            if( ciPathTargetPos != null )
            {
              //if( g_uiLastAttackedID == GetID() )
              //print( format( "## %s[%x] goal pos: %d %d\n", GetName(), GetID(), ciPathTargetPos.x, ciPathTargetPos.y ) );
            }

            if( HasPath() )
            {
              if( ciPathTargetPos.Equals( ciTargetPosition ) )
              {
                //if( g_uiLastAttackedID == GetID() )
                //print( format( "## %s[%x] following path to target %x\n", GetName(), GetID(), i_ciAttackTarget.m_uiTargetID ) );
                eDelay = AIFollowPath();
              }
              else
              {
                //if( g_uiLastAttackedID == GetID() )
                //print( format( "## %s[%x] abandoning stale path to target %x\n", GetName(), GetID(), i_ciAttackTarget.m_uiTargetID ) );
                AIAbandonPath();
              }
            }
            else
            {
              // Move closer to target
              local ciVector = ciMap.GetDirectionVector( ciPosition, i_ciAttackTarget.m_ciLastPosition );
              local ciNewPosition = ciPosition + ciVector;
              local eDir = GetDirectionFromVector( ciVector );

              //print( format( "## %s[%x] Attempting to move closer to %x\n", GetName(), GetID(), i_ciAttackTarget.m_uiTargetID ) );
              if( IsPositionHarmful( ciNewPosition ) || AIMove( eDir ) != rumSuccessMoveResultType )
              {
                // Plot a path around the harmful obstacle
                //print( format( "## %s[%x] harmful or failed move to %x, must path\n", GetName(), GetID(), i_ciAttackTarget.m_uiTargetID ) );
                local bFoundPath = AIFindPath( i_ciAttackTarget.m_ciLastPosition, i_ciAttackTarget.m_uiTargetID,
                                               s_iMaxPathRangeForAttacking );
                if( !bFoundPath )
                {
                  // Try a random direction - recursive call
                  local eRandomDir = GetRandomDirection();
                  if( eRandomDir != eDir && AIMove( eRandomDir) == rumSuccessMoveResultType )
                  {
                    eDelay = AIDetermineMoveDelay( ciNewPosition );
                  }
                }
                else
                {
                  eDelay = ActionDelay.Long;
                }
              }
              else if( !ciNewPosition.Equals( ciPosition ) )
              {
                eDelay = AIDetermineMoveDelay( ciNewPosition );
              }
            }
          }
        }
      }
      else
      {
        // Range attack
        local ciWeapon = ::rumGetInventoryAsset( eWeaponType );
        local uiWeaponRange =
          ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Attack_Range_PropertyID, 1 ) : 1;
        if( uiWeaponRange >= i_ciAttackTarget.m_uiTileDistance )
        {
          //if( g_uiLastAttackedID == GetID() )
          //print( "## " + GetName() + " range-attacking " + ciTarget.GetName() + "\n" );

          // The creature wants to attack and it is within range
          Attack( ciTarget, eWeaponType );
        }
      }
    }

    return eDelay;
  }


  function AICastSpell( i_ciTarget, i_uiTileDistanceToTarget )
  {
    local eSpellType = GetSpell();

    local ciSpell = ::rumGetCustomAsset( eSpellType );
    if( null == ciSpell )
    {
      // Something went wrong - disable spells for this npc
      m_bSpellReady = false;
      return false;
    }

    local uiRange = ciSpell.GetProperty( Spell_Range_PropertyID, 1 );
    if( i_uiTileDistanceToTarget <= uiRange )
    {
      CastSpell( eSpellType, Direction.None, null, i_ciTarget );
      return true;
    }

    return false;
  }


  function AIChangeHeading( i_eDir )
  {
    local iClockwiseTurns = 0;
    local iCounterClockwiseTurns = 0;

    if( m_eHeading > i_eDir )
    {
      iClockwiseTurns = ( i_eDir + 8 ) - m_eHeading;
      iCounterClockwiseTurns = m_eHeading - i_eDir;
    }
    else
    {
      iClockwiseTurns = i_eDir - m_eHeading;
      iCounterClockwiseTurns = m_eHeading - ( i_eDir - 8 );
    }

    if( iClockwiseTurns < iCounterClockwiseTurns )
    {
      local eNewHeading;

      // Turn clockwise
      for( local i = 0; i < iClockwiseTurns; ++i )
      {
        ++m_eHeading;
        if( m_eHeading > Direction.Southwest )
        {
          m_eHeading = Direction.West;
        }

        AIUpdateState();
      }
    }
    else
    {
      // Turn counter-clockwise
      for( local i = 0; i < iCounterClockwiseTurns; ++i )
      {
        --m_eHeading;
        if( m_eHeading < Direction.West )
        {
          m_eHeading = Direction.Southwest;
        }

        AIUpdateState();
      }
    }
  }


  function AIDetermine( i_iActionIndex )
  {
    // 1. If the NPC has a target, verify that the target is still valid (remove the target if not)
    // 2. Select a new target from a list of current targets
    // 3. Attempt to find a target

    if( i_iActionIndex != m_iActionIndex )
    {
      //if( g_uiLastAttackedID == GetID() )
      //print( format( "#### %s[%x] mismatched Action index!\n", GetName(), GetID() ) );
      return;
    }

    // Identify all ignored targets that have expired
    local uiTargetIDRemovalArray = [];
    foreach( uiTargetID, fLastPathTime in m_ciIgnoreTargetIDTable )
    {
      if( g_ciServer.m_fServerTime - fLastPathTime > 3.0 )
      {
        uiTargetIDRemovalArray.append( uiTargetID );
      }
    }

    // Remove all ignored targets that have expired
    foreach( uiTargetID in uiTargetIDRemovalArray )
    {
      //print( format( "## %s[%x] no longer ignoring %x\n", GetName(), GetID(), uiTargetID ) );
      delete m_ciIgnoreTargetIDTable[uiTargetID];
    }

    local eDelay = ActionDelay.Short;

    if( !( IsDead() || IsIncapacitated() ) )
    {
      if( m_ciOriginPos.Equals( GetPosition() ) )
      {
        m_iRehomeAttempts = 0;
      }

      // Hate degrades over time
      AIReduceHate();

      // Find new targets
      AIDiscoverTargets();

      // Consider already identified targets
      local ciAttackTarget = AISelectTarget();

      if( IsFleeing() )
      {
        //if( g_uiLastAttackedID == GetID() )
        //print( format( "## %s[%x] wants to flee!\n", GetName(), GetID() ) );
        eDelay = AIFlee( ciAttackTarget );
      }
      else if( ciAttackTarget != null )
      {
        //if( g_uiLastAttackedID == GetID() )
        //print( format( "## %s[%x] wants to attack target %x pos %d %d!\n", GetName(), GetID(), ciAttackTarget.m_uiTargetID, ciAttackTarget.m_ciLastPosition.x, ciAttackTarget.m_ciLastPosition.y ) );
        eDelay = AIAttack( ciAttackTarget );
      }
      else if( HasPath() )
      {
        //if( g_uiLastAttackedID == GetID() )
        //print( format( "## %s[%x] wants to follow a path!\n", GetName(), GetID() ) );
        eDelay = AIFollowPath();
      }
      else if( !IsTalking() )
      {
        // Remove this? Could be tracked on individual attack targets
        if( NeedsLeash() )
        {
          //if( g_uiLastAttackedID == GetID() )
          //print( format( "## %s[%x] needs to leash!\n", GetName(), GetID() ) );
          eDelay = AIPathHome();
        }
        else if( ( PostureType.Wandering == m_eDefaultPosture ) || GetProperty( Creature_Bribed_PropertyID, false ) )
        {
          //if( g_uiLastAttackedID == GetID() )
          //print( format( "## %s[%x] wants to wander!\n", GetName(), GetID() ) );
          eDelay = AIWander();
        }

        if( m_bCamouflages && !IsCamouflaged() )
        {
          local fLastCamoTime = GetProperty( Last_Camouflage_Time_PropertyID, 0.0 );
          if( g_ciServer.m_fServerTime - fLastCamoTime >= m_fCamoCooldownTime )
          {
            // Hide
            SetProperty( Camouflaged_PropertyID, true );
          }
        }
      }
    }

    // Check again in a little while
    local fDelay = GetActionDelay( eDelay ) + frandVariance( m_fTimeDecideVariance ) + 0.5;
    ::rumSchedule( this, AIDetermine, fDelay, ++m_iActionIndex );
  }


  function AIDetermineMoveDelay( i_ciPos )
  {
    local eDelay = ActionDelay.Short;

    local ciMap = GetMap();

    // Cost based on base tile type
    local ciPosData = ciMap.GetPositionData( i_ciPos );
    local ciTile = ::rumGetTileAsset( ciPosData.GetTileID() );
    local fWeight = ciTile.GetWeight();
    if( fWeight >= 2.0 )
    {
      eDelay = ActionDelay.Long;
    }
    else if( fWeight >= 1.0 )
    {
      eDelay = ActionDelay.Medium;
    }

    return eDelay;
  }


  function AIDetermineWindDelay()
  {
    // Cost based on wind direction
    local iMinDistance = 0;

    local eWindDirection = g_ciServer.m_ciUltima4World.m_eWindDirection;
    if( m_eHeading != eWindDirection )
    {
      local iClockwiseTurns = 0;
      local iCounterClockwiseTurns = 0;
      if( m_eHeading > eWindDirection )
      {
        iClockwiseTurns = ( eWindDirection + 8 ) - m_eHeading;
        iCounterClockwiseTurns = m_eHeading - eWindDirection;
      }
      else
      {
        iClockwiseTurns = eWindDirection - m_eHeading;
        iCounterClockwiseTurns = m_eHeading - ( eWindDirection - 8 );
      }

      iMinDistance = min( iClockwiseTurns, iCounterClockwiseTurns );
    }

    local eDelay = ActionDelay.Short;

    if( iMinDistance > 2 )
    {
      // Sailing against the wind
      eDelay = ActionDelay.Long;
    }
    else if( iMinDistance > 0 )
    {
      // Sailing somewhat with or perpendicular to the wind
      eDelay = ActionDelay.Medium;
    }

    return eDelay;
  }


  function AIDiscoverTargets()
  {
    if( GetProperty( Creature_Guard_PropertyID, false ) )
    {
      AIFindTargetsForGuard();
    }
    else if( PostureType.Attack == m_eDefaultPosture )
    {
      AIFindTargetsForEvilCreatures();
    }
  }


  function AIFindPath( i_ciDestPos, i_uiTargetID, i_iMaxRange )
  {
    //if( g_uiLastAttackedID == GetID() )
    //print( format( "## %s[%x] finding path to %d %d\n", GetName(), GetID(), i_ciDestPos.x, i_ciDestPos.y ) );

    local bFoundPath = false;

    if( GetMoveType() != MoveType.Stationary )
    {
      // Set to pathing so that special collision considerations can be made, e.g. humanoids won't collide with doors
      if( ( i_uiTargetID == rumInvalidGameID ) || !m_ciIgnoreTargetIDTable.rawin( i_uiTargetID ) )
      {
        m_bPathing = true;

        bFoundPath = FindPath( i_ciDestPos, i_iMaxRange );
        if( !bFoundPath && i_uiTargetID != rumInvalidGameID )
        {
          //print( format( "## %s[%x] ignoring %x\n", GetName(), GetID(), i_uiTargetID ) );
          // Add the target to the ignore table
          m_ciIgnoreTargetIDTable[i_uiTargetID] <- g_ciServer.m_fServerTime;
        }

        m_bPathing = false;
      }
    }

    //if( g_uiLastAttackedID == GetID() )
    //print( format( "## %s[%x] found path: %s\n", GetName(), GetID(), ( bFoundPath ? "true" : "false" ) ) );

    return bFoundPath;
  }


  function AIFindTargetsForEvilCreatures()
  {
    // Find a player within attack range using line of sight
    local ciMap = GetMap();

    if( IsJinxed() )
    {
      local ciPawns = ciMap.GetPawns( GetPosition(), GetMaxAggroRange(), true );
      foreach( ciPawn in ciPawns )
      {
        if( IsTargetValid( ciPawn ) && ciPawn != this && ( ciPawn instanceof NPC ) )
        {
          AddAttackTarget( ciPawn, rand()%32 );
        }
      }
    }
    else
    {
      // Get all players within aggro range that the NPC can see
      local ciPlayerArray = ciMap.GetPlayers( GetPosition(), GetMaxAggroRange(), true );
      foreach( ciPlayer in ciPlayerArray )
      {
        if( IsTargetValid( ciPlayer ) )
        {
          AddAttackTarget( ciPlayer, rand()%32 );
        }
      }
    }
  }


  function AIFindTargetsForGuard()
  {
    // Find a player or evil creature within attack range using line of sight
    local ciMap = GetMap();

    // Get all pawns within aggro range that the guard can see
    local ciPawnArray = ciMap.GetPawns( GetPosition(), GetMaxAggroRange(), true );
    foreach( ciPawn in ciPawnArray )
    {
      // Only target evil creatures or player criminals who are not in jail
      if( !( ciPawn instanceof Creature ) || !IsTargetValid( ciPawn ) )
      {
        continue;
      }

      if( ( ciPawn instanceof Player ) && ciPawn.IsCriminal() && !ciPawn.IsJailed() )
      {
        AddAttackTarget( ciPawn, rand()%64 );
      }
      else if( GetAlignment() == AlignmentType.Evil )
      {
        AddAttackTarget( ciPawn, rand()%32 );
      }
    }
  }


  function AIFlee( i_ciAttackTarget )
  {
    // NOTE: This should only be called from AIDetermine()

    if( null == i_ciAttackTarget )
    {
      // Stop fleeing
      m_uiFleeTargetID = rumInvalidGameID;
      return ActionDelay.Short;
    }

    // Current direction to the target
    local ciMap = GetMap();
    local ciDir = ciMap.GetDirectionVector( GetPosition(), i_ciAttackTarget.m_ciLastPosition );
    local eDir = GetDirectionFromVector( ciDir );

    // Flip the direction
    eDir = ( eDir + 4 ) % 8;

    // Possibly add some variance - 20% chance
    local iRoll = rand() % 100;
    if( iRoll < 10 )
    {
      eDir = ( eDir + 1 ) % 8;
    }
    else if( iRoll < 20 )
    {
      eDir = ( eDir - 1 ) % 8;
    }

    local bMoved = false;

    // Don't check for harmful tiles or widgets while fleeing!
    if( AIMove( eDir ) != rumSuccessMoveResultType )
    {
      // Try a random direction
      local eRandomDir = GetRandomDirection();
      if( eRandomDir != eDir && ( AIMove( eRandomDir ) == rumSuccessMoveResultType ) )
      {
        bMoved = true;
      }
    }
    else
    {
      bMoved = true;
    }

    if( bMoved )
    {
      return AIDetermineMoveDelay( GetPosition() );
    }

    return ActionDelay.Short;
  }


  function AIFollowPath()
  {
    local ciPos = GetPathPosition();

    local ciMap = GetMap();
    if( IsShip() )
    {
      // Does the AI need to rotate in order to move in the right direction?
      local ciCurrentPos = GetPosition();
      local ciVector = ciMap.GetDirectionVector( ciCurrentPos, ciPos );
      local eDir = GetDirectionFromVector( ciVector );
      if( m_eHeading != eDir )
      {
        AIChangeHeading( eDir );
        return ActionDelay.Short;
      }
    }

    local eDelay = ActionDelay.Short;
    local eResult = rumFailMoveResultType;

    if( IsPositionHarmful( ciPos ) )
    {
      //if( g_uiLastAttackedID == GetID() )
      //print( format( "## %s[%x] abandoning harmful path\n", GetName(), GetID() ) );

      // Abandon the path because it is dangerous
      AIAbandonPath();
    }
    else
    {
      eResult = ciMap.MovePawn( this, ciPos );
      if( rumSuccessMoveResultType != eResult )
      {
        //if( g_uiLastAttackedID == GetID() )
        //print( format( "## %s[%x] encountered a path obstacle\n", GetName(), GetID() ) );

        // Is there a door in the way?
        local ciPosData = ciMap.GetPositionData( ciPos );
        local ciWidget = null;
        while( ciWidget = ciPosData.GetNext( rumWidgetPawnType ) )
        {
          if( ciWidget.GetAssetID() == U4_Door_WidgetID )
          {
            // Only humanoid creatures are capable of opening doors. Presumably, a door wouldn't be encountered along
            // the path, but there's always a small chance that the door was open when the path was determined.
            local bHumanoid = GetProperty( Creature_Humanoid_PropertyID, false );
            if( bHumanoid )
            {
              //if( g_uiLastAttackedID == GetID() )
              //print( format( "## %s[%x] opening door\n", GetName(), GetID() ) );

              ciWidget.Open( this );
              return eDelay;
            }
          }
        }
      }

      if( rumSuccessMoveResultType != eResult )
      {
        //if( g_uiLastAttackedID == GetID() )
        //print( format( "## %s[%x] has path fully blocked by obstacle\n", GetName(), GetID() ) );

        // Abandon the path because it is blocked
        AIAbandonPath();
      }
      else
      {
        // Determine the movement cost
        if( IsShip() )
        {
          eDelay = AIDetermineWindDelay();
        }
        else
        {
          eDelay = AIDetermineMoveDelay( ciPos );
        }
      }
    }

    if( rumSuccessMoveResultType == eResult )
    {
      // We are done with the current path position, so move to the next
      PopPathPosition();
    }

    return eDelay;
  }


  function AIMove( i_eDir )
  {
    if( IsShip() )
    {
      // Does the AI need to rotate in order to move in the right direction?
      if( m_eHeading != i_eDir )
      {
        AIChangeHeading( i_eDir );
        return rumSuccessMoveResultType;
      }
    }

    local ciMap = GetMap();
    local ciPos = GetPosition() + GetDirectionVector( i_eDir );
    return ciMap.MovePawn( this, ciPos );
  }


  function AINudge()
  {
    if( PostureType.Wandering != m_eDefaultPosture || IsFleeing() || HasAttackTarget() || IsTalking() )
    {
      // Can't be nudged
      return;
    }

    local ciMap = GetMap();
    if( null == ciMap )
    {
      return;
    }

    // Get a random position adjacent to the current position
    local eDir = GetRandomDirection();
    local ciVector = GetDirectionVector( eDir );
    local ciPos = GetPosition() + ciVector;

    if( ciMap.IsPositionWithinBounds( ciPos ) &&
        ciMap.IsPositionWithinRadius( ciPos, m_ciOriginPos, s_fMaxWanderLeashRange ) &&
        !IsPositionHarmful( ciPos ) )
    {
      {
        AIMove( eDir );
      }
    }
  }


  function AIPathHome()
  {
    // NOTE: This should only be called from AIDetermine()

    AIAbandonPath();

    local ciMap = GetMap();
    if( null == ciMap )
    {
      return ActionDelay.Short;
    }

    local eDelay = ActionDelay.Short;

    // If the NPC has line of sight to its origin position, just move toward it
    local ciPos = GetPosition();
    if( !ciMap.TestLOS( ciPos, m_ciOriginPos ) )
    {
      //if( g_uiLastAttackedID == GetID() )
      //print( format( "## AIPathHome: %s[%x] can't see origin position, finding path\n", GetName(), GetID() ) );

      // Plot a path to the NPC's origin position
      if( !AIFindPath( m_ciOriginPos, rumInvalidGameID, s_iMaxPathRangeForLeashing ) )
      {
        //if( g_uiLastAttackedID == GetID() )
        //print( format( "## AIPathHome: %s[%x] failed to find a path home\n", GetName(), GetID() ) );
        // No obvious path home

        if( m_iRehomeAttempts < s_iMaxRehomeAttempts )
        {
          // Force an attempt to move close
          local ciVector = ciMap.GetDirectionVector( ciPos, m_ciOriginPos );
          local eDir = GetDirectionFromVector( ciVector );
          local ciNewPos = ciPos + ciVector;
          if( !IsPositionHarmful( ciNewPos ) )
          {
            if( AIMove( eDir ) != rumSuccessMoveResultType )
            {
              ++m_iRehomeAttempts;
            }
          }
        }
        else
        {
          //print( format( "## AIPathHome: %s[%x] Home not in LOS, m_iRehomeAttempts: %i\n", GetName(), GetID(), m_iRehomeAttempts ) );
          Rehome();
        }
      }
      else
      {
        //if( g_uiLastAttackedID == GetID() )
        //print( format( "## AIPathHome: %s[%x] found a path home\n", GetName(), GetID() ) );
      }

      eDelay = ActionDelay.Long;
    }
    else
    {
      local ciVector = ciMap.GetDirectionVector( ciPos, m_ciOriginPos );
      local eDir = GetDirectionFromVector( ciVector );
      local ciNewPos = ciPos + ciVector;

      if( IsPositionHarmful( ciNewPos ) || AIMove( eDir ) != rumSuccessMoveResultType )
      {
        //print( format( "## AIPathHome: %s[%x] finding path home\n", GetName(), GetID() ) );
        if( !AIFindPath( m_ciOriginPos, rumInvalidGameID, s_iMaxPathRangeForLeashing ) )
        {
          // No obvious path home
          if( ++m_iRehomeAttempts >= s_iMaxRehomeAttempts )
          {
            //print( format( "## AIPathHome: %s[%x] Home in LOS, m_iRehomeAttempts: %i\n", GetName(), GetID(), m_iRehomeAttempts ) );
            Rehome();
          }
        }

        eDelay = ActionDelay.Long;
      }
      else if( !ciNewPos.Equals( ciPos ) )
      {
        eDelay = AIDetermineMoveDelay( ciNewPos );
      }
    }

    return eDelay;
  }


  function AIReduceHate()
  {
    local ciAttackTargetArray = [];

    foreach( ciAttackTarget in m_ciAttackTargetTable )
    {
      ciAttackTarget.m_uiHateLevel = max( ciAttackTarget.m_uiHateLevel - 1, 0 );

      if( ciAttackTarget.m_uiHateLevel <= 0 )
      {
        // Add to the array for removal
        ciAttackTargetArray.append( ciAttackTarget.m_uiTargetID );
      }
    }

    // Remove attack targets
    foreach( uiTargetID in ciAttackTargetArray )
    {
      delete m_ciAttackTargetTable[uiTargetID];
    }
  }


  function AISelectTarget()
  {
    local ciMap = GetMap();
    if( null == ciMap )
    {
      return null;
    }

    local ciAttackTargetArray = [];
    ciAttackTargetArray.resize( m_ciAttackTargetTable.len() );

    // Fill the array
    local uiIndex = 0;
    foreach( ciAttackTarget in m_ciAttackTargetTable )
    {
      ciAttackTargetArray[uiIndex++] = ciAttackTarget;
    }

    // Sort the array by hate
    ciAttackTargetArray.sort( AISelectTargetCompare );

    local ciPos = GetPosition();
    local ciSelectedAttackTarget = null;

    // Visit each attack target to verify that it's valid
    foreach( uiIndex, ciAttackTarget in ciAttackTargetArray )
    {
      // Is the current target still valid?
      local ciTarget = ::rumFetchPawn( ciAttackTarget.m_uiTargetID );
      if( ciTarget != null && IsTargetValid( ciTarget ) )
      {
        local ciTargetMap = ciTarget.GetMap();
        if( ciMap == ciTargetMap )
        {
          local ciTargetPosition = ciTarget.GetPosition();
          local fOriginDistance = ciMap.GetDistance( m_ciOriginPos, ciTargetPosition );
          local uiTileDistance = ciMap.GetTileDistance( ciPos, ciTargetPosition );
          local bAdjacent = ( uiTileDistance <= 1 );

          // Update range info
          ciAttackTarget.m_uiTileDistance = uiTileDistance;
          ciAttackTarget.m_bInAggroRange = ( uiTileDistance <= GetMaxAggroRange() );
          ciAttackTarget.m_bInLeashRange = ( fOriginDistance <= s_fMaxAttackLeashRange );
          ciAttackTarget.m_bInPrimaryRange = ( uiTileDistance <= m_uiPrimaryWeaponRange );
          ciAttackTarget.m_bInSpecialRange = ( uiTileDistance <= m_uiSpecialWeaponRange );

          // Within attack range?
          if( null == ciSelectedAttackTarget &&
              ( bAdjacent || ( ciAttackTarget.m_bInAggroRange && ciAttackTarget.m_bInLeashRange ) ) )
          {
            // Within LOS?
            ciAttackTarget.m_bInLOS = bAdjacent || ciMap.TestLOS( ciPos, ciTargetPosition );
            if( ciAttackTarget.m_bInLOS )
            {
              //if( g_uiLastAttackedID == GetID() )
              //print( format( "## %s[%x] last target pos set to: %d %d\n", GetName(), GetID(), ciTargetPosition.x, ciTargetPosition.y ) );

              // Update the position of this target in the attack table
              ciAttackTarget.m_ciLastPosition = ciTargetPosition;

              if( !m_ciIgnoreTargetIDTable.rawin( ciAttackTarget.m_uiTargetID ) ||
                  ( ciAttackTarget.m_bInPrimaryRange || ciAttackTarget.m_bInSpecialRange ) )
              {
                // Attack path is unobstructed?
                local bHasClearPath = bAdjacent || ciMap.HasClearPath( ciPos, ciTargetPosition, MoveType.Ballistic);
                if( bHasClearPath )
                {
                  m_iRehomeAttempts = 0;
                  ciSelectedAttackTarget = ciAttackTarget;
                }
              }
            }
            else if( !ciPos.Equals( ciAttackTarget.m_ciLastPosition ) )
            {
              //if( g_uiLastAttackedID == GetID() )
              //{
                //print( format( "## %s[%x] lost sight of target %x\n", GetName(), GetID(), ciAttackTarget.m_uiTargetID ) );
                //print( format( "## Attacker pos: %d %d Target pos: %d %d", ciPos.x, ciPos.y, ciAttackTarget.m_ciLastPosition.x, ciAttackTarget.m_ciLastPosition.y ) );
              //}

              // Lost sight of this target, but pursuit can still happen to the last known position if there are no
              // other viable targets
              if( !HasPath() && ( ciAttackTargetArray.len() <= 1 || ( uiIndex + 1 >= ciAttackTargetArray.len() ) ) )
              {
                //if( g_uiLastAttackedID == GetID() )
                //print( format( "## %s[%x] pursuing target %x at last known position!\n", GetName(), GetID(), ciAttackTarget.m_uiTargetID ) );

                if( null == ciSelectedAttackTarget )
                {
                  m_iRehomeAttempts = 0;
                  ciSelectedAttackTarget = ciAttackTarget;
                }
              }
            }
            else if( 0 == ciAttackTarget.m_uiHateLevel )
            {
              // Target is on a different map, delete only if hate is low
              //if( g_uiLastAttackedID == GetID() )
              //print( format( "## %s[%x] target was able to hide: %x\n", GetName(), GetID(), ciAttackTarget.m_uiTargetID ) );
              delete m_ciAttackTargetTable[ciAttackTarget.m_uiTargetID];
            }
          }
        }
      }
    }

    return ciSelectedAttackTarget;
  }


  function AISelectTargetCompare( i_ciTarget1, i_ciTarget2 )
  {
    if( i_ciTarget1.m_uiHateLevel > i_ciTarget2.m_uiHateLevel )
    {
      return 1;
    }
    else if( i_ciTarget1.m_uiHateLevel < i_ciTarget2.m_uiHateLevel )
    {
      return -1
    }

    return 0;
  }


  function AIUpdateState()
  {
    // State_PropertyID drives client animation
    if( Direction.West == m_eHeading )
    {
      SetProperty( State_PropertyID, ShipStateType.FacingWest );
    }
    else if( Direction.North == m_eHeading )
    {
      SetProperty( State_PropertyID, ShipStateType.FacingNorth );
    }
    else if( Direction.East == m_eHeading )
    {
      SetProperty( State_PropertyID, ShipStateType.FacingEast );
    }
    else if( Direction.South == m_eHeading )
    {
      SetProperty( State_PropertyID, ShipStateType.FacingSouth );
    }
  }


  function AIWander()
  {
    // NOTE: This should only be called from AIDetermine()

    local eDelay = ActionDelay.Short;

    // 25% chance of moving in a random direction
    if( rand() % 256 < 64 )
    {
      local ciMap = GetMap();
      if( ciMap != null )
      {
        // Get a random position adjacent to the current position
        local eDir = GetRandomDirection();
        local ciVector = GetDirectionVector( eDir );
        local ciPos = GetPosition() + ciVector;

        if( ciMap.IsPositionWithinBounds( ciPos ) &&
            ciMap.IsPositionWithinRadius( ciPos, m_ciOriginPos, s_fMaxWanderLeashRange ) &&
            !IsPositionHarmful( ciPos ) )
        {
          if( AIMove( eDir ) == rumSuccessMoveResultType )
          {
            eDelay = AIDetermineMoveDelay( ciPos );
          }
        }
      }
    }

    return eDelay;
  }


  function Burn()
  {
    if( base.Burn() )
    {
      InterruptInteractions();
      return true;
    }

    return false;
  }


  function CallGuards( i_ciCriminal )
  {
    if( ( null == i_ciCriminal ) || !( i_ciCriminal instanceof Player ) )
    {
      return;
    }

    local ciMap = GetMap();
    local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
    if( ( MapType.Village == eMapType ) ||
        ( MapType.Towne == eMapType )   ||
        ( MapType.Keep == eMapType )    ||
        ( MapType.Castle == eMapType ) )
    {
      local iGuardRadius = 5;
      local iHateLevel = 100;
      local ciPawnArray = ciMap.GetAllPawns();
      foreach( ciPawn in ciPawnArray )
      {
        if( ( ciPawn instanceof NPC ) && ciPawn.IsVisible() && !ciPawn.IsIncapacitated() && ciPawn.IsGuard() )
        {
          if( ciMap.IsPositionWithinTileDistance( GetPosition(), ciPawn.GetPosition(), iGuardRadius ) )
          {
            ciPawn.AddAttackTarget( i_ciCriminal, iHateLevel );
          }
        }
      }
    }
  }


  function CheckAnswer( i_ciPlayer, i_strKeyword, i_strAnswer )
  {
    local strDesc = ::rumGetStringByName( "u4_dlg_" + m_iDialogueID + "_" + i_strKeyword + "_server_StringID" );
    if( null == strDesc )
    {
      return;
    }

    // TODO - localize answer!
    local strAnswer = i_strAnswer.tolower();
    if( "yes" == strAnswer || ( strAnswer.slice( 0, 1 ) == "y" ) )
    {
      strDesc = strDesc.slice( strDesc.find( "<y>" ), strDesc.find( "<n>" ) );
      if( strDesc.find( "<@lose_humility>" ) )
      {
        i_ciPlayer.AffectVirtue( VirtueType.Humility, -1, true, true );
      }
      else if( m_iDialogueID == 0 )
      {
        local iIndex = strDesc.find( "<@ultima" );
        if( iIndex >= 0 )
        {
          TalkEnd( i_ciPlayer, DialogueTerminationType.Standard );

          local strUltima = strDesc.slice( iIndex + 8, iIndex + 9 );

          local eVersion = strUltima.tointeger();
          eVersion = clamp( eVersion, GameType.Ultima1, GameType.Ultima4 );

          i_ciPlayer.ChangeWorld( eVersion );
        }
      }
    }
    else
    {
      strDesc = strDesc.slice( strDesc.find( "<n>" ) );
      if( strDesc.find( "<@gain_humility>" ) )
      {
        i_ciPlayer.AffectVirtue( VirtueType.Humility, 1, false, true );
      }
    }
  }


  function ChooseWeapon( i_uiTileDistanceToTarget )
  {
    local eChosenWeaponType = rumInvalidAssetID;
    local bPrimaryInRange = false;

    local eWeaponType = GetWeaponType();
    local ciWeapon = ::rumGetInventoryAsset( eWeaponType );
    local uiRange = ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Attack_Range_PropertyID, 1 ) : 1;
    if( uiRange >= i_uiTileDistanceToTarget )
    {
      eChosenWeaponType = eWeaponType;
      bPrimaryInRange = true;
    }

    if( m_bSpecialWeaponReady )
    {
      local eWeaponAltType = GetSpecialWeaponType();
      local ciWeaponAlt = ::rumGetInventoryAsset( eWeaponAltType );
      uiRange = ciWeaponAlt != null ? ciWeaponAlt.GetProperty( Inventory_Weapon_Attack_Range_PropertyID, 1 ) : 1;
      if( uiRange >= i_uiTileDistanceToTarget && ( !bPrimaryInRange || SkillRoll( GetIntelligence() ) ) )
      {
        // The currently selected special weapon is in range and the NPC wants to use it
        eChosenWeaponType = eWeaponAltType;
      }
    }

    return eChosenWeaponType;
  }


  function ConsumeMana( i_ciSpell )
  {
    return true;
  }


  function ConsumeSpell( i_ciSpell )
  {
    return true;
  }


  function Damage( i_iAmount, i_ciSource, i_eWeaponType = rumInvalidAssetID, i_bSendClientEffect = true )
  {
    //if( i_ciSource != null && ( i_ciSource instanceof Player ) )
    //{
    //  g_uiLastAttackedID = GetID();
    //}

    local bDamaged = base.Damage( i_iAmount, i_ciSource, i_eWeaponType );
    if( bDamaged && i_ciSource != null && ( i_ciSource instanceof Creature ) )
    {
      InterruptInteractions();

      AddAttackTarget( i_ciSource, i_iAmount );

      if( m_bCamouflages && IsCamouflaged() )
      {
        // Come out of hiding
        SetProperty( Camouflaged_PropertyID, false );
        SetProperty( Last_Camouflage_Time_PropertyID, g_ciServer.m_fServerTime );
      }

      local iHitpoints = GetProperty( Hitpoints_PropertyID, 1 );
      local iMaxHitpoints = GetMaxHitpoints();
      local fHealthPercent = iHitpoints / iMaxHitpoints.tofloat();
      local bNeverFlees = GetProperty( Creature_Never_Flees_PropertyID, false );
      if( fHealthPercent < 0.2 && GetMoveType() != MoveType.Stationary && !bNeverFlees )
      {
        m_uiFleeTargetID = i_ciSource.GetID();
        AIAbandonPath();
        ::rumSchedule( this, StopFleeing, s_fStopFleeTime );
      }

      CallGuards( i_ciSource );
    }

    return bDamaged;
  }


  function Electrify( io_ciImmunityTable, io_iRecursionDepth )
  {
    if( base.Electrify( io_ciImmunityTable, io_iRecursionDepth ) )
    {
      InterruptInteractions();
      return true;
    }

    return false;
  }


  function Freeze()
  {
    if( base.Freeze() )
    {
      InterruptInteractions();
      return true;
    }

    return false;
  }


  function GetArmourType()
  {
    return GetProperty( Creature_Armour_Type_PropertyID, rumInvalidAssetID );
  }


  function GetMaxAggroRange()
  {
    if( m_bCamouflages && IsCamouflaged() )
    {
      return 1;
    }

    return s_iMaxAggroRange;
  }


  function GetSpecialWeaponType()
  {
    local numWeapons = m_eSpecialWeaponArray.len();
    local iWeaponIndex = rand() % numWeapons;
    return m_eSpecialWeaponArray[iWeaponIndex];
  }


  function GetSpell()
  {
    return GetProperty( Creature_Spell_Type_PropertyID, rumInvalidAssetID );
  }


  function GetWeaponType()
  {
    return GetProperty( Creature_Weapon_Type_PropertyID, rumInvalidAssetID );
  }


  function HasAttackTarget()
  {
    return m_ciAttackTargetTable.len() != 0;
  }


  function HasSpellReady()
  {
    return m_bSpellReady;
  }


  function Incapacitate()
  {
    if( !( ResistsSleep() || IsUndead() ) && base.Incapacitate() )
    {
      InterruptInteractions();
      return true;
    }

    return false;
  }


  function InteractionBegin( i_ciPlayer )
  {
    if( i_ciPlayer != null )
    {
      local uiPlayerID = i_ciPlayer.GetID();
      m_uiInteractionTable[uiPlayerID] <- uiPlayerID;
      i_ciPlayer.m_uiInteractID = GetID();
    }
  }


  function InteractionEnd( i_ciPlayer )
  {
    if( i_ciPlayer != null )
    {
      if( GetID() == i_ciPlayer.m_uiInteractID )
      {
        i_ciPlayer.m_uiInteractID = rumInvalidGameID;
      }

      delete m_uiInteractionTable[i_ciPlayer.GetID()];
    }
  }


  function InterruptInteractions()
  {
    if( null == m_uiInteractionTable )
    {
      return;
    }

    local uiID = GetID();

    foreach( uiPlayerID in m_uiInteractionTable )
    {
      local ciPlayer = ::rumGetPlayer( uiPlayerID );
      if( ciPlayer != null )
      {
        if( uiID == ciPlayer.m_uiInteractID )
        {
          ciPlayer.m_uiInteractID = rumInvalidGameID;

          local ciBroadcast = ::rumCreate( Player_Talk_Interrupted_BroadcastID );
          ::rumSendPrivate( ciPlayer.GetSocket(), ciBroadcast );
        }

        delete m_uiInteractionTable[ciPlayer.GetID()];
      }
    }
  }


  function IsCamouflaged()
  {
    return GetProperty( Camouflaged_PropertyID, false );
  }


  function IsFleeing()
  {
    return m_uiFleeTargetID != rumInvalidGameID;
  }


  function IsGuard()
  {
    local eAssetID = GetAssetID();
    return ( U4_Guard_CreatureID == eAssetID ) ||
           ( U3_Guard_CreatureID == eAssetID ) ||
           ( U2_Guard_CreatureID == eAssetID ) ||
           ( U1_Guard_CreatureID == eAssetID );
  }


  function IsPathing()
  {
    return m_bPathing;
  }


  function IsShip()
  {
    return GetProperty( Creature_Is_Ship_PropertyID, false );
  }


  function IsTalking()
  {
    return m_uiInteractionTable != null && m_uiInteractionTable.len() != 0;
  }


  function IsTargetValid( i_ciTarget )
  {
    return( i_ciTarget != null && i_ciTarget.IsVisible() && !i_ciTarget.IsDead() );
  }


  function IsUndead()
  {
    return GetProperty( Creature_Undead_PropertyID, false );
  }


  function IsVenomous()
  {
    return GetProperty( Creature_Venomous_PropertyID, false );
  }


  function Jinx()
  {
    if( base.Jinx() )
    {
      InterruptInteractions();

      // Clear the entire attack table
      m_ciAttackTargetTable = null;
      return true;
    }

    return false;
  }


  function KeywordResponse( i_ciPlayer, i_strKeyword )
  {
    local strDesc = "u4_dlg_" + m_iDialogueID + "_";

    // Handle Lord British help requests
    if( ( NPCType.Special == m_eDefaultNpcType ) && ( NPCSpecialType.U4_LordBritish == m_iDialogueID ) )
    {
      local strKeywordHelp = ::rumGetStringByName( strDesc + "kw_help_server_StringID" );
      if( i_strKeyword == strKeywordHelp )
      {
        local eStage = i_ciPlayer.SetProperty( U4_Lord_British_Stage_PropertyID, 0 );
        local iLevel = i_ciPlayer.GetProperty( U4_Level_PropertyID, 1 );

        if( iLevel < 2 || LBHelpStageType.GeneralAdvice == eStage )
        {
          // Player has only met Lord British - this will be shown regardless of the LB stage until the
          // player is level at least experience level 2.
          i_strKeyword = format( "%s%d", i_strKeyword, LBHelpStageType.GeneralAdvice );

          if( LBHelpStageType.GeneralAdvice == eStage )
          {
            // Advance to the next eStage
            i_ciPlayer.SetProperty( U4_Lord_British_Stage_PropertyID, LBHelpStageType.SeekHelpFromOthers );
          }
        }
        else if( iLevel < 3 || ( LBHelpStageType.SeekHelpFromOthers == eStage ) )
        {
          // Player has received initial help from Lord British - this will be shown regardless of the LB
          // stage until the player is level at least experience level 3.
          i_strKeyword = format( "%s%d", i_strKeyword, LBHelpStageType.SeekHelpFromOthers );

          if( LBHelpStageType.SeekHelpFromOthers == eStage )
          {
            // Advance to the next eStage
            i_ciPlayer.SetProperty( U4_Lord_British_Stage_PropertyID, LBHelpStageType.ShrineRuneMantra );
          }
        }
        else if( LBHelpStageType.ShrineRuneMantra == eStage || !i_ciPlayer.HasAllRunes() )
        {
          // This will be shown regardless of LB stage until the player has all runes
          i_strKeyword = format( "%s%d", i_strKeyword, LBHelpStageType.ShrineRuneMantra );

          if( LBHelpStageType.ShrineRuneMantra == eStage )
          {
            // Advance to the next eStage
            i_ciPlayer.SetProperty( U4_Lord_British_Stage_PropertyID, LBHelpStageType.PartialAvatar );
          }
        }
        else if( ( LBHelpStageType.PartialAvatar == eStage ) || !i_ciPlayer.IsPartialAvatar() )
        {
          // This will be shown regardless of LB stage until the player reaches partial avatarhood
          i_strKeyword = format( "%s%d", i_strKeyword, LBHelpStageType.PartialAvatar );

          if( LBHelpStageType.PartialAvatar == eStage )
          {
            // Advance to the next eStage
            i_ciPlayer.SetProperty( U4_Lord_British_Stage_PropertyID, LBHelpStageType.DungeonStones );
          }
        }
        else if( ( LBHelpStageType.DungeonStones == eStage ) || !i_ciPlayer.HasAllStones() )
        {
          // This will be shown regardless of LB stage until the player has all dungeon stones
          i_strKeyword = format( "%s%d", i_strKeyword, LBHelpStageType.DungeonStones );

          if( LBHelpStageType.DungeonStones == eStage )
          {
            // Advance to the next eStage
            i_ciPlayer.SetProperty( U4_Lord_British_Stage_PropertyID, LBHelpStageType.EightPartAvatar );
          }
        }
        else if( ( LBHelpStageType.EightPartAvatar == eStage ) || !i_ciPlayer.IsEightPartsAvatar() )
        {
          // This will be shown regardless of LB stage until the player is a full Avatar
          i_strKeyword = format( "%s%d", i_strKeyword, LBHelpStageType.EightPartAvatar );

          if( LBHelpStageType.EightPartAvatar == eStage )
          {
            // Advance to the next eStage
            i_ciPlayer.SetProperty( U4_Lord_British_Stage_PropertyID, LBHelpStageType.BellBookCandle );
          }
        }
        else if( ( LBHelpStageType.BellBookCandle == eStage ) || !i_ciPlayer.HasBellBookAndCandle() )
        {
          // This will be shown regardless of LB stage until the player has an imbued Bell, Book, and Candle
          i_strKeyword = format( "%s%d", i_strKeyword, LBHelpStageType.BellBookCandle );

          if( LBHelpStageType.BellBookCandle == eStage )
          {
            // Advance to the next eStage
            i_ciPlayer.SetProperty( U4_Lord_British_Stage_PropertyID, LBHelpStageType.KeyAndWord );
          }
        }
        else if( ( LBHelpStageType.KeyAndWord == eStage ) || !i_ciPlayer.HasThreePartsKey() )
        {
          // This will be shown regardless of LB stage until the player has the Three Parts Key
          i_strKeyword = format( "%s%d", i_strKeyword, LBHelpStageType.KeyAndWord );

          if( LBHelpStageType.KeyAndWord == eStage )
          {
            // Advance to the next eStage
            i_ciPlayer.SetProperty( U4_Lord_British_Stage_PropertyID, LBHelpStageType.JourneyToAbyss );
          }
        }
        else
        {
          i_strKeyword = format( "%s%d", i_strKeyword, LBHelpStageType.JourneyToAbyss );
        }
      }
    }

    // Strip out any server side function calls
    local strToken = strDesc + i_strKeyword + "_server_StringID";
    local strResponse = StripTags( ::rumGetStringByName( strToken, i_ciPlayer.m_iLanguageID ), "<@", ">" );

    local ciBroadcast = ::rumCreate( Player_Talk_Keyword_BroadcastID, i_strKeyword, strResponse );
    ::rumSendPrivate( i_ciPlayer.GetSocket(), ciBroadcast );
  }


  function NeedsLeash()
  {
    local bLeash = false;

    if( m_ciOriginPos != null && !m_ciOriginPos.Equals( GetPosition() ) )
    {
      if( ( PostureType.Wandering == m_eDefaultPosture ) || ( GetProperty( Creature_Bribed_PropertyID, false ) ) )
      {
        // Wandering creatures just need to be within the wander leash radius
        local ciMap = GetMap();
        bLeash = !ciMap.IsPositionWithinRadius( GetPosition(), m_ciOriginPos, s_fMaxWanderLeashRange );
      }
      else
      {
        // Stationary creatures need to return to their origin position
        bLeash = true;
      }
    }

    return bLeash;
  }


  function OnAddedToMap()
  {
    if( m_eDefaultNpcType != NPCType.Standard || m_iDialogueID > 0 )
    {
      // The NPC is capable of interactions, so create the interaction table
      m_uiInteractionTable = {};
    }

    ::rumSchedule( this, AIDetermine, frandVariance( m_fTimeDecideVariance ), ++m_iActionIndex );

    // Store the NPCs original position for leashing, etc.
    m_ciOriginPos = GetPosition();
  }


  function OnCreated()
  {
    m_bCamouflages = IsCamouflaged();

    // Handle HP variation
    local uiMaxHP = GetMaxHitpoints();

    local uiVariance = (uiMaxHP * 0.1).tointeger()
    local uiNewMaxHP = uiMaxHP + uiVariance;
    local uiNewMinHP = uiMaxHP - uiVariance;

    // TODO - use min/max limits sets by Max_Hitpoints_PropertyID property
    local uiRandomMaxHP = rand()%( uiNewMaxHP - uiNewMinHP) + uiNewMinHP;
    local uiFinalMaxHP = clamp( uiRandomMaxHP, 1, 65535 );

    SetProperty( Max_Hitpoints_PropertyID, uiFinalMaxHP );
    SetProperty( Hitpoints_PropertyID, uiFinalMaxHP );

    local eSpellType = GetSpell();
    m_bSpellReady = ( eSpellType != rumInvalidAssetID );

    m_eSpecialWeaponArray = array( 0 );

    local eWeaponType = GetProperty( Creature_Special_Weapon_1_Type_PropertyID, rumInvalidAssetID );
    if( eWeaponType != rumInvalidAssetID )
    {
      m_eSpecialWeaponArray.append( eWeaponType );
    }

    eWeaponType = GetProperty( Creature_Special_Weapon_2_Type_PropertyID, rumInvalidAssetID );
    if( eWeaponType != rumInvalidAssetID )
    {
      m_eSpecialWeaponArray.append( eWeaponType );
    }

    eWeaponType = GetProperty( Creature_Special_Weapon_3_Type_PropertyID, rumInvalidAssetID );
    if( eWeaponType != rumInvalidAssetID )
    {
      m_eSpecialWeaponArray.append( eWeaponType );
    }

    eWeaponType = GetProperty( Creature_Special_Weapon_4_Type_PropertyID, rumInvalidAssetID );
    if( eWeaponType != rumInvalidAssetID )
    {
      m_eSpecialWeaponArray.append( eWeaponType );
    }

    // Determine the max weapon range of this creature
    eWeaponType = GetWeaponType();
    if( eWeaponType != rumInvalidAssetID )
    {
      local ciWeapon = ::rumGetInventoryAsset( eWeaponType );
      if( ciWeapon != null )
      {
        m_uiPrimaryWeaponRange = ciWeapon.GetProperty( Inventory_Weapon_Attack_Range_PropertyID, 1 );
      }
    }

    foreach( eSpecialType in m_eSpecialWeaponArray )
    {
      local ciWeapon = ::rumGetInventoryAsset( eSpecialType );
      if( ciWeapon != null )
      {
        local iRange = ciWeapon.GetProperty( Inventory_Weapon_Attack_Range_PropertyID, 1 );
        m_uiSpecialWeaponRange = max( m_uiSpecialWeaponRange, iRange );
      }
    }

    ReadySpecialWeapon();
  }


  function OnDeath( i_ciSource, i_bKillCredit = true )
  {
    base.OnDeath( i_ciSource, i_bKillCredit );

    InterruptInteractions();

    if( i_ciSource instanceof Player )
    {
      CallGuards( i_ciSource );
    }

    SetVisibility( false );

    local ciMap = GetMap();

    if( i_bKillCredit )
    {
      local eLootType = GetProperty( Creature_Loot_Type_PropertyID, rumInvalidAssetID );
      if( eLootType != rumInvalidAssetID )
      {
        // Check to see if the loot can be spawned where the player can typically walk
        local ciPos = GetPosition();
        local ciPosData = ciMap.GetPositionData( ciPos );
        local ciTile = ::rumGetTileAsset( ciPosData.GetTileID() );
        if( !ciTile.IsCollision( MoveType.Terrestrial ) )
        {
          local ciObject = ::rumCreate( eLootType );
          if( ciMap.AddPawn( ciObject, GetPosition() ) )
          {
            local fExpireInterval = ciObject.GetProperty( Expiration_Interval_PropertyID, 0.0 );
            ::rumSchedule( ciObject, ciObject.Expire, fExpireInterval );
          }
        }
      }
    }

    local eCarcassType = GetProperty( Creature_Carcass_Type_PropertyID, rumInvalidAssetID );
    if( eCarcassType != rumInvalidAssetID )
    {
      local ciObject = ::rumCreate( eCarcassType );
      if( ciMap.AddPawn( ciObject, GetPosition() ) )
      {
        local fExpireInterval = ciObject.GetProperty( Expiration_Interval_PropertyID, 0.0 );
        ::rumSchedule( ciObject, ciObject.Expire, fExpireInterval );
      }
    }

    if( m_bRespawns )
    {
      ScheduleRespawn();
    }
  }


  function OnJinxEnd()
  {
    base.OnJinxEnd();

    // Clear the entire attack table
    m_ciAttackTargetTable = null;
  }


  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    base.OnPropertyUpdated( i_ePropertyID, i_vValue );

    if( NPC_Dialogue_ID_PropertyID == i_ePropertyID )
    {
      m_iDialogueID = i_vValue;
    }
    else if( NPC_Posture_Type_PropertyID == i_ePropertyID )
    {
      m_eDefaultPosture = i_vValue;
    }
    else if( NPC_Type_PropertyID == i_ePropertyID )
    {
      m_eDefaultNpcType = i_vValue;
    }
  }


  function OnSpellCast()
  {
    m_bSpellReady = false;

    local fInterval = GetProperty( Creature_Spell_Cooldown_Duration_PropertyID, 20.0 );
    ::rumSchedule( this, OnSpellReady, fInterval );
  }


  function OnSpellReady()
  {
    m_bSpellReady = true;
  }


  function OnWeaponUsed( i_eWeaponType )
  {
    if( GetWeaponType() != i_eWeaponType )
    {
      // Since the primary method of attack wasn't used, set a cooldown on the special attack
      local fInterval = GetProperty( Creature_Special_Weapon_Cooldown_Duration_PropertyID, 10.0 );
      ::rumSchedule( this, ReadySpecialWeapon, fInterval );

      m_bSpecialWeaponReady = false;
    }
  }


  function Poison( i_bForce = false )
  {
    if( i_bForce || !( IsVenomous() || ResistsPoison() || IsUndead() ) )
    {
      base.Poison( i_bForce );
    }
  }


  function ReadySpecialWeapon()
  {
    if( m_eSpecialWeaponArray.len() > 0 )
    {
      m_bSpecialWeaponReady = true;
    }
  }


  function Rehome()
  {
    InterruptInteractions();

    m_iRehomeAttempts = 0;
    local ciMap = GetMap();
    ciMap.MovePawn( this, m_ciOriginPos,
                    rumIgnoreTileCollisionMoveFlag | rumIgnorePawnCollisionMoveFlag | rumIgnoreDistanceMoveFlag );
  }


  function ResistsFire()
  {
    return GetProperty( Creature_Resists_Fire_PropertyID, false );
  }


  function ResistsLightning()
  {
    return GetProperty( Creature_Resists_Lightning_PropertyID, false );
  }


  function ResistsPoison()
  {
    return GetProperty( Creature_Resists_Poison_PropertyID, false );
  }


  function ResistsSleep()
  {
    return GetProperty( Creature_Resists_Sleep_PropertyID, false );
  }


  function Respawn()
  {
    SetProperty( Hitpoints_PropertyID, GetMaxHitpoints() );
    SetVisibility( true );

    Rehome();
  }


  function StopFleeing()
  {
    if( IsFleeing() && !IsDead() )
    {
      // Reward player for sparing the NPC
      local ciTarget = ::rumFetchPawn( m_uiFleeTargetID );
      if( ciTarget != null && ( ciTarget instanceof Player ) )
      {
        ciTarget.AffectVirtue( VirtueType.Compassion, 2, false, true );
      }
    }

    m_uiFleeTargetID = rumInvalidGameID;
  }


  function Talk( i_ciPlayer )
  {
    if( HasAttackTarget() || IsFleeing() )
    {
      // Funny, no response
      i_ciPlayer.ActionFailed( msg_no_response_client_StringID );
      return;
    }

    if( NPCType.Merchant == m_eDefaultNpcType )
    {
      local ciMap = GetMap();
      ciMap.Transact( m_iDialogueID, i_ciPlayer );

      InteractionBegin( i_ciPlayer );
    }
    else
    {
      local eVersion = i_ciPlayer.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
      switch( eVersion )
      {
        case GameType.Ultima4: TalkU4( i_ciPlayer ); break;
        case GameType.Ultima3: TalkU3( i_ciPlayer ); break;
        case GameType.Ultima2: TalkU2( i_ciPlayer ); break;
        case GameType.Ultima1: TalkU1( i_ciPlayer ); break;
      }
    }
  }


  function TalkEnd( i_ciPlayer, i_eTerminationType )
  {
    InteractionEnd( i_ciPlayer );

    if( NPCType.Merchant != m_eDefaultNpcType )
    {
      local ciBroadcast = ::rumCreate( Player_Talk_Bye_BroadcastID, i_eTerminationType );
      ::rumSendPrivate( i_ciPlayer.GetSocket(), ciBroadcast );
    }
  }
}


class U1_NPC extends NPC
{
  function OnDeath( i_ciSource, i_bKillCredit = true )
  {
    base.OnDeath( i_ciSource, i_bKillCredit );

    // There is a small chance of giving items to players
    if( i_bKillCredit && ( i_ciSource instanceof Player ) )
    {
      // There is a 15% chance of finding a torch on torch carrying NPCs
      local bRewardsTorch = GetProperty( Creature_Rewards_Torch_PropertyID, false );
      if( bRewardsTorch && rand() % 100 < 15 )
      {
        local iNewAmount = i_ciSource.GetProperty( U1_Torches_PropertyID, 0 ) + 1;
        i_ciSource.SetProperty( U1_Torches_PropertyID, iNewAmount );
      }
    }
  }


  function TalkU1( i_ciPlayer )
  {
    if( NPCType.Special == m_eDefaultNpcType )
    {
      if( NPCSpecialType.U1_King == m_iDialogueID )
      {
        InteractionBegin( i_ciPlayer );
        local ciBroadcast = ::rumCreate( Player_Talk_U1_King_BroadcastID, U1_KingTalkType.Greet );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else if( NPCSpecialType.U1_Princess == m_iDialogueID )
      {
        local ciMap = GetMap();

        local iExp = i_ciPlayer.GetProperty( U1_Experience_PropertyID, 0 );
        local iLvl = iExp / 1000 + 1;

        local strGreet = ::rumGetString( u1_princess_saved_server_StringID, i_ciPlayer.m_iLanguageID );
        local eNameStringID = GetProperty( NPC_Name_PropertyID, rumInvalidStringToken );

        // Has this princess been previously saved?
        local ePrincessID = GetProperty( Princess_ID_PropertyID, 0 );
        local iPrincessFlags = i_ciPlayer.GetProperty( U1_Rescued_Princess_Flags_PropertyID, 0 );
        local bSaved = ::rumBitOn( iPrincessFlags, ePrincessID );
        if( !bSaved )
        {
          local ciBroadcast = ::rumCreate( Player_Talk_U1_Princess_BroadcastID, U1_PrincessTalkType.Greet, eNameStringID,
                                           strGreet );
          i_ciPlayer.SendBroadcast( ciBroadcast );

          i_ciPlayer.SetProperty( U1_Experience_PropertyID, iExp + 500 );

          i_ciPlayer.AffectHitpoints( 500 );
          i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, 500 );

          // Experience might have been clamped, so grab the value again
          iExp = i_ciPlayer.GetProperty( U1_Experience_PropertyID, 0 );
          iLvl = iExp / 1000 + 1;

          // Remember that the princess has been saved
          iPrincessFlags = ::rumBitSet( iPrincessFlags, ePrincessID );
          i_ciPlayer.SetProperty( U1_Rescued_Princess_Flags_PropertyID, iPrincessFlags );
        }

        local bSpaceAce = i_ciPlayer.GetProperty( U1_Space_Enemies_Killed_PropertyID, 0 ) >= 20;
        if( !bSpaceAce || iLvl < 8 )
        {
          local eExitMapID = ciMap.GetExitMapID();
          local ciDestMap = GetOrCreateMap( i_ciPlayer, eExitMapID );
          if( ciDestMap != null )
          {
            ciMap.Exit( i_ciPlayer, ciDestMap );
          }

          // Not ready for time travel
          local strDesc = ::rumGetString( u1_princess_not_ready_server_StringID, i_ciPlayer.m_iLanguageID );

          local ciBroadcast = ::rumCreate( Player_Talk_U1_Princess_BroadcastID, U1_PrincessTalkType.TimeMachine, strDesc );
          i_ciPlayer.SendBroadcast( ciBroadcast );
        }
        else
        {
          local iFlags = i_ciPlayer.GetProperty( Ultima_Completions_PropertyID, 0 );
          local bKilledMondain = ::rumBitOn( iFlags, UltimaCompletions.KilledMondain );
          if( !bKilledMondain )
          {
            // The princess gives the player directions to a hidden time machine
            local strDesc = ::rumGetString( u1_princess_time_machine_server_StringID, i_ciPlayer.m_iLanguageID );

            local ciBroadcast = ::rumCreate( Player_Talk_U1_Princess_BroadcastID, U1_PrincessTalkType.TimeMachine, strDesc );
            i_ciPlayer.SendBroadcast( ciBroadcast );

            // Transfer player to time machine
            local ciDestMap = GetOrCreateMap( i_ciPlayer, U1_Time_Machine_MapID );
            if( ciDestMap != null )
            {
              ciMap.TransferPawn( i_ciPlayer, ciDestMap, ::rumPos( 5, 3 ) );
            }
          }
          else
          {
            local strDesc = ::rumGetString( u4_codex_endgame5_server_StringID, i_ciPlayer.m_iLanguageID );

            local ciBroadcast = ::rumCreate( Player_Talk_U1_Princess_BroadcastID, U1_PrincessTalkType.TimeMachine, strDesc );
            i_ciPlayer.SendBroadcast( ciBroadcast );

            local eExitMapID = ciMap.GetExitMapID();
            local ciDestMap = GetOrCreateMap( i_ciPlayer, eExitMapID );
            if( ciDestMap != null )
            {
              ciMap.Exit( i_ciPlayer, ciDestMap );
            }
          }
        }
      }
    }
    else if( NPCType.Standard == m_eDefaultNpcType )
    {
      if( 0 == m_iDialogueID )
      {
        if( U1_Guard_CreatureID == GetAssetID() )
        {
          // Re-use U2 guard responses
          local strDesc = format( "u2_dlg_guard_%d_server_StringID", rand() % 3 + 1 );
          strDesc = ::rumGetStringByName( strDesc, i_ciPlayer.m_iLanguageID );

          local ciBroadcast = ::rumCreate( Player_Talk_BroadcastID, strDesc );
          i_ciPlayer.SendBroadcast( ciBroadcast );
        }
        else
        {
          i_ciPlayer.ActionFailed( msg_no_response_client_StringID );
        }
        return;
      }
      else
      {
        // Grab the NPC response from the database and send it to the client
        local strDesc = format( "u1_dlg_%d_server_StringID", m_iDialogueID );
        strDesc = ::rumGetStringByName( strDesc, i_ciPlayer.m_iLanguageID );

        local ciBroadcast = ::rumCreate( Player_Talk_BroadcastID, strDesc );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
    }
  }
}


class U2_NPC extends NPC
{
  static s_iHitpointTributeCost = 50;
  static s_iRoomCost = 50;
  static s_iRoomCostWithTip = 100;
  static s_iStatTributeCost = 100;

  static s_iClerkStatIncrementAmount = 4;
  static s_iLBStatIncrementAmount = 2;


  function TalkU2( i_ciPlayer )
  {
    if( NPCType.Standard == m_eDefaultNpcType )
    {
      local strDesc;

      if( 0 == m_iDialogueID )
      {
        local strClass;
        local eAssetID = GetAssetID();

        if( U2_Guard_CreatureID == eAssetID )
        {
          strClass = "guard";
        }
        else if( U2_Jester_CreatureID == eAssetID )
        {
          strClass = "jester";
        }
        else if( U2_Villager_CreatureID == eAssetID )
        {
          strClass = "villager";
        }
        else if( U2_Fighter_CreatureID == eAssetID )
        {
          strClass = "fighter";
        }
        else if( U2_Cleric_CreatureID == eAssetID )
        {
          strClass = "cleric";
        }
        else if( U2_Wizard_CreatureID == eAssetID )
        {
          strClass = "wizard";
        }
        else if( U2_Thief_CreatureID == eAssetID )
        {
          strClass = "thief";
        }

        if( strClass != null )
        {
          strDesc = ::rumGetStringByName( format( "u2_dlg_%s_%d_server_StringID", strClass, rand() % 3 + 1 ),
                                          i_ciPlayer.m_iLanguageID );
        }
      }
      else
      {
        strDesc = ::rumGetStringByName( format( "u2_dlg_%d_server_StringID", m_iDialogueID ), i_ciPlayer.m_iLanguageID );
      }

      if( strDesc != null )
      {
        local ciBroadcast = ::rumCreate( Player_Talk_BroadcastID, strDesc );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_no_response_client_StringID );
      }

      // Does the interaction with this NPC need to be remembered?
      local ePropertyID = GetProperty( Interaction_Stamp_PropertyID, rumInvalidAssetID );
      if( ePropertyID != rumInvalidAssetID )
      {
        i_ciPlayer.SetProperty( ePropertyID, true );
      }
    }
    else if( NPCType.Special == m_eDefaultNpcType )
    {
      if( NPCSpecialType.U2_LordBritish == m_iDialogueID )
      {
        InteractionBegin( i_ciPlayer );
        local ciBroadcast = ::rumCreate( Player_Talk_U2_LordBritish_BroadcastID, U2_LBTalkType.Greet );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else if( NPCSpecialType.U2_HotelClerk == m_iDialogueID )
      {
        InteractionBegin( i_ciPlayer );
        local ciBroadcast = ::rumCreate( Player_Talk_U2_HotelClerk_BroadcastID, U2_ClerkTalkType.Greet );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else if( NPCSpecialType.U2_OldMan == m_iDialogueID )
      {
        local ciBroadcast;

        local uiRingQuestState = i_ciPlayer.GetProperty( U2_Ring_Quest_State_PropertyID, RingQuestState.Started );
        if( uiRingQuestState > RingQuestState.Started )
        {
          // Tell the player where to find the magic ring
          local iRingIndex = i_ciPlayer.GetProperty( U2_Ring_Index_PropertyID, 0 );
          local eMapID = g_eU2RingMapArray[iRingIndex];
          local strDesc = ::rumGetString( u2_oldman_ring_location_server_StringID, i_ciPlayer.m_iLanguageID );
          ciBroadcast = ::rumCreate( Player_Talk_U2_OldMan_BroadcastID, strDesc, eMapID );

          if( uiRingQuestState != RingQuestState.ReceivedDirections )
          {
            i_ciPlayer.SetProperty( U2_Ring_Quest_State_PropertyID, RingQuestState.ReceivedDirections );
          }
        }
        else
        {
          local iOracleFlags = i_ciPlayer.GetProperty( U2_Oracle_Flags_PropertyID, 0 );
          local ciProperty = ::rumGetPropertyAsset( U2_Oracle_Flags_PropertyID );
          local bPurchasedAllClues = ( iOracleFlags == ciProperty.GetMaxValue() );
          if( bPurchasedAllClues )
          {
            local strDesc = ::rumGetString( u2_oldman_antos_server_StringID, i_ciPlayer.m_iLanguageID );
            ciBroadcast = ::rumCreate( Player_Talk_BroadcastID, strDesc );
          }
          else
          {
            local strDesc = ::rumGetString( u2_oldman_greet_server_StringID, i_ciPlayer.m_iLanguageID );
            ciBroadcast = ::rumCreate( Player_Talk_BroadcastID, strDesc );
          }
        }

        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else if( NPCSpecialType.U2_Oracle == m_iDialogueID )
      {
        local bPaid = false;

        // Show the clue if the player has already paid this Oracle
        local iOracleID = GetProperty( U2_Oracle_ID_PropertyID, 0 );
        if( iOracleID > 0 )
        {
          local iOracleFlags = i_ciPlayer.GetProperty( U2_Oracle_Flags_PropertyID, 0 );
          bPaid = ::rumBitOn( iOracleFlags, iOracleID );
          if( bPaid )
          {
            // Show the Oracle's clue
            local strClue = format( "u2_oracle_%d_server_StringID", iOracleID - 1 );
            strClue = ::rumGetStringByName( strClue, i_ciPlayer.m_iLanguageID );

            local ciBroadcast = ::rumCreate( Player_Talk_U2_Oracle_BroadcastID, U2_OracleTalkType.Clue, strClue );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
        }

        if( !bPaid )
        {
          InteractionBegin( i_ciPlayer );
          local ciBroadcast = ::rumCreate( Player_Talk_U2_Oracle_BroadcastID, U2_ClerkTalkType.Greet );
          i_ciPlayer.SendBroadcast( ciBroadcast );
        }
      }
      else if( NPCSpecialType.U2_Sentri == m_iDialogueID )
      {
        local eQuestState = i_ciPlayer.GetProperty( U2_Quicksword_Quest_State_PropertyID, QuickswordQuestState.NotStarted )
        if( QuickswordQuestState.NotStarted == eQuestState )
        {
          local strDesc = ::rumGetString( u2_sentri_greet_server_StringID, i_ciPlayer.m_iLanguageID );
          local ciBroadcast = ::rumCreate( Player_Talk_U2_Sentri_BroadcastID, strDesc );
          i_ciPlayer.SendBroadcast( ciBroadcast );

          i_ciPlayer.SetProperty( U2_Quicksword_Quest_State_PropertyID, QuickswordQuestState.Started );
        }
        else if( QuickswordQuestState.Started == eQuestState )
        {
          local uiMaterialFlags = i_ciPlayer.GetProperty( U2_Item_Quicksword_Materials_PropertyID, 0 );
          if( ::rumBitAllOn( uiMaterialFlags, QuickswordMaterialFlags.All ) )
          {
            // Give the quicksword to the player
            i_ciPlayer.AddOrCreateItem( U2_Quick_Sword_Weapon_InventoryID );
            i_ciPlayer.SetProperty( U2_Quicksword_Quest_State_PropertyID, QuickswordQuestState.Finished );
            i_ciPlayer.RemoveProperty( U2_Item_Quicksword_Materials_PropertyID );

            local strDesc = ::rumGetString( u2_sentri_quicksword_server_StringID, i_ciPlayer.m_iLanguageID );
            local ciBroadcast = ::rumCreate( Player_Talk_U2_Sentri_BroadcastID, strDesc );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
          else if( !::rumBitOn( uiMaterialFlags, QuickswordMaterialFlags.Blade ) )
          {
            local strDesc = ::rumGetString( u2_sentri_hint_0_server_StringID, i_ciPlayer.m_iLanguageID );
            local ciBroadcast = ::rumCreate( Player_Talk_U2_Sentri_BroadcastID, strDesc );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
          else if( !::rumBitOn( uiMaterialFlags, QuickswordMaterialFlags.Pommel ) )
          {
            local strDesc = ::rumGetString( u2_sentri_hint_1_server_StringID, i_ciPlayer.m_iLanguageID );
            local ciBroadcast = ::rumCreate( Player_Talk_U2_Sentri_BroadcastID, strDesc );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
          else if( !::rumBitOn( uiMaterialFlags, QuickswordMaterialFlags.Blade ) )
          {
            local strDesc = ::rumGetString( u2_sentri_hint_2_server_StringID, i_ciPlayer.m_iLanguageID );
            local ciBroadcast = ::rumCreate( Player_Talk_U2_Sentri_BroadcastID, strDesc );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
        }
        else if( QuickswordQuestState.Finished == eQuestState )
        {
          local strDesc = ::rumGetString( u2_sentri_hint_3_server_StringID, i_ciPlayer.m_iLanguageID );
          local ciBroadcast = ::rumCreate( Player_Talk_U2_Sentri_BroadcastID, strDesc );
          i_ciPlayer.SendBroadcast( ciBroadcast );
        }
      }
    }
  }
}


class U3_NPC extends NPC
{
  static s_iBribeCost = 100;


  function TalkU3( i_ciPlayer )
  {
    if( NPCType.Standard == m_eDefaultNpcType )
    {
      local strDesc;

      if( 0 == m_iDialogueID )
      {
        // Good day!
        strDesc = ::rumGetString( u3_dlg_good_day_server_StringID, i_ciPlayer.m_iLanguageID );
      }
      else
      {
        // Grab the NPC response from the database and send it to the client
        strDesc = ::rumGetStringByName( format( "u3_dlg_%d_server_StringID", m_iDialogueID ), i_ciPlayer.m_iLanguageID );
      }

      local ciBroadcast = ::rumCreate( Player_Talk_BroadcastID, strDesc );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
    else if( NPCType.Special == m_eDefaultNpcType )
    {
      if( NPCSpecialType.U3_LordBritish == m_iDialogueID )
      {
        InteractionBegin( i_ciPlayer );
        local ciBroadcast = ::rumCreate( Player_Talk_U3_LordBritish_BroadcastID, U3_LBTalkType.Greet );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else if( NPCSpecialType.U3_Timelord == m_iDialogueID )
      {
        local iExodusIndex = i_ciPlayer.GetProperty( U3_Exodus_Card_Index_PropertyID, 9 );

        local ciBroadcast = ::rumCreate( Player_Talk_U3_Timelord_BroadcastID,
                                         g_uiU3ExodusOrderIndex0Array[iExodusIndex],
                                         g_uiU3ExodusOrderIndex1Array[iExodusIndex],
                                         g_uiU3ExodusOrderIndex2Array[iExodusIndex],
                                         g_uiU3ExodusOrderIndex3Array[iExodusIndex] );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
    }
  }
}


class U4_NPC extends NPC
{
  function TalkU4( i_ciPlayer )
  {
    if( NPCType.Special == m_eDefaultNpcType )
    {
      if( NPCSpecialType.U4_SeerHawkwind == m_iDialogueID )
      {
        InteractionBegin( i_ciPlayer );
        local ciBroadcast = ::rumCreate( Player_Talk_Seer_BroadcastID, U4_SeerTalkType.Greet );
        i_ciPlayer.SendBroadcast( ciBroadcast );
        return;
      }
      else if( NPCSpecialType.U4_LordBritish != m_iDialogueID )
      {
        // Funny, no response
        i_ciPlayer.ActionFailed( msg_no_response_client_StringID );
        return;
      }
    }
    else if( 0 == m_iDialogueID )
    {
      // Funny, no response
      i_ciPlayer.ActionFailed( msg_no_response_client_StringID );
      return;
    }

    InteractionBegin( i_ciPlayer );

    // Grab the standard NPC responses from the database, and send them to the client
    local strDesc = "u4_dlg_" + m_iDialogueID;
    local ePronounType = GetProperty( NPC_Pronoun_Type_PropertyID, PronounType.He );
    local strName = ::rumGetStringByName( strDesc + "_name_server_StringID", i_ciPlayer.m_iLanguageID );
    local strLook = ::rumGetStringByName( strDesc + "_look_server_StringID", i_ciPlayer.m_iLanguageID );

    local ciBroadcast = ::rumCreate( Player_Talk_BroadcastID, m_iDialogueID, ePronounType, strName, strLook );
    i_ciPlayer.SendBroadcast( ciBroadcast );

    // Is the player speaking to Lord British?
    if( ( NPCType.Special == m_eDefaultNpcType ) && ( NPCSpecialType.U4_LordBritish == m_iDialogueID ) )
    {
      local bInitialMeeting = false;

      local eStage = i_ciPlayer.GetProperty( U4_Lord_British_Stage_PropertyID, LBHelpStageType.Introduction );
      if( LBHelpStageType.Introduction == eStage )
      {
        // The player is meeting Lord British for the first time
        ciBroadcast = ::rumCreate( Player_Talk_Lord_British_BroadcastID, U4_LBTalkType.Greet );
        i_ciPlayer.SendBroadcast( ciBroadcast );

        // Remember that player has met Lord British
        i_ciPlayer.SetProperty( U4_Lord_British_Stage_PropertyID, LBHelpStageType.GeneralAdvice );

        bInitialMeeting = true;
      }

      local bPlayerLeveled = false;

      local iPlayerLevel = i_ciPlayer.GetProperty( U4_Level_PropertyID, 1 );
      local iMaxLevel = ::rumGetMaxPropertyValue( U4_Level_PropertyID );
      if( iPlayerLevel < iMaxLevel )
      {
        local iExpLevel = i_ciPlayer.GetExperienceLevel();

        if( iPlayerLevel < iExpLevel )
        {
          local iLevelDelta = iExpLevel - iPlayerLevel;

          local ePlayerClassID = i_ciPlayer.GetProperty( U4_PlayerClass_PropertyID, U4_Mage_Class_CustomID );
          local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );

          local iStrengthBonus = ciPlayerClass.GetProperty( Class_Strength_Bonus_PropertyID, 0 ) * iLevelDelta;
          local iDexterityBonus = ciPlayerClass.GetProperty( Class_Dexterity_Bonus_PropertyID, 0 ) * iLevelDelta;
          local iIntelligenceBonus = ciPlayerClass.GetProperty( Class_Intelligence_Bonus_PropertyID, 0 ) *
                                     iLevelDelta;

          local iStrength= i_ciPlayer.GetProperty( U4_Strength_PropertyID, 0 );
          local iDexterity = i_ciPlayer.GetProperty( U4_Dexterity_PropertyID, 0 );
          local iIntelligence = i_ciPlayer.GetProperty( U4_Intelligence_PropertyID, 0 );

          local iNewStrength= iStrength+ iStrengthBonus;
          iNewStrength= min( iNewStrength, ciPlayerClass.GetProperty( Class_Strength_Cap_PropertyID, 99 ) );

          local iNewDexterity = iDexterity + iDexterityBonus;
          iNewDexterity = min( iNewDexterity, ciPlayerClass.GetProperty( Class_Dexterity_Cap_PropertyID, 99 ) );

          local iNewIntelligence = iIntelligence + iIntelligenceBonus;
          iNewIntelligence = min( iNewIntelligence,
                                  ciPlayerClass.GetProperty( Class_Intelligence_Cap_PropertyID, 99 ) );

          // Build an array for the client to determine everything that changed
          local iStatChangeArray = [ iExpLevel,
                                     iNewStrength - iStrength,
                                     iNewDexterity - iDexterity,
                                     iNewIntelligence - iIntelligence ];

          // Send the level up information to the client
          ciBroadcast = ::rumCreate( Player_Talk_Lord_British_BroadcastID, U4_LBTalkType.LevelUp, iStatChangeArray );
          i_ciPlayer.SendBroadcast( ciBroadcast );

          // Save level and attribute changes
          i_ciPlayer.SetProperty( U4_Level_PropertyID, iExpLevel );
          i_ciPlayer.SetProperty( U4_Strength_PropertyID, iNewStrength );
          i_ciPlayer.SetProperty( U4_Dexterity_PropertyID, iNewDexterity );
          i_ciPlayer.SetProperty( U4_Intelligence_PropertyID, iNewIntelligence );

          // Give the player a free heal
          i_ciPlayer.AffectHitpoints( i_ciPlayer.GetMaxHitpoints() );

          // Show cast effect on LB and affected player
          local ciMap = GetMap();

          SendClientEffect( this, ClientEffectType.Cast );
          SendClientEffect( i_ciPlayer, ClientEffectType.Cast );

          bPlayerLeveled = true;
        }
      }

      if( !bInitialMeeting )
      {
        ciBroadcast = ::rumCreate( Player_Talk_Lord_British_BroadcastID, U4_LBTalkType.Standard );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
    }
  }
}
