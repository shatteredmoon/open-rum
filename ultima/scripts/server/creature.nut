class Creature extends rumCreature
{
  m_iMoveIndex = 0;
  m_iPoisonIndex = 0;

  m_ciEffectsTable = null;


  constructor()
  {
    base.constructor();
    m_ciEffectsTable = {};
  }


  function ActionFailed( i_strReason )
  {}


  function AffectHitpoints( i_iDelta )
  {
    local iHitpoints = GetProperty( Hitpoints_PropertyID, 0 );
    local iMaxHitpoints = GetMaxHitpoints();

    local iNewHitpoints = iHitpoints + i_iDelta;
    iNewHitpoints = clamp( iNewHitpoints, 0, iMaxHitpoints );

    if( iHitpoints != iNewHitpoints )
    {
      SetProperty( Hitpoints_PropertyID, iNewHitpoints );
    }
  }


  function ApplyPoison()
  {
    SetProperty( Poisoned_PropertyID, true );
  }


  function ApplyPositionEffects()
  {
    local ciMap = GetMap();
    if( ciMap )
    {
      ++m_iMoveIndex;

      local ciPosData = ciMap.GetPositionData( GetPosition() );

      if( IsPositionHarmful( GetPosition() ) )
      {
        local ciTile = ::rumGetTileAsset( ciPosData.GetTileID() );
        local fReapplyTimer = 2.0;

        local eTileID = ciTile.GetAssetID();
        switch( eTileID )
        {
          case U2_Marsh_TileID:
            {
              local iDamage = 10;
              Damage( rand()%iDamage + 1, eTileID );
              ::rumSchedule( this, ReApplyPositionEffects, fReapplyTimer, m_iMoveIndex );
            }
            break;

          case U3_Lava_TileID:
          case U4_Lava_TileID:
            Burn();
            ::rumSchedule( this, ReApplyPositionEffects, fReapplyTimer, m_iMoveIndex );
            break;

          case U4_Marsh_TileID:
            Poison();
            ::rumSchedule( this, ReApplyPositionEffects, fReapplyTimer, m_iMoveIndex );
            break;
        }
      }

      local ciWidget;
      while( ciWidget = ciPosData.GetNext( rumWidgetPawnType ) )
      {
        // See if this widget affects the creature. NOTE: We do this even if the widget is currently invisible
        // in the event that it becomes visible while a creature is occupying the same space.
        ciWidget.AffectCreature( this );
      }
    }
  }


  function Attack( i_ciTarget, i_vValue )
  {
    if( null == i_ciTarget )
    {
      return AttackReturnType.Failed_InvalidTarget;
    }

    local eWeaponType = i_vValue;
    if( i_vValue instanceof Transport_Widget )
    {
      eWeaponType = i_vValue.GetWeaponType();
    }

    local eArmourType = GetArmourType();
    local bValidTarget = false;

    if( i_ciTarget.IsVisible() )
    {
      if( i_ciTarget instanceof Widget )
      {
        bValidTarget = i_ciTarget.GetProperty( Widget_Destructible_PropertyID, false );
      }
      else
      {
        bValidTarget = !i_ciTarget.IsDead();
      }
    }

    if( !bValidTarget )
    {
      return AttackReturnType.Failed_InvalidTarget;
    }

    if( i_ciTarget instanceof Player )
    {
      if( this instanceof Player )
      {
        local ciParty = GetParty();
        if( ciParty != null )
        {
          // Is the target in the attacker's party?
          if( ciParty.HasMember( i_ciTarget.GetID() ) )
          {
            return AttackReturnType.Failed_InvalidTarget;
          }
        }
      }

      // If the target player is on a transport, the transport becomes the target
      local ciTransport = i_ciTarget.GetTransport();
      if( ciTransport != null )
      {
        i_ciTarget = ciTransport;
      }
    }

    local ciWeapon = eWeaponType ? ::rumGetInventoryAsset( eWeaponType ) : null;
    if( ciWeapon != null )
    {
      //print( "!! Attacker " + GetName() + " has weapon: " + ciWeapon.GetName() + "\n" );
    }

    // Hands/claws do a minimum weapon damage of 8
    local iBaseDamage = ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Damage_PropertyID, 8 ) : 8;
    local iDamage = iBaseDamage;

    local iRange = ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Attack_Range_PropertyID, 1 ) : 1;
    if( eWeaponType == U4_Halberd_Weapon_InventoryID )
    {
      iRange = 2;
    }

    local ciArmour = eArmourType ? ::rumGetInventoryAsset( eArmourType ) : null;
    if( ciArmour != null )
    {
      //print( "!! Attacker " + GetName() + " has armour: " + ciArmour.GetName() + "\n" );
    }

    local ciPos = GetPosition();
    local ciTargetPos = i_ciTarget.GetPosition();

    local ciMap = GetMap();
    local ciTargetMap = i_ciTarget.GetMap();
    if( ciMap != ciTargetMap )
    {
      print( "***Ignoring attack - attacker and target are on different maps!\n" );
      return AttackReturnType.Failed_OutOfRange;
    }

    local iDistance = ciMap.GetTileDistance( ciPos, ciTargetPos );

    local bTargetInRange = iRange >= iDistance;
    if( !bTargetInRange )
    {
      return AttackReturnType.Failed_OutOfRange;
    }

    if( iDistance > 1 )
    {
      local bTargetInLOS = ciMap.TestLOS( ciPos, ciTargetPos, iRange );
      if( !bTargetInLOS )
      {
        return AttackReturnType.Failed_LOS;
      }

      local bHasClearPath = ciMap.HasClearPath( ciPos, ciTargetPos, MoveType.Ballistic);
      if( !bHasClearPath )
      {
        return AttackReturnType.Failed_Collision;
      }

      local bThrowable = ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Throwable_PropertyID, false ) : false;
      if( bThrowable )
      {
        OnWeaponThrown( eWeaponType, iDistance );
      }
    }

    local iTargetDefense = 0;
    local iAttackerOffense = 0;

    local eMapID = ciMap.GetAssetID();

    local bAlwaysHits = ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Always_Hits_PropertyID, false ) : false;
    if( !bAlwaysHits )
    {
      local eTargetArmourType = i_ciTarget.GetArmourType();
      local ciTargetArmour = eTargetArmourType ? ::rumGetInventoryAsset( eTargetArmourType ) : null;

      if( i_ciTarget instanceof Player )
      {
        local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
        if( MapType.Abyss == eMapType )
        {
          local bMagical = ciTargetArmour ? ciTargetArmour.GetProperty( Inventory_Magical_PropertyID, false ) : false;
          if( !bMagical )
          {
            // The player's armour is effectively useless here
            eTargetArmourType = rumInvalidAssetID;
          }
        }
        else if( ( ( U3_Castle_Fire_MapID == eMapID ) || ( U3_Castle_Fire_2_MapID == eMapID ) ) &&
                 eTargetArmourType != U3_Exotic_Armour_InventoryID )
        {
          // The player's armour is effectively useless here
          eTargetArmourType = rumInvalidAssetID;
        }
        else if( U2_Earth_Legends_Castle_Shadow_Guard_MapID == eMapID )
        {
          local bHasRing = i_ciTarget.GetProperty( U2_Magic_Ring_PropertyID, false );
          if( !bHasRing )
          {
            // The player's armour is effectively useless here
            eTargetArmourType = rumInvalidAssetID;
          }
        }
      }

      // Does the target's armour have a dexterity penalty?
      local iDefensePenalty =
        ciTargetArmour != null ? ciTargetArmour.GetProperty( Inventory_Armour_Dexterity_Penalty_PropertyID, 0 ) : 0;

      // Does the target's strength offset the penalty?
      if( iDefensePenalty > 0 )
      {
        iDefensePenalty = iDefensePenalty * ( ( i_ciTarget.GetStrength() + 1 ) / 100 );
      }

      local iDefense = i_ciTarget.GetDexterity() - iDefensePenalty;
      iTargetDefense = rand() % iDefense;

      local iOffensePenalty =
        ciArmour != null ? ciArmour.GetProperty( Inventory_Armour_Dexterity_Penalty_PropertyID, 0 ) : 0;

      // Does the target's strength offset the penalty?
      if( iOffensePenalty > 0 )
      {
        iOffensePenalty = iOffensePenalty * ( ( GetStrength() + 1 ) / 100 );
      }

      local iOffense = i_ciTarget.GetDexterity() - iDefensePenalty;
      iAttackerOffense = rand() % iOffense;
    }

    local eVersion =
      ciWeapon != null ? ciWeapon.GetProperty( Ultima_Version_PropertyID, GameType.Invalid ) : GameType.Invalid;

    local bDamaged = false;
    local eAttackResult = AttackResultType.Hit;
    if( bAlwaysHits || iAttackerOffense > iTargetDefense )
    {
      if( this instanceof Player )
      {
        // The target was hit, determine effectiveness
        local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
        if( MapType.Abyss == eMapType )
        {
          local bMagical = ciWeapon != null ? ciWeapon.GetProperty( Inventory_Magical_PropertyID, false ) : false;
          if( !bMagical )
          {
            eAttackResult = AttackResultType.Unaffected;
          }
        }
        else if( ( ( U3_Castle_Fire_MapID == eMapID ) || ( U3_Castle_Fire_2_MapID == eMapID ) ) &&
                 eWeaponType != U3_Exotic_Weapon_InventoryID )
        {
          eAttackResult = AttackResultType.Unaffected;
        }
        else if( ( U2_Earth_Legends_Castle_Shadow_Guard_MapID == eMapID ) &&
                 eWeaponType != U2_Quick_Sword_Weapon_InventoryID )
        {
          eAttackResult = AttackResultType.Unaffected;
        }
      }

      if( i_ciTarget instanceof Creature )
      {
        if( GameType.Ultima4 == eVersion )
        {
          local bFireEffect =
            ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Fire_Effect_PropertyID, false ) : false;
          if( bFireEffect )
          {
            if( i_ciTarget.ResistsFire() )
            {
              eAttackResult = AttackResultType.Unaffected;
            }
            else
            {
              i_ciTarget.Burn();
            }
          }
        }

        local bIceEffect =
          ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Ice_Effect_PropertyID, false ) : false;
        if( bIceEffect )
        {
          i_ciTarget.Freeze();
        }

        local bLightningEffect =
          ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Lightning_Effect_PropertyID, false ) : false;
        if( bLightningEffect )
        {
          if( i_ciTarget.ResistsLightning() )
          {
            eAttackResult = AttackResultType.Unaffected;
          }
          else
          {
            // Start an electrify chain on the target (the attack is automatically immune)
            local ciImmunityTable = {};
            ciImmunityTable[this] <- true;
            local iChainDepth =
              ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Damage_Chain_Depth_PropertyID, 0 ) : 0;
            i_ciTarget.Electrify( ciImmunityTable, iChainDepth );
          }
        }

        local bPoisonEffect =
          ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Poison_Effect_PropertyID, false ) : false;
        if( bPoisonEffect || ( IsVenomous() && iRange <= 1 ) )
        {
          if( i_ciTarget.ResistsPoison() )
          {
            eAttackResult = AttackResultType.Unaffected;
          }
          else
          {
            i_ciTarget.Poison();
          }
        }

        local bSleepEffect =
          ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Sleep_Effect_PropertyID, false ) : false;
        if( bSleepEffect )
        {
          if( i_ciTarget.ResistsSleep() )
          {
            eAttackResult = AttackResultType.Unaffected;
          }
          else
          {
            i_ciTarget.Incapacitate();
          }
        }
      }

      if( AttackResultType.Hit == eAttackResult )
      {
        if( U1_Magic_Missile_Weapon_InventoryID == eWeaponType )
        {
          // Add bonus to magic missile if certain weapons are equipped. Note that this can differ from the weapon type
          // passed in via input parameter.
          switch( GetWeaponType() )
          {
            case U1_Amulet_Weapon_InventoryID:   iDamage += ( iDamage / 2 ); break;
            case U1_Wand_Weapon_InventoryID:     iDamage *= 2; break;

            case U1_Staff_Weapon_InventoryID:    // fall through
            case U1_Triangle_Weapon_InventoryID: iDamage *= 3; break;
          }
        }

        if( iDamage > 0 )
        {
          if( ( GameType.Ultima4 == eVersion ) && ( i_ciTarget instanceof Creature ) )
          {
            if( i_ciTarget.VulnerableToFire() )
            {
              local bFireEffect =
                ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Fire_Effect_PropertyID, false ) : false;
              if( bFireEffect )
              {
                iDamage += iBaseDamage;
              }
            }

            if( i_ciTarget.VulnerableToCold() )
            {
              local bIceEffect =
                ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Ice_Effect_PropertyID, false ) : false;
              if( bIceEffect )
              {
                // Double the weapon's base damage
                iDamage += iBaseDamage;
              }
            }
          }

          iDamage = rand() % iDamage + 1;

          // Strength modifier - note that nothing equipped = hands/claws that *do* have a strength bonus
          local bStrBonus =
            ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Has_Strength_Bonus_PropertyID, false ) : true;
          if( bStrBonus )
          {
            local fStrBonus = GetStrength() * 0.1 + 0.5;
            iDamage += fStrBonus.tointeger();
          }

          bDamaged = i_ciTarget.Damage( iDamage, this, eWeaponType, true );
          if( !bDamaged )
          {
            // The attack ultimately didn't do any damage
            eAttackResult = AttackResultType.Unaffected;
          }
        }

        // TEMP!
        if( this instanceof Player )
        {
          print( format( "Attacker = %d, Defender %d HIT %d damage\n", iAttackerOffense, iTargetDefense, iDamage ) );
        }
      }
      else
      {
        // TEMP!
        if( this instanceof Player )
        {
          print( format( "Attacker = %d, Defender %d UNAFFECTED\n", iAttackerOffense, iTargetDefense ) );
        }
      }
    }
    else
    {
      eAttackResult = AttackResultType.Miss;

      // TEMP!
      if( this instanceof Player )
      {
        print( format( "Attacker = %d, Defender %d MISS\n", iAttackerOffense, iTargetDefense ) );
      }
    }

    local iRadius = 11;
    local ciBroadcast = ::rumCreate( Attack_Update_BroadcastID, GetID(), i_ciTarget.GetID(), eWeaponType,
                                     eAttackResult );
    ciMap.SendRadial( ciBroadcast, ciPos, iRadius );

    if( GameType.Ultima4 == eVersion )
    {
      local eWidgetType =
        ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Creates_Widget_ID_PropertyID, null ) : null;
      if( eWidgetType != null )
      {
        local ciObject = ::rumCreate( eWidgetType );
        if( ciObject )
        {
          if( ciMap.AddPawn( ciObject, ciTargetPos ) )
          {
            local fDuration = 15.0;
            ::rumSchedule( ciObject, ciObject.Expire, fDuration );
          }
        }
      }
    }

    OnWeaponUsed( eWeaponType );

    return AttackReturnType.Success;
  }


  function Burn()
  {
    if( IsBurning() )
    {
      return true;
    }

    if( IsDead() || ResistsFire() || IsProtected() )
    {
      return false;
    }

    // Creatue gets a chance to resist being affected
    if( SkillRoll( GetDexterity() ) )
    {
      return false;
    }

    // Create the spell effect
    local ciEffect = Burn_Effect();
    ciEffect.m_uiTargetID = GetID();

    // Put the effect in the target's effect table
    m_ciEffectsTable[ciEffect] <- ciEffect;

    // Give the creature a chance to break the effect at a later time, otherwise expire
    ::rumSchedule( ciEffect, ciEffect.Update, ciEffect.s_fInterval );
    ::rumSchedule( ciEffect, ciEffect.Expire, ciEffect.s_fDuration );

    local iDamage = 16 + rand() % 32;

    // Give Aquatic creatures a bonus against damage
    if( GetMoveType() == MoveType.Aquatic )
    {
      iDamage = iDamage / 2;
    }
    else
    {
      // Non aquatic creatures may continue to burn
      SetProperty( Burning_PropertyID, true );
    }

    Damage( iDamage, null );

    if( IsFrozen() )
    {
      // Remove the freeze effect if it is active
      local ciEffect = GetEffect( Freeze_Effect );
      if( ciEffect )
      {
        ciEffect.Remove();
      }
    }

    return true;
  }


  function CastSpell( i_eSpellID, i_eDirection, i_eType, i_ciTarget )
  {
    // TODO - read somewhere that Cleric spells never fail? That's kind of boring, though.

    local bResult = false;
    if( !IsNegated() )
    {
      local ciSpell = ::rumGetCustomAsset( i_eSpellID );

      // Does this creature meet the requirements for casting the spell?
      if( ConsumeSpell( ciSpell ) )
      {
        // Check and consume mana
        if( ConsumeMana( ciSpell ) )
        {
          // Does the spell require a target?
          if( i_ciTarget != null )
          {
            if( i_ciTarget instanceof Creature )
            {
              local iRange = ciSpell.GetProperty( Spell_Range_PropertyID, 1 );

              // The caster must be able to see the target, and the target must be within spell range
              local ciMap = GetMap();
              if( ciMap.TestLOS( GetPosition(), i_ciTarget.GetPosition(), iRange ) )
              {
                bResult = Cast( ciSpell, this, i_ciTarget, i_eType );
              }
              else
              {
                ActionFailed( msg_not_here_client_StringID );
              }
            }
            else
            {
              ActionFailed( msg_no_effect_client_StringID );
            }
          }
          else
          {
            bResult = Cast( ciSpell, this, i_eDirection, i_eType );
          }
        }
        else
        {
          ActionFailed( msg_low_mana_client_StringID );
        }
      }
      else
      {
        ActionFailed( msg_not_here_client_StringID );
      }
    }
    else
    {
      ActionFailed( msg_cant_client_StringID );
    }

    return bResult
  }


  function ConsumeSpell( i_ciSpell )
  {
    return false;
  }


  function Cure()
  {
    RemoveProperty( Poisoned_PropertyID );
  }


  function Damage( i_iAmount, i_ciSource, i_eWeaponType = rumInvalidAssetID, i_bSendClientEffect = true )
  {
    if( i_iAmount <= 0 || IsDead() || IsInvincible() )
    {
      return false;
    }

    // For U1 & U2, Stamina is used for bonus damage mitigation
    local iStamina = GetStamina();
    if( iStamina != null && i_iAmount > 1 )
    {
      // Reduce damage based on Stamina
      local fModifier = iStamina / 2.0 / 100.0;
      i_iAmount -= ( i_iAmount * fModifier ).tointeger();
      i_iAmount = max( i_iAmount, 1 );
    }

    if( IsProtected() )
    {
      local fAmount = i_iAmount - ( i_iAmount * 0.95 );
      local i_iAmount = fAmount.tointeger();
    }
    else
    {
      local eArmourType = GetArmourType();
      local ciArmour = eArmourType != rumInvalidAssetID ? ::rumGetInventoryAsset( eArmourType ) : null;
      if( ciArmour != null )
      {
        local fMitigation = ciArmour.GetProperty( Inventory_Armour_Damage_Mitigation_PropertyID, 0.0 );
        if( fMitigation > 0.0 )
        {
          local fAmount = i_iAmount - ( i_iAmount * fMitigation );
          i_iAmount = fAmount.tointeger();

          // TODO Deduct mitigation amount from armour hp for wear on the armour?
        }
      }
    }

    AffectHitpoints( -i_iAmount );

    if( i_bSendClientEffect )
    {
      SendClientEffect( this, ClientEffectType.Damage );
    }

    if( IsDead() )
    {
      local bKillCredit = true;

      local ciWeapon = i_eWeaponType != rumInvalidAssetID ? ::rumGetInventoryAsset( i_eWeaponType ) : null;
      if( ciWeapon != null )
      {
        local bInhibitKillCredit = ciWeapon.GetProperty( Weapon_Inhibits_Kill_Credit_PropertyID, false );
        bKillCredit = !bInhibitKillCredit;
      }

      if( i_ciSource != null && ( i_ciSource instanceof Creature ) )
      {
        print( "**** CREATURE KILLED BY " + i_ciSource + " " + i_ciSource.GetName() + "\n" );

        // Inform the attacker
        i_ciSource.OnKilled( this, bKillCredit );
      }

      OnDeath( i_ciSource, bKillCredit );
    }

    return true;
  }


  function Electrify( io_ciImmunityTable, io_iRecursionDepth )
  {
    if( !IsVisible() || IsDead() || IsProtected() || ResistsLightning() || ( this in io_ciImmunityTable ) )
    {
      return false;
    }

    // Creature gets a chance to resist being affected
    if( SkillRoll( GetStrength() ) )
    {
      return false;
    }

    Damage( 16 + rand() % 32, null );

    // This creature can no longer be electrified during this chain
    io_ciImmunityTable[this] <- true;

    if( --io_iRecursionDepth > 0 )
    {
      // Electrify adjacent creatures
      local ciMap = GetMap();
      local ciPawnArray = ciMap.GetPawns( GetPosition(), 1, false );
      foreach( ciPawn in ciPawnArray )
      {
        if( ( ciPawn instanceof Creature ) && ciPawn != this )
        {
          ciPawn.Electrify( io_ciImmunityTable, io_iRecursionDepth );
        }
      }
    }

    return true;
  }


  function Freeze()
  {
    if( IsDead() || IsFrozen() || IsProtected() )
    {
      return false;
    }

    // Creatue gets a chance to resist being affected
    if( SkillRoll( GetStrength() ) )
    {
      return false;
    }

    SetProperty( Frozen_PropertyID, true );

    // Create the spell effect
    local ciEffect = Freeze_Effect();
    ciEffect.m_uiTargetID = GetID();

    // Put the effect in the target's effect table
    m_ciEffectsTable[ciEffect] <- ciEffect;

    // Give the creature a chance to break the effect at a later time, otherwise expire
    ::rumSchedule( ciEffect, ciEffect.Update, ciEffect.s_fInterval );
    ::rumSchedule( ciEffect, ciEffect.Expire, ciEffect.s_fDuration );

    if( IsBurning() )
    {
      // Remove the burning effect if it is active
      local ciEffect = GetEffect( Burn_Effect );
      if( ciEffect )
      {
        ciEffect.Remove();
      }
    }

    return true;
  }


  function GetActionDelay( i_eDelayType )
  {
    // Based on Dexterity values of 0 to 100, durations can be from (seconds):

    // Dex:      0      99
    // -----------------------
    // Short  =  0.2 to 0.1
    // Medium =  0.3 to 0.2
    // Long   =  0.4 to 0.3

    return i_eDelayType > ActionDelay.None ? ( 100.0 - GetDexterity() ) / 1000.0 + ( i_eDelayType / 10.0 ) : 0.0;
  }


  function GetAlignment()
  {
    return GetProperty( Alignment_PropertyID, AlignmentType.Good );
  }


  function GetAltWeaponType()
  {
    return rumInvalidAssetID;
  }


  function GetArmourType()
  {
    return rumInvalidAssetID;
  }


  function GetCharisma()
  {
    return GetProperty( Creature_Charisma_PropertyID, 15 );
  }


  function GetDexterity()
  {
    return GetProperty( Creature_Dexterity_PropertyID, 15 );
  }


  // Returns an effect instance in the effects table if one matching the input class is found
  function GetEffect( i_cEffect )
  {
    foreach( ciEffect in m_ciEffectsTable )
    {
      if( ciEffect instanceof i_cEffect )
      {
        return ciEffect;
      }
    }

    return null;
  }


  function GetHitpoints()
  {
    return GetProperty( Hitpoints_PropertyID, 1 );
  }


  function GetIntelligence()
  {
    return GetProperty( Creature_Intelligence_PropertyID, 15 );
  }


  function GetMana()
  {
    return GetProperty( Mana_PropertyID, 0 );
  }


  function GetMaxHitpoints()
  {
    return GetProperty( Max_Hitpoints_PropertyID, 99 );
  }


  function GetMaxMana()
  {
    return GetProperty( Max_Mana_PropertyID, 99 );
  }


  function GetStamina()
  {
    return GetProperty( Creature_Stamina_PropertyID, 15 );
  }


  function GetStrength()
  {
    return GetProperty( Creature_Strength_PropertyID, 15 );
  }


  function GetWeaponType()
  {
    return rumInvalidAssetID;
  }


  function GetWisdom()
  {
    return GetProperty( Creature_Wisdom_PropertyID, 15 );
  }


  // Returns true if the specified effect class is in the active effects table
  function HasEffect( i_eEffectID )
  {
    return GetEffect( i_eEffectID ) != null;
  }


  function Incapacitate()
  {
    if( IsDead() || IsIncapacitated() || IsProtected() )
    {
      return false;
    }

    if( SkillRoll( GetIntelligence() ) )
    {
      return false;
    }

    SetProperty( Unconscious_PropertyID, true );

    // Create the effect
    local ciEffect = Sleep_Effect();
    ciEffect.m_uiTargetID = GetID();

    // Put the effect in the target's effect table
    m_ciEffectsTable[ciEffect] <- ciEffect;

    // Give the creature a chance to break the effect at a later time, otherwise expire
    ::rumSchedule( ciEffect, ciEffect.Update, ciEffect.s_fInterval );
    ::rumSchedule( ciEffect, ciEffect.Expire, ciEffect.s_fDuration );

    return true;
  }


  function IsBurning()
  {
    return GetProperty( Burning_PropertyID, false );
  }


  function IsCamouflaged()
  {
    return false;
  }


  function IsCriminal()
  {
    return false;
  }


  function IsDaemonImmune()
  {
    return false;
  }


  function IsDead()
  {
    return GetHitpoints() < 1;
  }


  function IsFlying()
  {
    return false;
  }


  function IsFrozen()
  {
    return GetProperty( Frozen_PropertyID, false );
  }


  function IsIncapacitated()
  {
    return IsUnconscious() || IsFrozen();
  }


  function IsInvincible()
  {
    return GetProperty( Invincible_PropertyID, false );
  }


  function IsJailed()
  {
    return false;
  }


  function IsJinxed()
  {
    return GetProperty( Jinxed_PropertyID, false );
  }


  function IsNegated()
  {
    return GetProperty( Negated_PropertyID, false );
  }


  function IsPoisoned()
  {
    return GetProperty( Poisoned_PropertyID, false );
  }


  function IsPositionHarmful( i_ciPos )
  {
    local ciMap = GetMap();

    // Start with the base tile
    local ciPosData = ciMap.GetPositionData( i_ciPos );
    local ciTile = ::rumGetTileAsset( ciPosData.GetTileID() );
    if( ciTile == null )
    {
      return false;
    }

    local bHarmful = ciTile.GetProperty( Tile_Harmful_PropertyID, false );
    if( bHarmful )
    {
      local eMoveType = GetMoveType();
      local bImmune = true;

      local eTileID = ciTile.GetAssetID();
      switch( eTileID )
      {
        case U2_Marsh_TileID:
          bImmune = IsVenomous() || ResistsPoison() || IsUndead() ||
                    ( MoveType.Aerial == eMoveType ) || ( MoveType.Flies == eMoveType );
          break;

        case U3_Lava_TileID:
          bImmune = ResistsFire() || ( MoveType.Aerial == eMoveType );
          if( !bImmune && this instanceof Player )
          {
            local iFlags = GetProperty( U3_Marks_PropertyID, 0 );
            if( ::rumBitOn( iFlags, U3_MarkType.Fire ) )
            {
              bImmune = true;
            }
          }
          break;

        case U4_Lava_TileID:
          bImmune = ResistsFire() || ( MoveType.Aerial == eMoveType );
          break;

        case U4_Marsh_TileID:
          bImmune = IsVenomous() || ResistsPoison() || IsUndead() || ( MoveType.Aerial == eMoveType );
          break;
      }

      bHarmful = !bImmune;
    }

    if( !bHarmful )
    {
      // Check all widgets
      local ciWidget;
      while( ciWidget = ciPosData.GetNext( rumWidgetPawnType ) )
      {
        if( ciWidget.IsVisible() && ciWidget.IsHarmful( this ) )
        {
          return true;
        }
      }
    }

    return bHarmful;
  }


  function IsProtected()
  {
    return GetProperty( Protected_PropertyID, false );
  }


  function IsQuickened()
  {
    return GetProperty( Quickened_PropertyID, false );
  }


  function IsUnconscious()
  {
    return GetProperty( Unconscious_PropertyID, false );
  }


  function IsUndead()
  {
    return false;
  }


  function IsVenomous()
  {
    return false;
  }


  function Jinx()
  {
    if( IsDead() || IsIncapacitated() || IsJinxed() || IsProtected() )
    {
      return false;
    }

    if( SkillRoll( GetIntelligence() ) )
    {
      return false;
    }

    SetProperty( Jinxed_PropertyID, true );

    // Create the spell effect
    local ciEffect = Jinx_Effect();
    ciEffect.m_uiTargetID = GetID();

    // Put the effect in the target's effect table
    m_ciEffectsTable[ciEffect] <- ciEffect;

    // Give the creature a chance to break the effect at a later time, otherwise expire
    ::rumSchedule( ciEffect, ciEffect.Update, ciEffect.s_fInterval );
    ::rumSchedule( ciEffect, ciEffect.Expire, ciEffect.s_fDuration );

    return true;
  }


  function Kill( i_ciSource, i_ciWeapon = null, i_bSendClientEffect = true )
  {
    if( IsDead() || IsProtected() )
    {
      return false;
    }

    AffectHitpoints( -GetMaxHitpoints() );

    if( i_bSendClientEffect )
    {
      SendClientEffect( this, ClientEffectType.Damage );
    }

    if( i_ciSource instanceof Creature )
    {
      // Inform the attacker
      i_ciSource.OnKilled( this, true );
    }

    OnDeath( i_ciSource, true );

    if( i_ciSource )
    {
      print( "**** CREATURE KILLED BY " + i_ciSource.GetName() + "\n" );
    }

    return true;
  }


  function Negate( i_fDuration = Negate_Effect.s_fDuration )
  {
    if( IsDead() || IsIncapacitated() || IsProtected() )
    {
      return false;
    }

    SetProperty( Negated_PropertyID, true );

    // Create the spell effect
    local ciEffect = Negate_Effect();
    ciEffect.m_uiTargetID = GetID();

    // Put the effect in the target's effect table
    m_ciEffectsTable[ciEffect] <- ciEffect;

    // Give the creature a chance to break the effect at a later time, otherwise expire
    if( i_fDuration > ciEffect.s_fInterval )
    {
      ::rumSchedule( ciEffect, ciEffect.Update, ciEffect.s_fInterval );
    }

    ::rumSchedule( ciEffect, ciEffect.Expire, i_fDuration );

    return true;
  }


  // Called when this creature has been killed by something else (source)
  function OnDeath( i_ciSource, i_bKillCredit = true )
  {
    RemoveEffects();

    if( IsPoisoned() )
    {
      // Death cures poisoning!
      RemoveProperty( Poisoned_PropertyID );
    }
  }


  function OnInventoryAdded( i_ciItem )
  {
    //print( "Inventory item added: " + i_ciItem.GetName() + ", id [" + i_ciItem.GetID() + "]\n" );
  }


  function OnJinxEnd()
  {}


  // Called when this creature has killed something else (target)
  function OnKilled( i_ciTarget, i_bKillCredit = true )
  {
    // Base version does nothing
  }


  function OnObjectReleased()
  {
    // Release all affects since they potentially hold a reference to the creature
    m_ciEffectsTable.clear();
  }


  function OnPositionUpdated( i_ciNewPos, i_ciOldPos )
  {
    ApplyPositionEffects();
  }


  function OnPropertyRemoved( i_ePropertyID )
  {}


  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {}


  function OnSteal( i_ciPlayer )
  {}


  function OnWeaponThrown( i_ciWeapon, i_iDistance )
  {}


  function OnWeaponUsed( i_eWeaponType )
  {}


  function Poison( i_bForce = false )
  {
    if( IsDead() || ( !i_bForce && ( IsPoisoned() || IsProtected() ) ) )
    {
      return false;
    }

    // Creatue gets a chance to resist being affected
    if( !i_bForce && SkillRoll( GetDexterity() ) )
    {
      return false;
    }

    ApplyPoison();

    ++m_iPoisonIndex;

    // Create the poison effect
    local ciEffect = Poison_Effect();
    ciEffect.m_uiTargetID = GetID();

    // Put the effect in the target's effect table
    m_ciEffectsTable[ciEffect] <- ciEffect;

    // The effect does not expire, but damages the player on an interval
    ::rumSchedule( ciEffect, ciEffect.Update, ciEffect.s_fInterval, m_iPoisonIndex );

    return true;
  }


  function Protect()
  {
    if( IsDead() || IsProtected() )
    {
      return false;
    }

    SetProperty( Protected_PropertyID, true );

    // Create the spell effect
    local ciEffect = Protection_Effect();
    ciEffect.m_uiTargetID = GetID();

    // Put the effect in the target's effect table
    m_ciEffectsTable[ciEffect] <- ciEffect;

    // The spell expires after the specified duration
    ::rumSchedule( ciEffect, ciEffect.Expire, ciEffect.s_fDuration );

    return true;
  }


  function Quicken()
  {
    if( IsDead() || IsQuickened() )
    {
      return false;
    }

    SetProperty( Quickened_PropertyID, true );

    // Create the spell effect
    local ciEffect = Quickness_Effect();
    ciEffect.m_uiTargetID = GetID();

    // Put the effect in the target's effect table
    m_ciEffectsTable[ciEffect] <- ciEffect;

    // The spell expires after the specified duration
    ::rumSchedule( ciEffect, ciEffect.Expire, ciEffect.s_fDuration );

    return true;
  }


  function RemoveEffects()
  {
    if( IsBurning() )
    {
      RemoveProperty( Burning_PropertyID );
      local ciEffect = GetEffect( Burn_Effect );
      if( ciEffect != null )
      {
        ciEffect.Remove();
      }
    }

    if( IsFrozen() )
    {
      RemoveProperty( Frozen_PropertyID );
      local ciEffect = GetEffect( Freeze_Effect );
      if( ciEffect != null )
      {
        ciEffect.Remove();
      }
    }

    if( IsJinxed() )
    {
      RemoveProperty( Jinxed_PropertyID );
      local ciEffect = GetEffect( Jinx_Effect );
      if( ciEffect != null )
      {
        ciEffect.Remove();
      }
    }

    if( IsNegated() )
    {
      RemoveProperty( Negated_PropertyID );
      local ciEffect = GetEffect( Negate_Effect );
      if( ciEffect != null )
      {
        ciEffect.Remove();
      }
    }

    if( IsPoisoned() )
    {
      // Note: Poison is a permanent effect, so the property stays unchanged
      local ciEffect = GetEffect( Poison_Effect );
      if( ciEffect != null )
      {
        ciEffect.Remove();
      }
    }

    if( IsProtected() )
    {
      RemoveProperty( Protected_PropertyID );
      local ciEffect = GetEffect( Protection_Effect );
      if( ciEffect != null )
      {
        ciEffect.Remove();
      }
    }

    if( IsQuickened() )
    {
      RemoveProperty( Quickened_PropertyID );
      local ciEffect = GetEffect( Quickness_Effect );
      if( ciEffect != null )
      {
        ciEffect.Remove();
      }
    }

    if( IsUnconscious() )
    {
      RemoveProperty( Unconscious_PropertyID );
      local ciEffect = GetEffect( Sleep_Effect );
      if( ciEffect != null )
      {
        ciEffect.Remove();
      }
    }
  }


  function ResistsFire()
  {
    return false;
  }


  function ResistsLightning()
  {
    return false;
  }


  function ResistsPoison()
  {
    return false;
  }


  function ResistsSleep()
  {
    return false;
  }


  function ReApplyPositionEffects( i_iMoveIndex )
  {
    if( i_iMoveIndex == m_iMoveIndex )
    {
      ApplyPositionEffects();
    }
  }


  function Respawn()
  {
    SetVisibility( true );
  }


  function ScheduleRespawn()
  {
    local fRespawnTime = GetProperty( Respawn_Interval_PropertyID, 0.0 );
    if( fRespawnTime > 0.0 )
    {
      ::rumSchedule( this, Respawn, fRespawnTime );
    }
  }


  // This function returns true if a dice roll falls at or beneath the skill amount passed inot the function.
  function SkillRoll( i_iSkillAmount )
  {
    // Using 110 so that creatures should always have a chance of failing a skill roll with a maximum stat value of 99
    return ( rand() % 110 <= i_iSkillAmount );
  }


  function VulnerableToCold()
  {
    return GetProperty( Creature_Cold_Susceptible_PropertyID, true );
  }


  function VulnerableToFire()
  {
    return GetProperty( Creature_Fire_Susceptible_PropertyID, true );
  }
}
