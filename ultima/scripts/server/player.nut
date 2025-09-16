g_eConfiscatedGoldPropertyVersionArray <-
[
  null,
  U1_Confiscated_Coin_PropertyID,
  U2_Confiscated_Gold_PropertyID,
  U3_Confiscated_Gold_PropertyID,
  U4_Confiscated_Gold_PropertyID
]

g_eConfiscatedKeysPropertyVersionArray <-
[
  null,
  U1_Confiscated_Keys_PropertyID,
  U2_Confiscated_Keys_PropertyID,
  U3_Confiscated_Keys_PropertyID,
  U4_Confiscated_Keys_PropertyID
]

g_eNotorietyPropertyVersionArray <-
[
  null,
  U1_Notoriety_PropertyID,
  U2_Notoriety_PropertyID,
  U3_Notoriety_PropertyID,
  U4_Notoriety_PropertyID,
]

g_eTransportBoardedWidgetPropertyVersionArray <-
[
  null,
  U1_Transport_Boarded_Widget_PropertyID,
  U2_Transport_Boarded_Widget_PropertyID,
  U3_Transport_Boarded_Widget_PropertyID,
  U4_Transport_Boarded_Widget_PropertyID
]

g_eTransportCodePropertyVersionArray <-
[
  null,
  U1_Transport_Code_PropertyID,
  U2_Transport_Code_PropertyID,
  U3_Transport_Code_PropertyID,
  U4_Transport_Code_PropertyID
]

g_eTransportPosXPropertyVersionArray <-
[
  null,
  U1_Transport_PosX_PropertyID,
  U2_Transport_PosX_PropertyID,
  U3_Transport_PosX_PropertyID,
  U4_Transport_PosX_PropertyID
]

g_eTransportPosYPropertyVersionArray <-
[
  null,
  U1_Transport_PosY_PropertyID,
  U2_Transport_PosY_PropertyID,
  U3_Transport_PosY_PropertyID,
  U4_Transport_PosY_PropertyID
]


class Player extends Creature
{
  static s_eDefaultMoveType = MoveType.Terrestrial;

  static s_fCampingDuration = 30.0;
  static s_fManaRestoreInterval = 5.0;
  static s_fMealInterval = 30.0;
  static s_fNotorietyReductionInterval = 30.0;

  static s_iNotorietyCriminalLevel = 5;

  static s_fCombatInterval = 5.0;

  static s_fDeathInterval = 60.0;

  m_bCamping = false;
  m_iCampIndex = 0;

  m_iCombatIndex = 0;
  m_iDeathIndex = 0;
  m_iManaUpdateIndex = 0;
  m_iMealUpdateIndex = 0;
  m_iNotorietyIndex = 0;

  // TODO - this should be fetched from DB
  m_iLanguageID = english_LanguageID;

  // If the player is actively interacting with something, it will be set to the interacting target pawn ID
  m_uiInteractID = rumInvalidGameID;

  m_uiPartyID = 0;

  m_eArmourType = rumInvalidAssetID;
  m_eWeaponType = rumInvalidAssetID;


  constructor()
  {
    base.constructor();

    SetProperty( U4_Wind_Direction_Override_PropertyID, Direction.None );

    SetMoveType( MoveType.Terrestrial );
    SetCollisionFlags( MoveType.Terrestrial | MoveType.Aquatic | MoveType.Sails | MoveType.Stationary );
  }


  function ActionFailed( i_eStringID, i_bClientLookup = true )
  {
    local ciBroadcast = ::rumCreate( Command_Result_BroadcastID, i_eStringID, g_strColorTagArray.Red,
                                     i_bClientLookup );
    SendBroadcast( ciBroadcast );
  }


  function ActionInfo( i_eStringID, i_bClientLookup = true )
  {
    local ciBroadcast = ::rumCreate( Command_Result_BroadcastID, i_eStringID, g_strColorTagArray.Yellow,
                                     i_bClientLookup );
    SendBroadcast( ciBroadcast );
  }


  function ActionSuccess( i_eStringID, i_bClientLookup = true )
  {
    local ciBroadcast = ::rumCreate( Command_Result_BroadcastID, i_eStringID, g_strColorTagArray.Green,
                                     i_bClientLookup );
    SendBroadcast( ciBroadcast );
  }


  function ActionWarning( i_eStringID, i_bClientLookup = true )
  {
    local ciBroadcast = ::rumCreate( Command_Result_BroadcastID, i_eStringID, g_strColorTagArray.Magenta,
                                     i_bClientLookup );
    SendBroadcast( ciBroadcast );
  }


  function AddOrCreateItem( i_eItemAsset, i_iQuantity = 1 )
  {
    local bCreated = false;
    local bAdded = false;

    local ciAsset = ::rumGetInventoryAsset( i_eItemAsset );
    if( null == ciAsset )
    {
      return false;
    }

    local bStacks = ciAsset.GetProperty( Inventory_Stacks_PropertyID, false );

    local ciItem = GetInventoryByType( i_eItemAsset );
    if( !bStacks || ( null == ciItem ) )
    {
      // Create the item and add it to the player's inventory
      bCreated = AddItemAsset( i_eItemAsset );
      if( bCreated )
      {
        ciItem = GetInventoryByType( i_eItemAsset );
      }
    }

    if( bStacks && i_iQuantity >= 1 && ciItem != null )
    {
      // Increment the quantity
      local iNewCount = ciItem.GetProperty( Inventory_Quantity_PropertyID, 0 ) + i_iQuantity;
      ciItem.SetProperty( Inventory_Quantity_PropertyID, iNewCount );
      bAdded = true;
    }

    return bCreated || bAdded;
  }


  function AdjustVersionedProperty( i_ePropertyArray, i_vDelta )
  {
    local eVersion = GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
    local ePropertyID = i_ePropertyArray[eVersion];
    if( ePropertyID != null )
    {
      local varCurrent = GetProperty( ePropertyID, 0 );
      local varAdjusted = varCurrent + i_vDelta;
      SetProperty( ePropertyID, varAdjusted );
      return varAdjusted;
    }

    return null;
  }


  function AffectHitpoints( i_iDelta )
  {
    local eVersion = GetProperty( Ultima_Version_PropertyID, GameType.Invalid );
    local ePropertyID = g_eHitpointsPropertyVersionArray[eVersion];

    local iHitpoints = GetProperty( ePropertyID, 1 );
    local iMaxHitpoints = GetMaxHitpoints();
    local iNewHitpoints = iHitpoints + i_iDelta;
    iNewHitpoints = clamp( iNewHitpoints, 0, iMaxHitpoints );

    if( iHitpoints != iNewHitpoints )
    {
      SetProperty( ePropertyID, iNewHitpoints );
    }
  }


  function AffectVirtue( i_eVirtue, i_iDelta, i_bForce, i_bSendNotification )
  {
    local eVersion = GetProperty( Ultima_Version_PropertyID, GameType.Invalid );
    if( eVersion != GameType.Ultima4 )
    {
      return;
    }

    local ePropertyID = g_eU4VirtuePropertyArray[i_eVirtue];
    local iLevel = GetProperty( ePropertyID, 0 );
    local iAdjusted = iLevel + i_iDelta;

    local ciMap = GetMap();
    local iMapID = ciMap.GetAssetID();

    local iLastMapID = GetProperty( U4_Last_Incremented_Virtue_Map_PropertyID, -1 );
    local eLastVirtue = GetProperty( U4_Last_Incremented_Virtue_PropertyID, -1 );

    // If this virtue is the last virtue that was modified, or if the player's map has not changed since the last
    // modification, then do not affect the virtue (unless the change is forced)
    if( ( ( eLastVirtue == i_eVirtue ) || ( iLastMapID == iMapID ) ) && !i_bForce )
    {
      return;
    }

    if( iLevel != iAdjusted )
    {
      // Adjust the virtue level and remember which virtue was just incremented
      SetProperty( ePropertyID, iAdjusted );

      if( i_iDelta > 0 )
      {
        // Remember the last incremented virtue and the map it was incremented on. This is used to prevent
        // players from rapidly increasing a virtue level
        SetProperty( U4_Last_Incremented_Virtue_PropertyID, i_eVirtue );
        SetProperty( U4_Last_Incremented_Virtue_Map_PropertyID, iMapID );
      }

      // Has the player just lost an eighth?
      if( iLevel == 100 && iAdjusted < 100 )
      {
        // Give player the bad news
        ActionFailed( msg_lost_eighth_client_StringID );
        local iFlags = GetProperty( U4_Virtue_Elevation_PropertyID, 0 );
        ::rumBitClear( iFlags, i_eVirtue );
        SetProperty( U4_Virtue_Elevation_PropertyID, iFlags );
      }

      if( iLevel < iAdjusted )
      {
        // Clear Seer Hawkwind elevation permission for the affected virtue
        local iFlags = GetProperty( U4_Seer_Bestowals_PropertyID, 0 );
        ::rumBitClear( iFlags, i_eVirtue );
        SetProperty( U4_Seer_Bestowals_PropertyID, iFlags );
      }

      if( i_bSendNotification )
      {
        if( iLevel < iAdjusted )
        {
          ActionSuccess( msg_more_virtuous_client_StringID );
        }
        else
        {
          ActionFailed( msg_less_virtuous_client_StringID );
        }
      }
    }
  }


  function ApplyPoison()
  {
    SetVersionedProperty( g_ePoisonedPropertyVersionArray, true );
  }


  function ApplyPositionEffects()
  {
    if( !IsFlying() )
    {
      base.ApplyPositionEffects();
    }
  }


  function Attack( i_ciTarget, i_vValue )
  {
    CombatBegin();

    local ciMap = GetMap();
    if( ciMap.GetAssetID() == U4_Shrine_Humility_MapID )
    {
      // Silver Horn Daemon immunity is lost if the player attacks anything
      RemoveProperty( U4_Shrine_Daemon_Immunity_PropertyID )
    }

    local eResult = base.Attack( i_ciTarget, i_vValue );
    switch( eResult )
    {
      case AttackReturnType.Success:
        if( ( i_ciTarget instanceof Creature ) && i_ciTarget.IsDead() )
        {
          i_ciTarget = null;
        }
        break;

      case AttackReturnType.Failed_InvalidTarget:
      case AttackReturnType.Failed_LOS:
        ActionFailed( msg_not_here_client_StringID );
        break;

      case AttackReturnType.Failed_OutOfRange:
        ActionFailed( msg_out_of_range_client_StringID );
        break;

      case AttackReturnType.Failed_Collision:
        ActionFailed( msg_blocked_client_StringID );
        break;
    }

    return eResult;
  }


  function AttemptSteal( i_ciCreature, i_iModifier = 0, i_bForce = false )
  {
    if( null == i_ciCreature )
    {
      return false;
    }

    if( PostureType.Attack == i_ciCreature.m_eDefaultPosture )
    {
      ActionFailed( msg_no_effect_client_StringID );
      return false;
    }

    local ciMap = GetMap();
    if( !ciMap.IsPositionWithinTileDistance( i_ciCreature.GetPosition(), GetPosition(), 2 ) )
    {
      // Player not within acceptable range
      return false;
    }

    if( IsThief() )
    {
      // Difficulty is mitigated for thief classes
      i_iModifier /= 4;
    }

    local iDexterity = GetDexterity();
    local iDifficulty = 100 + i_iModifier;

    // Was the theft successful?
    local bSuccess = i_bForce || ( rand() % iDifficulty < iDexterity );
    if( !bSuccess )
    {
      ActionFailed( msg_caught_client_StringID );
      ActionFailed( msg_thief_client_StringID );
      AdjustVersionedProperty( g_eNotorietyPropertyVersionArray, 4 );

      // Alert guards
      if( i_ciCreature instanceof NPC )
      {
        i_ciCreature.CallGuards( this );
      }
    }

    return bSuccess;
  }


  function CalculateMaxMana( i_eVersion )
  {
    local uiMaxMana = 0;

    if( GameType.Ultima4 == i_eVersion )
    {
      /*-------------------\
      | Class    | Mod     |
       --------------------
      | Mage     | 2.0 Int |
      | Bard     | 1.0 Int |
      | Fighter  | 0.0     |
      | Druid    | 1.5 Int |
      | Tinker   | 0.5     |
      | Paladin  | 1.0 Int |
      | Ranger   | 1.0 Int |
      | Shepherd | 0.0     |
      | Avatar   | 2.0 Int |
      \-------------------*/

      local ePlayerClassID = GetProperty( U4_PlayerClass_PropertyID, U4_Mage_Class_CustomID );
      local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );
      local bCastsSpells = ciPlayerClass.GetProperty( Class_Casts_Spells_PropertyID, false );
      if( bCastsSpells )
      {
        local fManaModifier = ciPlayerClass.GetProperty( Class_Mana_Modifier_PropertyID, 0.0 );
        local iIntelligence = GetProperty( U4_Intelligence_PropertyID, 1 );
        uiMaxMana = iIntelligence * fManaModifier;
        uiMaxMana = uiMaxMana.tointeger();
      }
    }
    else if( GameType.Ultima3 == i_eVersion )
    {
      /*------------------------------\
      | Class       | Mod             |
       -------------------------------
      | Fighter     | 0.0             |
      | Cleric      | 1.0 Wis         |
      | Wizard      | 1.0 Int         |
      | Thief       | 0.0             |
      | Paladin     | 0.5 Wis         |
      | Barbarian   | 0.0             |
      | Lark        | 0.5 Int         |
      | Illusionist | 0.5 Wis         |
      | Druid       | 0.5 Int\Wis Max |
      | Alchemist   | 0.5 Int         |
      | Ranger      | 0.5 Int\Wis Min |
      \------------------------------*/

      local ePlayerClassID = GetProperty( U3_PlayerClass_PropertyID, U3_Fighter_Class_CustomID );
      local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );
      local bCastsSpells = ciPlayerClass.GetProperty( Class_Casts_Spells_PropertyID, false );
      if( bCastsSpells )
      {
        local fManaModifier = ciPlayerClass.GetProperty( Class_Mana_Modifier_PropertyID, 0.0 );

        if( U3_Druid_Class_CustomID == ePlayerClassID )
        {
          local iIntelligence = GetProperty( U3_Intelligence_PropertyID, 1 );
          local iWisdom = GetProperty( U3_Wisdom_PropertyID, 1 );
          local iStat = max( iIntelligence, iWisdom );
          uiMaxMana = iStat * fManaModifier;
          uiMaxMana = uiMaxMana.tointeger();
        }
        else if( U3_Ranger_Class_CustomID == ePlayerClassID )
        {
          local iIntelligence = GetProperty( U3_Intelligence_PropertyID, 1 );
          local iWisdom = GetProperty( U3_Wisdom_PropertyID, 1 );
          local iStat = min( iIntelligence, iWisdom );
          uiMaxMana = iStat * fManaModifier;
          uiMaxMana = uiMaxMana.tointeger();
        }
        else
        {
          local ePropertyID = ciPlayerClass.GetProperty( Class_Mana_Source_ID_PropertyID,
                                                         U3_Intelligence_PropertyID );
          local iStat = GetProperty( ePropertyID, 1 );
          uiMaxMana = iStat * fManaModifier;
          uiMaxMana = uiMaxMana.tointeger();
        }
      }
    }
    else if( GameType.Ultima2 == i_eVersion )
    {
      /*----------------------\
      | Class       | Mod     |
       -----------------------
      | Fighter     | 0.0     |
      | Cleric      | 1.0 Wis |
      | Wizard      | 1.0 Int |
      | Thief       | 0.0     |
      \----------------------*/

      local ePlayerClassID = GetProperty( U2_PlayerClass_PropertyID, U2_Fighter_Class_CustomID );
      local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );
      local bCastsSpells = ciPlayerClass.GetProperty( Class_Casts_Spells_PropertyID, false );
      if( bCastsSpells )
      {
        local fManaModifier = ciPlayerClass.GetProperty( Class_Mana_Modifier_PropertyID, 0.0 );

        local ePropertyID = ciPlayerClass.GetProperty( Class_Mana_Source_ID_PropertyID,
                                                       U2_Intelligence_PropertyID );
        local iStat = GetProperty( ePropertyID, 1 );

        uiMaxMana = iStat * fManaModifier;
        uiMaxMana = uiMaxMana.tointeger();
      }
    }
    else if( GameType.Ultima1 == i_eVersion )
    {
      /*----------------------\
      | Class       | Mod     |
       -----------------------
      | Fighter     | 0.0     |
      | Cleric      | 1.0 Wis |
      | Wizard      | 1.0 Int |
      | Thief       | 0.0     |
      \----------------------*/

      local ePlayerClassID = GetProperty( U1_PlayerClass_PropertyID, U1_Fighter_Class_CustomID );
      local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );
      local bCastsSpells = ciPlayerClass.GetProperty( Class_Casts_Spells_PropertyID, false );
      if( bCastsSpells )
      {
        local fManaModifier = ciPlayerClass.GetProperty( Class_Mana_Modifier_PropertyID, 0.0 );

        local ePropertyID = ciPlayerClass.GetProperty( Class_Mana_Source_ID_PropertyID,
                                                       U1_Intelligence_PropertyID );
        local iStat = GetProperty( ePropertyID, 1 );

        uiMaxMana = iStat * fManaModifier;
        uiMaxMana = uiMaxMana.tointeger();
      }
    }

    return uiMaxMana;
  }


  function Camp()
  {
    // The player sleeps for 30 seconds - during this time the player will not be able
    // to move or do anything unless wakened by another player
    ActionSuccess( msg_resting_client_StringID );

    // Prevent the player from moving
    SetProperty( Unconscious_PropertyID, true );

    // Create the effect
    local ciEffect = Sleep_Effect();
    ciEffect.m_uiTargetID = GetID();

    // Put the effect in the target's effect table
    m_ciEffectsTable[ciEffect] <- ciEffect;

    ++m_iCampIndex;
    m_bCamping = true;

    // Schedule a wakeup for the player
    ::rumSchedule( this, OnCampingDone, s_fCampingDuration, m_iCampIndex );
  }


  function ChangeWorld( i_eVersion )
  {
    if( i_eVersion < GameType.Ultima1 || i_eVersion > GameType.Ultima4 )
    {
      // Invalid version
      return;
    }

    local eCurrentVersion = GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
    if( i_eVersion == eCurrentVersion )
    {
      // Player is already on the requested version
      return;
    }

    // If the player is already on a transport, exit that transport. This should really only happen to admins or via
    // admin intervention.
    local ciTransport = GetTransport();
    if( ciTransport != null && !ciTransport.Exit( this, GetPosition(), MoveType.Incorporeal ) )
    {
      ActionFailed( msg_only_on_foot_client_StringID );
      return;
    }

    SetProperty( Character_Prep_PropertyID, true );

    local ciPos = rumPos( 0, 0 );

    // Determine the player's position for the new version
    local eMapID = GetProperty( g_eLastMapPropertyVersionArray[i_eVersion], rumInvalidAssetID );
    if( rumInvalidAssetID == eMapID )
    {
      // This is the first time the player is visiting this Ultima version, so determine the starting point
      eMapID = g_eWorldStartMapArray[i_eVersion];
      local ciMap = ::rumGetMapAsset( eMapID );
      ciPos.x = ciMap.GetProperty( Class_Starting_Pos_X_PropertyID, 0 );
      ciPos.y = ciMap.GetProperty( Class_Starting_Pos_Y_PropertyID, 0 );
    }
    else
    {
      // Player is re-visiting, so use the previous map and position
      ciPos.x = GetProperty( g_eLastPosXPropertyVersionArray[i_eVersion], 0 );
      ciPos.y = GetProperty( g_eLastPosYPropertyVersionArray[i_eVersion], 0 );
    }

    // Save the current map and position
    local ciPrevMap = GetMap();
    local ciLastPos = GetPosition();

    // Transfer the player to the specified map and position
    local ciDestMap = GetOrCreateMap( this, eMapID );
    if( ciDestMap != null && ciPrevMap.TransferPawn( this, ciDestMap, ciPos ) )
    {
      SetProperty( g_eLastMapPropertyVersionArray[eCurrentVersion], ciPrevMap.GetAssetID() );
      SetProperty( g_eLastPosXPropertyVersionArray[eCurrentVersion], ciLastPos.x );
      SetProperty( g_eLastPosYPropertyVersionArray[eCurrentVersion], ciLastPos.y );

      // Remove all active effects on the player
      RemoveEffects();

      HandleWorldUpdate( i_eVersion );
    }

    RemoveProperty( Character_Prep_PropertyID );
  }


  function ClearWearyMind()
  {
    local iNumCycles = GetProperty( U4_Weary_Mind_Cycle_Count_PropertyID, 0 ) - 1;
    if( iNumCycles > 0 )
    {
      SetProperty( U4_Weary_Mind_Cycle_Count_PropertyID, iNumCycles );
    }
    else
    {
      // Player is free to meditate again
      RemoveProperty( U4_Weary_Mind_Cycle_Count_PropertyID );
    }
  }


  function CombatBegin()
  {
    ++m_iCombatIndex;
    SetProperty( Combat_PropertyID, true );
    ::rumSchedule( this, CombatEnd, s_fCombatInterval, m_iCombatIndex );
  }


  function CombatEnd( i_iCombatIndex )
  {
    if( i_iCombatIndex == m_iCombatIndex )
    {
      RemoveProperty( Combat_PropertyID );
    }
  }


  function ConsumeMana( i_ciSpell )
  {
    local iSpellMana = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_Mana_Cost_PropertyID, 0 ) : 0;
    local iPlayerMana = GetMana();
    if( iSpellMana > 0 && iPlayerMana >= iSpellMana )
    {
      AdjustVersionedProperty( g_eManaPropertyVersionArray, -iSpellMana );
      return true;
    }

    return false;
  }


  function ConsumeSpell( i_ciSpell )
  {
    local ePropertyID = i_ciSpell != null ? i_ciSpell.GetProperty( Spell_ID_PropertyID, rumInvalidAssetID )
                                          : rumInvalidAssetID;
    local iNumSpells = GetProperty( ePropertyID, 0 );
    if( iNumSpells > 0 && ePropertyID != rumInvalidAssetID )
    {
      SetProperty( ePropertyID, iNumSpells - 1 );
      return true;
    }

    return false;
  }


  function Cure()
  {
    RemoveVersionedProperty( g_ePoisonedPropertyVersionArray );
  }


  function Damage( i_iAmount, i_ciSource, i_eWeaponType = rumInvalidAssetID, i_bSendClientEffect = true )
  {
    base.Damage( i_iAmount, i_ciSource, i_eWeaponType, i_bSendClientEffect );

    if( !IsDead() && ( i_ciSource instanceof Creature ) )
    {
      StopDialogue( DialogueTerminationType.Interrupted );
      CombatBegin();
    }
  }


  function DiscoverReagent( i_eReagentType )
  {
    local ePropertyID = g_eU4ReagentPropertyArray[i_eReagentType];

    // Player can find up to 9 components, with a minimum of 5
    SetProperty( ePropertyID, GetProperty( ePropertyID, 0 ) + rand() % 5 + 5 );

    // Don't allow the player to find anymore reagents for this moon cycle
    SetProperty( Reagent_Discovery_Blocked_PropertyID, true );

    if( Reagents.Mandrake_Root == i_eReagentType )
    {
      ActionInfo( msg_mandrake_client_StringID );
    }
    else if( Reagents.Nightshade == i_eReagentType )
    {
      ActionInfo( msg_nightshade_client_StringID );
    }
  }


  function Electrify( io_ciImmunityTable, io_iRecursionDepth )
  {
    if( base.Electrify( io_ciImmunityTable, io_iRecursionDepth ) )
    {
      ActionWarning( msg_electrified_client_StringID );
    }
  }


  // Internal equipping used by inventory restoration from the database and during world switching. This version of
  // equip bypasses a lot of checks because the player was already able to equip this item previously.
  function Equip( i_ciItem )
  {
    if( null == i_ciItem )
    {
      return;
    }

    local bEquipped = i_ciItem.GetProperty( Inventory_Equipped_PropertyID, false );
    if( !bEquipped )
    {
      return;
    }

    local ePlayerVersion = GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
    local eItemVersion = i_ciItem.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
    if( eItemVersion != ePlayerVersion )
    {
      return;
    }

    local eInventoryType = i_ciItem.GetProperty( Inventory_Type_PropertyID, InventoryType.Standard );
    if( InventoryType.Armour == eInventoryType )
    {
      SetProperty( Equipped_Armour_PropertyID, i_ciItem.GetID() );
      m_eArmourType = i_ciItem.GetAssetID();
    }
    else if( InventoryType.Weapon == eInventoryType )
    {
      SetProperty( Equipped_Weapon_PropertyID, i_ciItem.GetID() );
      m_eWeaponType = i_ciItem.GetAssetID();
    }
  }


  function EquipArmour( i_uiArmourID )
  {
    // TODO - early out if the requested armour is somehow already worn?

    RemoveArmour();

    // If the armour is invalid, it could be that the player is just unequipping
    if( rumInvalidGameID == i_uiArmourID )
    {
      return;
    }

    // The player wants to equip armour, so fetch it from inventory
    local ciArmour = GetInventory( i_uiArmourID );
    if( null == ciArmour || !( ciArmour instanceof rumInventory ) )
    {
      // The client should've caught the fact that this isn't valid inventory
      IncrementHackAttempts();
      return;
    }

    // Check that this is actually armour
    local eInventoryType = ciArmour.GetProperty( Inventory_Type_PropertyID, InventoryType.Standard );
    if( eInventoryType != InventoryType.Armour )
    {
      // The client should've caught the fact that this isn't armour
      IncrementHackAttempts();
      return;
    }

    local ePlayerVersion = GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
    local eArmourVersion = ciArmour.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
    if( ePlayerVersion != eArmourVersion )
    {
      // The client should've caught the version mismatch
      IncrementHackAttempts();
      return;
    }

    // Check that the armour is compatible with the player
    local ciPlayerClass = GetPlayerClass();
    local uiClassFlag = 1 << ciPlayerClass.GetProperty( Class_ID_PropertyID, 0 );
    local uiCompatibilityFlags = ciArmour.GetProperty( Inventory_Class_Compatibility_Flags_PropertyID, 0 );
    if( ( uiClassFlag & uiCompatibilityFlags ) == 0 )
    {
      // The client should've caught the fact that this armour is incompatible with the player's class
      IncrementHackAttempts();
      return;
    }

    local uiRequiredStrength = ciArmour.GetProperty( Inventory_Strength_Min_PropertyID, 0 );
    local uiStrength = uiRequiredStrength;

    // Check stat compatibility
    if( GameType.Ultima2 == eArmourVersion )
    {
      uiStrength = GetProperty( U2_Strength_PropertyID, 0 );
    }

    if( uiStrength < uiRequiredStrength )
    {
      // The client should've caught the stat requirement
      IncrementHackAttempts();
      return;
    }

    ciArmour.SetProperty( Inventory_Equipped_PropertyID, true );
    SetProperty( Equipped_Armour_PropertyID, i_uiArmourID );
    m_eArmourType = ciArmour.GetAssetID();
  }


  function EquipWeapon( i_uiWeaponID )
  {
    // TODO - early out if the requested weapon is already equipped?

    RemoveWeapon();

    // If the weapon is invalid, it could be that the player is just unequipping
    if( rumInvalidGameID == i_uiWeaponID )
    {
      return;
    }

    // The player wants to equip a weapon, so fetch it from inventory
    local ciWeapon = GetInventory( i_uiWeaponID );
    if( null == ciWeapon || !( ciWeapon instanceof rumInventory ) )
    {
      // The client should've caught the fact that this isn't valid inventory
      IncrementHackAttempts();
      return;
    }

    // Check that this is actually a weapon
    local eInventoryType = ciWeapon.GetProperty( Inventory_Type_PropertyID, InventoryType.Standard );
    if( eInventoryType != InventoryType.Weapon )
    {
      // The client should've caught the fact that this isn't a weapon
      IncrementHackAttempts();
      return;
    }

    local ePlayerVersion = GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
    local eWeaponVersion = ciWeapon.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
    if( ePlayerVersion != eWeaponVersion )
    {
      // The client should've caught the version mismatch
      IncrementHackAttempts();
      return;
    }

    // Check that the weapon is compatible with the player
    local ciPlayerClass = GetPlayerClass();
    local uiClassFlag = 1 << ciPlayerClass.GetProperty( Class_ID_PropertyID, 0 );
    local uiCompatibilityFlags = ciWeapon.GetProperty( Inventory_Class_Compatibility_Flags_PropertyID, 0 );
    if( ( uiClassFlag & uiCompatibilityFlags ) == 0 )
    {
      // The client should've caught the fact that the weapon is incompatible with the player's class
      IncrementHackAttempts();
      return;
    }

    local bTorchLit = false;

    // A non-magical light effect means that the player has a torch lit
    local ciEffect = GetEffect( Light_Effect );
    if( ciEffect && !ciEffect.m_bMagical )
    {
      bTorchLit = true;
    }

    // Make sure the player has a free hand for weapons requiring more than one hand
    local uiRequiredHands = ciWeapon.GetProperty( Inventory_Weapon_Num_Hands_PropertyID, 1 );
    if( uiRequiredHands > 1 && bTorchLit )
    {
      ActionFailed( msg_hands_full_client_StringID );
      return;
    }

    local uiRequiredAgility = ciWeapon.GetProperty( Inventory_Agility_Min_PropertyID, 0 );
    local uiAgility = uiRequiredAgility;

    // Check stat compatibility
    if( GameType.Ultima1 == eWeaponVersion )
    {
      uiAgility = GetProperty( U1_Agility_PropertyID, 0 );
    }
    else if( GameType.Ultima2 == eWeaponVersion )
    {
      uiAgility = GetProperty( U2_Agility_PropertyID, 0 );
    }

    if( uiAgility < uiRequiredAgility )
    {
      // The client should've caught the stat requirement
      IncrementHackAttempts();
      return;
    }

    ciWeapon.SetProperty( Inventory_Equipped_PropertyID, true );
    SetProperty( Equipped_Weapon_PropertyID, i_uiWeaponID );
    m_eWeaponType = ciWeapon.GetAssetID();
  }


  function ExtinguishLight()
  {
    local ciEffect = GetEffect( Light_Effect );
    if( ciEffect instanceof Light_Effect )
    {
      ciEffect.Expire();
    }
  }


  function GetAlignment()
  {
    return IsCriminal() ? AlignmentType.Evil : AlignmentType.Good;
  }


  function GetArmourType()
  {
    if( rumInvalidAssetID == m_eArmourType )
    {
      return base.GetArmourType();
    }

    return m_eArmourType;
  }


  function GetCharisma()
  {
    return GetVersionedProperty( g_eCharismaPropertyVersionArray, null );
  }


  function GetDexterity()
  {
    return GetVersionedProperty( g_eDexterityPropertyVersionArray, null );
  }


  function GetDiscountPercent()
  {
    // At 99 Charisma points, players can expect about a 40% discount on all merchant transactions
    return GetCharisma() * 0.004;
  }


  function GetEquippedArmour()
  {
    local uiArmourID = GetEquippedArmourID();
    if( uiArmourID != rumInvalidGameID )
    {
      return GetInventory( uiArmourID );
    }

    return null;
  }


  function GetEquippedArmourID()
  {
    return GetProperty( Equipped_Armour_PropertyID, rumInvalidGameID );
  }


  function GetEquippedWeapon()
  {
    local uiWeaponID = GetEquippedWeaponID();
    if( uiWeaponID != rumInvalidGameID )
    {
      return GetInventory( uiWeaponID );
    }

    return null;
  }


  function GetEquippedWeaponID()
  {
    return GetProperty( Equipped_Weapon_PropertyID, rumInvalidGameID );
  }


  function GetExperienceLevel()
  {
    // The level the player can attain with the current amount of experience points. This may differ from the
    // actual player level persisted to the database where a player has enough experience for the next level, but
    // has not yet been awarded the new level by Lord British (Ultima 3 & 4).

    local iExpLevel = 0;

    local eCurrentVersion = GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
    if( GameType.Ultima4 == eCurrentVersion )
    {
      /*-------------\
      | Lvl | XP Req |
      --------------
      | 1   | 0      |
      | 2   | 100    |
      | 3   | 200    |
      | 4   | 400    |
      | 5   | 800    |
      | 6   | 1600   |
      | 7   | 3200   |
      | 8   | 6400   |
      \-------------*/

      // The player's current experience points
      local iExp = GetProperty( U4_Experience_PropertyID, 0 );
      if( iExp < 100 )
      {
        iExpLevel = 1;
      }
      else
      {
        local fExpLevel = log( iExp / 100.0 ) / log( 2.0 ) + 2.0;
        iExpLevel = fExpLevel.tointeger();
      }
    }
    else if( GameType.Ultima3 == eCurrentVersion )
    {
      // This deviates slightly from the original U3... Level 2 requires 100 exp, and each additional level
      // requires 25 exp more than the previous level. So, level 3 requires 125 + 100 = 225. Level 4 requires
      // 150 + 225 = 375:

      /*-------------\
      | Lvl | XP Req |
      --------------
      | 1   | 0      |
      | 2   | 100    |
      | 3   | 225    |
      | 4   | 375    |
      | 5   | 550    |
      | ... | ...    |
      | 24  | 8625   |
      | 25  | 9300   |
      \-------------*/

      iExpLevel = 1;

      // The player's current experience points
      local iExp = GetProperty( U3_Experience_PropertyID, 0 );
      if( iExp >= 100 )
      {
        local iLastNeeded = 0;
        local iMaxLevel = ::rumGetMaxPropertyValue( U3_Level_PropertyID );
        for( local iLevel = 2; iLevel <= iMaxLevel; ++iLevel )
        {
          local iNeeded = ( iLevel - 2 ) * 25 + 100 + iLastNeeded;
          if( iExp >= iNeeded )
          {
            iExpLevel = iLevel;
            iLastNeeded = iNeeded;
          }
          else
          {
            break;
          }
        }
      }

      return iExpLevel;
    }
    else if( GameType.Ultima2 == eCurrentVersion )
    {
      local iExp = GetProperty( U2_Experience_PropertyID, 0 );
      local fExpLevel = iExp / 100 + 1;
      iExpLevel = fExpLevel.tointeger();
    }
    else if( GameType.Ultima1 == eCurrentVersion )
    {
      local iExp = GetProperty( U1_Experience_PropertyID, 0 );
      local fExpLevel = iExp / 1000 + 1;
      iExpLevel = fExpLevel.tointeger();
    }

    return iExpLevel;
  }


  function GetHitpoints()
  {
    return GetVersionedProperty( g_eHitpointsPropertyVersionArray, 1 );
  }


  function GetIntelligence()
  {
    return GetVersionedProperty( g_eIntelligencePropertyVersionArray, null );
  }


  function GetInventoryByType( i_eAssetType )
  {
    local ciInventory = GetInventory();
    local ciItem;
    while( ciItem = ciInventory.GetNextObject() )
    {
      if( ciItem.GetAssetID() == i_eAssetType )
      {
        ciInventory.Stop();
        return ciItem;
      }
    }

    return null;
  }


  function GetMana()
  {
    return GetVersionedProperty( g_eManaPropertyVersionArray );
  }


  function GetMaxHitpoints()
  {
    local iMaxHitpoints = 0;

    local eVersion = GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
    if( GameType.Ultima4 == eVersion )
    {
      local iLevel = GetProperty( U4_Level_PropertyID, 1 );
      iMaxHitpoints = iLevel * 100;
    }
    else if( GameType.Ultima3 == eVersion )
    {
      local iLevel = GetProperty( U3_Level_PropertyID, 1 );
      iMaxHitpoints = iLevel * 100 + 50;
    }
    else if( GameType.Ultima2 == eVersion )
    {
      // Unlike in the original, hitpoints cannot exceed experience cap for each level, or 400 at a minimum
      local iExp = GetProperty( U2_Experience_PropertyID, 0 );
      local iLevel = iExp / 100 + 1;
      iMaxHitpoints = max( iLevel * 100, 400 );
      iMaxHitpoints = min( iMaxHitpoints, 9999 );
    }
    else if( GameType.Ultima1 == eVersion )
    {
      // Unlike in the original, hitpoints cannot exceed experience cap for each level, or 999 at a minimum
      local iExp = GetProperty( U1_Experience_PropertyID, 0 );
      local iLevel = iExp / 1000 + 1;
      iMaxHitpoints = max( iLevel * 1000, 999 );
      iMaxHitpoints = min( iMaxHitpoints, 9999 );
    }

    return iMaxHitpoints;
  }


  function GetParty()
  {
    if( m_uiPartyID != 0 && ( m_uiPartyID in g_ciServer.m_uiPartyTable ) )
    {
      return g_ciServer.m_uiPartyTable[ m_uiPartyID ];
    }

    return null;
  }


  function GetPartyID()
  {
    return m_uiPartyID;
  }


  function GetPlayerClass()
  {
    local ePlayerClassID = GetVersionedProperty( g_ePlayerClassPropertyVersionArray, rumInvalidAssetID );
    return( ePlayerClassID != rumInvalidAssetID ? ::rumGetCustomAsset( ePlayerClassID ) : null );
  }


  function GetPrivateInstanceID()
  {
    local ciParty = GetParty();
    if( ciParty != null )
    {
      // Use the party ID
      return ciParty.m_iID;
    }
    else
    {
      // Use the player's ID
      return GetID();
    }
  }


  function GetStamina()
  {
    return GetVersionedProperty( g_eStaminaPropertyVersionArray, null );
  }


  function GetStatCap( i_ePropertyID )
  {
    local ciPlayerClass = GetPlayerClass();
    local eCapPropertyID = rumInvalidAssetID;

    switch( i_ePropertyID )
    {
      case U4_Strength_PropertyID:
      case U3_Strength_PropertyID:
      case U2_Strength_PropertyID:
      case U1_Strength_PropertyID:
        eCapPropertyID = Class_Strength_Cap_PropertyID;
        break;

      case U4_Dexterity_PropertyID:
      case U3_Dexterity_PropertyID:
      case U2_Agility_PropertyID:
      case U1_Agility_PropertyID:
        eCapPropertyID = Class_Dexterity_Cap_PropertyID;
        break;

      case U4_Intelligence_PropertyID:
      case U3_Intelligence_PropertyID:
      case U2_Intelligence_PropertyID:
      case U1_Intelligence_PropertyID:
        eCapPropertyID = Class_Intelligence_Cap_PropertyID;
        break;

      case U3_Wisdom_PropertyID:
      case U2_Wisdom_PropertyID:
      case U1_Wisdom_PropertyID:
        eCapPropertyID = Class_Wisdom_Cap_PropertyID;
        break;

      case U2_Stamina_PropertyID:
      case U1_Stamina_PropertyID:
        eCapPropertyID = Class_Stamina_Cap_PropertyID;
        break;

      case U2_Charisma_PropertyID:
      case U1_Charisma_PropertyID:
        eCapPropertyID = Class_Charisma_Cap_PropertyID;
        break;
    }

    return ciPlayerClass.GetProperty( eCapPropertyID, 99 );
  }


  function GetStrength()
  {
    return GetVersionedProperty( g_eStrengthPropertyVersionArray, null );
  }


  function GetTransport()
  {
    local uiTransportID = GetTransportID();
    return uiTransportID != rumInvalidGameID ? ::rumFetchPawn( uiTransportID ) : null;
  }


  function GetTransportID()
  {
    return GetVersionedProperty( g_eTransportIDPropertyVersionArray, rumInvalidGameID );
  }


  function GetVersionedProperty( i_ePropertyArray, i_vDefault = 0 )
  {
    local eVersion = GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
    local ePropertyID = i_ePropertyArray[eVersion];
    return ePropertyID != null ? GetProperty( ePropertyID, i_vDefault ) : i_vDefault;
  }


  function GetWeaponType()
  {
    if( rumInvalidAssetID == m_eWeaponType )
    {
      return base.GetWeaponType();
    }

    return m_eWeaponType;
  }


  function GetWisdom()
  {
    return GetVersionedProperty( g_eWisdomPropertyVersionArray, null );
  }


  function HandleWorldUpdate( i_eVersion )
  {
    // Player leaves the party
    local ciParty = GetParty();
    if( ciParty != null )
    {
      ciParty.Dismiss( this, PartyBroadcastType.PlayerLeft );
    }

    // Update the version - this will kick off a series of events on the client to update player state
    SetProperty( Ultima_Version_PropertyID, i_eVersion );

    // Update max mana before setting mana
    SetProperty( Max_Mana_PropertyID, CalculateMaxMana( i_eVersion ) );

    if( IsDead() )
    {
      // Resurrect the player from the void
      Resurrect( ResurrectionType.Void );
    }
    else if( IsPoisoned() )
    {
      // Re-apply poison effect
      Poison( true );
    }

    // Re-equip the player's weapons and armour
    local ciInventory = GetInventory();
    local ciItem;
    while( ciItem = ciInventory.GetNextObject() )
    {
      Equip( ciItem );
    }

    // Britannia controls wind direction for all Ultima versions
    local ciBroadcast = ::rumCreate( Wind_Direction_BroadcastID, Ultima4World.m_eWindDirection );
    SendBroadcast( ciBroadcast );

    if( IsJailed() )
    {
      Negate();
      SendJailUpdate();
    }

    // Was the player previously on a transport?
    local eBoardedWidgetID = GetVersionedProperty( g_eTransportBoardedWidgetPropertyVersionArray, rumInvalidAssetID );

    local bBoardedTransport = false;
    local ciTransport = SummonLastTransport();
    if( ciTransport != null )
    {
      local bShouldBoard = ( eBoardedWidgetID == ciTransport.GetAssetID() );
      if( bShouldBoard )
      {
        bBoardedTransport = ciTransport.Board( this, true /* force boarding */ );
      }
    }

    // Was the player on a personal transport?
    if( !bBoardedTransport && eBoardedWidgetID != rumInvalidAssetID )
    {
      // Get the transport asset
      local ciAsset = ::rumGetWidgetAsset( eBoardedWidgetID );
      if( ciAsset != null )
      {
        // Horses and rafts/skiffs
        local bPersonal = ciAsset.GetProperty( Transport_Personal_PropertyID, false );
        if( bPersonal )
        {
          local ciPos = GetPosition();
          local ciMap = GetMap();
          local ciPosData = ciMap.GetPositionData( ciPos );

          // Try to reboard any similar transports at the same location
          while( !bBoardedTransport && ( ciTransport = ciPosData.GetNext( rumWidgetPawnType ) ) )
          {
            if( ciTransport.GetAssetID() == eBoardedWidgetID )
            {
              bBoardedTransport = ciTransport.Board( this );
            }
          }

          if( !bBoardedTransport )
          {
            ciTransport = CreateTransport( eBoardedWidgetID, 0, ciMap, ciPos );
            if( ciTransport != null )
            {
              bBoardedTransport = ciTransport.Board( this );
            }
          }
        }
      }
    }
  }


  function HasAllRunes()
  {
    local iFlags = GetProperty( U4_Item_Runes_PropertyID, 0 );
    local iMaxValue = ::rumGetMaxPropertyValue( U4_Item_Runes_PropertyID );
    return ( iMaxValue == iFlags );
  }


  function HasAllStones()
  {
    local iFlags = GetProperty( U4_Item_Stones_PropertyID, 0 );
    local iMaxValue = ::rumGetMaxPropertyValue( U4_Item_Stones_PropertyID );
    return ( iMaxValue == iFlags );
  }


  function HasBellBookAndCandle()
  {
    local eBell = GetProperty( U4_Item_Bell_PropertyID, U4_QuestItemState.NotFound );
    if( U4_QuestItemState.Imbued == eBell )
    {
      local eBook = GetProperty( U4_Item_Book_PropertyID, U4_QuestItemState.NotFound );
      if( U4_QuestItemState.Imbued == eBook )
      {
        local eCandle = GetProperty( U4_Item_Candle_PropertyID, U4_QuestItemState.NotFound );
        return ( U4_QuestItemState.Imbued == eCandle );
      }
    }

    return false;
  }


  function HasRune( i_eVirtue )
  {
    local iFlags = GetProperty( U4_Item_Runes_PropertyID, 0 );
    return ( iFlags && ::rumBitOn( iFlags, i_eVirtue ) );
  }


  function HasThreePartsKey()
  {
    local iFlags = GetProperty( U4_Item_Three_Part_Key_PropertyID, 0 );
    local iMaxValue = ::rumGetMaxPropertyValue( U4_Item_Three_Part_Key_PropertyID );
    return ( iMaxValue == iFlags );
  }


  function IncrementHackAttempts()
  {
    local iHacks = GetProperty( HackAttempts_PropertyID, 0 );

    SetProperty( HackAttempts_PropertyID, iHacks + 1 );
    ActionInfo( msg_hack_warn_client_StringID );

    if( iHacks > 50 )
    {
      // TODO - ban account?
      ::rumPlayerLogout( GetSocket() );
    }
  }


  function Initialize( i_eGenderType, i_eCardVirtueArray )
  {
    ///////////////
    // Ultima IV //
    ///////////////
    if( i_eCardVirtueArray.len() != 7 )
    {
      return false;
    }

    // Set starting stats
    local iStrength = 15;
    local iDexterity = 15;
    local iIntelligence = 15;
    local iPotions = 0;
    local iGinseng = 0;
    local iGarlic = 0;

    // Virtue points accumulated due to bead selection
    local uiVirtuePointsArray = [0, 0, 0, 0, 0, 0, 0, 0];

    // Last card represents the player's class identifier
    local eClassVirtue = i_eCardVirtueArray[6];
    local eU4PlayerClassID = g_eU4PlayerClassArray[eClassVirtue];

    local iCount = 0;

    // Visit each gypsy card selected, each card represents a virtue
    // Note: Technically, a player could send a bogus ordering and receive a very high buff in a single stat or
    // virtue. A "good enough" check is to take the final virtue and make sure it only occurs two other times.
    foreach( eCardVirtue in i_eCardVirtueArray )
    {
      eCardVirtue = clamp( eCardVirtue, 0, VirtueType.NumVirtues - 1 );

      local ePlayerClassID = g_eU4PlayerClassArray[eCardVirtue];
      local ciCardClass = ::rumGetCustomAsset( ePlayerClassID );

      // Retrieve stat bonuses from the class table
      iStrength += ciCardClass.GetProperty( Class_Strength_Bonus_PropertyID, 0 );
      iDexterity += ciCardClass.GetProperty( Class_Dexterity_Bonus_PropertyID, 0 );
      iIntelligence += ciCardClass.GetProperty( Class_Intelligence_Bonus_PropertyID, 0 );

      // Apply virtue bonuses based on answers
      uiVirtuePointsArray[eCardVirtue] += 5;

      if( eClassVirtue == eCardVirtue )
      {
        // Keep track of how many times the main virtue was specified
        ++iCount;
      }
    }

    // The last chosen virtue card should appear 3 times overall, if not, the values are likely bogus
    if( iCount != 3 )
    {
      // Received a bogus cards array, likely manufactured by a player
      return false;
    }

    // Initialize player based on class
    local ciPlayerClass = ::rumGetCustomAsset( eU4PlayerClassID );
    if( null == ciPlayerClass )
    {
      return false;
    }

    local eMapID = ciPlayerClass.GetProperty( Class_Starting_Map_ID_PropertyID, rumInvalidAssetID );
    local iPosX = ciPlayerClass.GetProperty( Class_Starting_Pos_X_PropertyID, 0 );
    local iPosY = ciPlayerClass.GetProperty( Class_Starting_Pos_Y_PropertyID, 0 );

    // Starting weapon
    local eWeaponID = ciPlayerClass.GetProperty( Creature_Weapon_Type_PropertyID, rumInvalidAssetID );
    if( eWeaponID != rumInvalidAssetID )
    {
      AddOrCreateItem( eWeaponID );
    }

    // Starting armour
    local eArmourID = ciPlayerClass.GetProperty( Creature_Armour_Type_PropertyID, rumInvalidAssetID );
    if( eArmourID != rumInvalidAssetID )
    {
      AddOrCreateItem( eArmourID );
    }

    // Set player properties - keep dependencies in mind (i.e. Hitpoints depends on level, etc.)
    SetProperty( U4_PlayerClass_PropertyID, eU4PlayerClassID );
    SetProperty( U4_Experience_PropertyID, 0 );
    SetProperty( U4_Level_PropertyID, 1 );

    SetProperty( U4_Hitpoints_PropertyID, 100 );
    SetProperty( U4_Strength_PropertyID, iStrength );
    SetProperty( U4_Dexterity_PropertyID, iDexterity );
    SetProperty( U4_Intelligence_PropertyID, iIntelligence );

    i_eGenderType = clamp( i_eGenderType, 0, GenderType.NumGenderTypes - 1 );
    SetProperty( Gender_PropertyID, i_eGenderType );

    SetProperty( U4_Graphic_ID_PropertyID,
                 ciPlayerClass.GetProperty( Class_Graphic_ID_PropertyID, rumInvalidAssetID ) );

    SetProperty( U4_Virtue_Honesty_PropertyID, g_uiVirtuePointsTable.Medium + uiVirtuePointsArray[0] );
    SetProperty( U4_Virtue_Compassion_PropertyID, g_uiVirtuePointsTable.Medium + uiVirtuePointsArray[1] );
    SetProperty( U4_Virtue_Valor_PropertyID, g_uiVirtuePointsTable.Medium + uiVirtuePointsArray[2] );
    SetProperty( U4_Virtue_Justice_PropertyID, g_uiVirtuePointsTable.Medium + uiVirtuePointsArray[3] );
    SetProperty( U4_Virtue_Sacrifice_PropertyID, g_uiVirtuePointsTable.Medium + uiVirtuePointsArray[4] );
    SetProperty( U4_Virtue_Honor_PropertyID, g_uiVirtuePointsTable.Medium + uiVirtuePointsArray[5] );
    SetProperty( U4_Virtue_Spirituality_PropertyID, g_uiVirtuePointsTable.Medium + uiVirtuePointsArray[6] );
    SetProperty( U4_Virtue_Humility_PropertyID, g_uiVirtuePointsTable.Medium + uiVirtuePointsArray[7] );
    SetProperty( U4_Last_Incremented_Virtue_PropertyID, -1 );
    SetProperty( U4_Last_Incremented_Virtue_Map_PropertyID, -1 );

    SetProperty( U4_Gold_PropertyID, 300 );
    SetProperty( U4_Food_PropertyID, 200 );
    SetProperty( U4_Keys_PropertyID, 0 );
    SetProperty( U4_Torches_PropertyID, 5 );
    SetProperty( U4_Gems_PropertyID, 0 );

    local iMana = CalculateMaxMana( GameType.Ultima4 );
    SetProperty( U4_Mana_PropertyID, iMana );

    // Give spell components to those capable of casting Heal & Cure, otherwise give potions
    if( iMana >= 10 )
    {
      // Add ginseng and garlic to player
      SetProperty( U4_Reagent_Ginseng_PropertyID, 3 );
      SetProperty( U4_Reagent_Garlic_PropertyID, 4 );
    }
    else
    {
      // Give 4 starting potions
      SetProperty( U4_Potions_PropertyID, 4 );
    }

    ////////////////
    // Ultima III //
    ////////////////

    local eU3PlayerClassID = rumInvalidAssetID;

    // Determine the Ultima 3 player class and stats based on the Ultima 4 virtue test
    switch( eU4PlayerClassID )
    {
      case U4_Mage_Class_CustomID:
        eU3PlayerClassID = U3_Wizard_Class_CustomID;
        if( iDexterity >= iStrength )
        {
          eU3PlayerClassID = U3_Alchemist_Class_CustomID;
        }
        break;

      case U4_Bard_Class_CustomID:
        eU3PlayerClassID = U3_Thief_Class_CustomID;
        if( iIntelligence >= iStrength )
        {
          eU3PlayerClassID = U3_Lark_Class_CustomID;
        }
        break;

      case U4_Fighter_Class_CustomID:
        eU3PlayerClassID = U3_Fighter_Class_CustomID;
        if( iDexterity >= iIntelligence )
        {
          eU3PlayerClassID = U3_Barbarian_Class_CustomID;
        }
        break;

      case U4_Druid_Class_CustomID:
        eU3PlayerClassID = U3_Druid_Class_CustomID;
        if( iStrength >= iDexterity )
        {
          eU3PlayerClassID = U3_Wizard_Class_CustomID;
        }
        break;

      case U4_Tinker_Class_CustomID:
        eU3PlayerClassID = U3_Barbarian_Class_CustomID;
        if( iIntelligence >= iDexterity )
        {
          eU3PlayerClassID = U3_Fighter_Class_CustomID;
        }
        break;

      case U4_Paladin_Class_CustomID:
        eU3PlayerClassID = U3_Paladin_Class_CustomID;
        if( iDexterity >= iStrength )
        {
          eU3PlayerClassID = U3_Cleric_Class_CustomID;
        }
        break;

      case U4_Ranger_Class_CustomID:
        eU3PlayerClassID = U3_Ranger_Class_CustomID;
        if( iIntelligence >= iStrength )
        {
          eU3PlayerClassID = U3_Thief_Class_CustomID;
        }
        break;

      case U4_Shepherd_Class_CustomID:
        eU3PlayerClassID = U3_Cleric_Class_CustomID;
        if( iDexterity >= iStrength )
        {
          eU3PlayerClassID = U3_Illusionist_Class_CustomID;
        }
        break;
    }

    // 50 points total to allocate, with 5 in each attribute at a minimum...
    iStrength = 5;
    iDexterity = 5;
    iIntelligence = 5;
    local iWisdom = 5;

    // Distribute the remaining 30 points

    foreach( eCardVirtue in i_eCardVirtueArray )
    {
      switch( eCardVirtue )
      {
        case VirtueType.Honesty:
          iIntelligence += 3;
          break;

        case VirtueType.Compassion:
          iDexterity += 3;
          break;

        case VirtueType.Valor:
          iStrength += 3;
          break;

        // Truth and love
        case VirtueType.Justice:
          iIntelligence += 2;
          iDexterity += 1;
          break;

        // Courage and love
        case VirtueType.Sacrifice:
          iStrength += 2;
          iDexterity += 1;
          break;

        // Courage and Truth
        case VirtueType.Honor:
          iStrength += 2;
          iIntelligence += 1;
          break;

        case VirtueType.Spirituality:
          iWisdom += 3;
          break;

        case VirtueType.Humility:
          iWisdom += 3;
          break;
      }
    }

    local iExcess = 0;

    // No starting U3 attribute can be higher than 25
    if( iStrength > 25 )
    {
      iExcess += ( iStrength - 25 );
      iStrength = 25;
    }

    if( iDexterity > 25 )
    {
      iExcess += ( iDexterity - 25 );
      iDexterity = 25;
    }

    if( iIntelligence > 25 )
    {
      iExcess += ( iIntelligence - 25 );
      iIntelligence = 25;
    }

    if( iWisdom > 25 )
    {
      iExcess += ( iWisdom - 25 );
      iWisdom = 25;
    }

    while( iExcess > 0 )
    {
      // This could probably be reworked to benefit particular classes better, but meh.
      if( iDexterity < 25 )
      {
        ++iDexterity;
      }
      else if( iStrength < 25 )
      {
        ++iStrength;
      }
      else if( iIntelligence < 25 )
      {
        ++iIntelligence;
      }
      else if( iWisdom < 25 )
      {
        ++iWisdom;
      }
      else
      {
        // Give up
        iExcess = 0;
      }
    }

    ciPlayerClass = ::rumGetCustomAsset( eU3PlayerClassID );

    // Starting weapon
    eWeaponID = ciPlayerClass.GetProperty( Creature_Weapon_Type_PropertyID, rumInvalidAssetID );
    if( eWeaponID != rumInvalidAssetID )
    {
      AddOrCreateItem( eWeaponID );
    }

    // Starting armour
    eArmourID = ciPlayerClass.GetProperty( Creature_Armour_Type_PropertyID, rumInvalidAssetID );
    if( eArmourID != rumInvalidAssetID )
    {
      AddOrCreateItem( eArmourID );
    }

    SetProperty( U3_Graphic_ID_PropertyID,
                 ciPlayerClass.GetProperty( Class_Graphic_ID_PropertyID, rumInvalidAssetID ) );

    SetProperty( U3_PlayerClass_PropertyID, eU3PlayerClassID );
    SetProperty( U3_Experience_PropertyID, 0 );
    SetProperty( U3_Level_PropertyID, 1 );

    SetProperty( U3_Hitpoints_PropertyID, 400 );

    SetProperty( U3_Strength_PropertyID, iStrength );
    SetProperty( U3_Dexterity_PropertyID, iDexterity );
    SetProperty( U3_Intelligence_PropertyID, iIntelligence );
    SetProperty( U3_Wisdom_PropertyID, iWisdom );

    SetProperty( U3_Gold_PropertyID, 150 );
    SetProperty( U3_Food_PropertyID, 150 );

    SetProperty( U3_Gems_PropertyID, 0 );
    SetProperty( U3_Keys_PropertyID, 0 );
    SetProperty( U3_Torches_PropertyID, 0 );

    // Randomize the locations where exotics can be discovered
    local iWeaponIndex = rand() % ( g_uiU3ExoticPosXArray.len() );
    SetProperty( U3_Exotic_Weapon_Pos_Index_PropertyID, iWeaponIndex );
    local iArmourIndex = iWeaponIndex;
    while( iArmourIndex == iWeaponIndex )
    {
      iArmourIndex = rand() % ( g_uiU3ExoticPosXArray.len() );
    }
    SetProperty( U3_Exotic_Armour_Pos_Index_PropertyID, iArmourIndex );

    // Randomize the Exodus card order
    local iExodusIndex = rand() % ( g_uiU3ExodusOrderIndex0Array.len() );
    SetProperty( U3_Exodus_Card_Index_PropertyID, iExodusIndex );

    iMana = CalculateMaxMana( GameType.Ultima3 );
    SetProperty( U3_Mana_PropertyID, iMana );

    if( eU3PlayerClassID != U3_Paladin_Class_CustomID && eU3PlayerClassID != U3_Cleric_Class_CustomID )
    {
      // Give 4 starting potions
      SetProperty( U3_Potions_PropertyID, 4 );
    }

    ///////////////
    // Ultima II //
    ///////////////

    local eU2PlayerClassID = rumInvalidAssetID;

    iStrength = 10;
    iDexterity = 10; // A.K.A. Agility
    iIntelligence = 10;
    iWisdom = 10;

    local iCharisma = 10;
    local iStamina = 10;

    // Determine the Ultima 2 player class and stats based on the Ultima 4 virtue test
    switch( eU4PlayerClassID )
    {
      case U4_Mage_Class_CustomID:
      case U4_Druid_Class_CustomID:
        eU2PlayerClassID = U2_Wizard_Class_CustomID;
        break;

      case U4_Bard_Class_CustomID:
      case U4_Ranger_Class_CustomID:
        eU2PlayerClassID = U2_Thief_Class_CustomID;
        break;

      case U4_Fighter_Class_CustomID:
      case U4_Tinker_Class_CustomID:
        eU2PlayerClassID = U2_Fighter_Class_CustomID;
        break;

      case U4_Paladin_Class_CustomID:
      case U4_Shepherd_Class_CustomID:
        eU2PlayerClassID = U2_Cleric_Class_CustomID;
        break;
    }

    ciPlayerClass = ::rumGetCustomAsset( eU2PlayerClassID );

    // Apply profession bonuses
    iIntelligence += ciPlayerClass.GetProperty( Class_Intelligence_Bonus_PropertyID, 0 );
    iDexterity += ciPlayerClass.GetProperty( Class_Dexterity_Bonus_PropertyID, 0 );
    iStrength += ciPlayerClass.GetProperty( Class_Strength_Bonus_PropertyID, 0 );
    iWisdom += ciPlayerClass.GetProperty( Class_Wisdom_Bonus_PropertyID, 0 );

    // There are 90 points to distribute, but each attribute must be given at least 10 points, leaving 30 left to
    // allocate. Unlike U1, U2 does not have a cap on starting attributes.

    foreach( eCardVirtue in i_eCardVirtueArray )
    {
      iCharisma += 1;
      iStamina += 1;

      switch( eCardVirtue )
      {
        case VirtueType.Honesty:
          iIntelligence += 3;
          break;

        case VirtueType.Compassion:
          iDexterity += 3;
          break;

        case VirtueType.Valor:
          iStrength += 3;
          break;

        case VirtueType.Justice:
          iDexterity += 2;
          iIntelligence += 1;
          break;

        case VirtueType.Sacrifice:
          iStrength += 2;
          iDexterity += 1;
          break;

        case VirtueType.Honor:
          iIntelligence += 2;
          iStrength += 1;
          break;

        case VirtueType.Spirituality:
          iWisdom += 3;
          break;

        case VirtueType.Humility:
          iWisdom += 3;
          break;
      }
    }

    // U2 had race bonuses as well, but in this version of the game, everyone's human. Just bump each of the
    // primary stats by two points to compensate.
    iIntelligence += 2;
    iDexterity += 2;
    iStrength += 2;
    iWisdom += 2;

    // U2 also had gender bonuses... ignored.

    SetProperty( U2_Graphic_ID_PropertyID,
                 ciPlayerClass.GetProperty( Class_Graphic_ID_PropertyID, rumInvalidAssetID ) );

    SetProperty( U2_PlayerClass_PropertyID, eU2PlayerClassID );
    SetProperty( U2_Experience_PropertyID, 0 );

    SetProperty( U2_Hitpoints_PropertyID, 400 );

    SetProperty( U2_Strength_PropertyID, iStrength );
    SetProperty( U2_Agility_PropertyID, iDexterity );
    SetProperty( U2_Intelligence_PropertyID, iIntelligence );
    SetProperty( U2_Stamina_PropertyID, iStamina );
    SetProperty( U2_Wisdom_PropertyID, iWisdom );
    SetProperty( U2_Charisma_PropertyID, iCharisma );

    SetProperty( U2_Gold_PropertyID, 400 );
    SetProperty( U2_Food_PropertyID, 400 );

    SetProperty( U2_Ankhs_PropertyID, 0 );
    SetProperty( U2_Blue_Tassles_PropertyID, 0 );
    SetProperty( U2_Boots_PropertyID, 0 );
    SetProperty( U2_Brass_Buttons_PropertyID, 0 );
    SetProperty( U2_Cloaks_PropertyID, 0 );
    SetProperty( U2_Green_Idols_PropertyID, 0 );
    SetProperty( U2_Helms_PropertyID, 0 );
    SetProperty( U2_Rings_PropertyID, 0 );
    SetProperty( U2_Skull_Keys_PropertyID, 0 );
    SetProperty( U2_Staffs_PropertyID, 0 );
    SetProperty( U2_Tools_PropertyID, 0 );
    SetProperty( U2_Torches_PropertyID, 5 );
    SetProperty( U2_Tri_Lithium_PropertyID, 0 );
    SetProperty( U2_Wands_PropertyID, 0 );

    // Randomize the Ring location
    local iRingIndex = rand() % ( g_eU2RingMapArray.len() );
    SetProperty( U2_Ring_Index_PropertyID, iRingIndex );

    iMana = CalculateMaxMana( GameType.Ultima2 );
    SetProperty( U2_Mana_PropertyID, iMana );

    if( U2_Cleric_Class_CustomID == eU2PlayerClassID )
    {
      // Give 4 starting heal spells
      SetProperty( U2_Spell_Heal_PropertyID, 4 );
    }
    else
    {
      // Give 4 starting potions
      SetProperty( U2_Potions_PropertyID, 4 );
    }

    //////////////
    // Ultima I //
    //////////////

    local eU1PlayerClassID = rumInvalidAssetID;

    iStrength = 10;
    iDexterity = 10;
    iIntelligence = 10;
    iWisdom = 10;
    iCharisma = 10;
    iStamina = 10;

    // There are 90 points to distribute, but each attribute must be given at least 10 points, leaving 30 left to
    // allocate. Unlike U2, U1 has a cap of 25 on starting attributes.

    foreach( eCardVirtue in i_eCardVirtueArray )
    {
      iCharisma += 1;
      iStamina += 1;

      switch( eCardVirtue )
      {
        case VirtueType.Honesty:
          iIntelligence += 3;
          break;

        case VirtueType.Compassion:
          iDexterity += 3;
          break;

        case VirtueType.Valor:
          iStrength += 3;
          break;

        case VirtueType.Justice:
          iDexterity += 2;
          iIntelligence += 1;
          break;

        case VirtueType.Sacrifice:
          iStrength += 2;
          iDexterity += 1;
          break;

        case VirtueType.Honor:
          iIntelligence += 2;
          iStrength += 1;
          break;

        case VirtueType.Spirituality:
          iWisdom += 3;
          break;

        case VirtueType.Humility:
          iWisdom += 3;
          break;
      }
    }

    iExcess = 0;

    // No starting U1 attribute can be higher than 25
    if( iStrength > 25 )
    {
      iExcess += ( iStrength - 25 );
      iStrength = 25;
    }

    if( iDexterity > 25 )
    {
      iExcess += ( iDexterity - 25 );
      iDexterity = 25;
    }

    if( iIntelligence > 25 )
    {
      iExcess += ( iIntelligence - 25 );
      iIntelligence = 25;
    }

    if( iWisdom > 25 )
    {
      iExcess += ( iWisdom - 25 );
      iWisdom = 25;
    }

    if( iExcess > 0 )
    {
      // If there are any excess points, add the remainder to stamina
      iStamina += iExcess;

      if( iStamina > 25 )
      {
        iExcess = ( iStamina - 25 );

        // If stamina overflowed, add the remaining points to charisma
        if( iExcess > 0 )
        {
          iCharisma += iExcess;
          iCharisma = clamp( iChr, 10, 25 );
        }
      }
    }

    // Determine the Ultima 1 player class and stats based on the Ultima 4 virtue test
    switch( eU4PlayerClassID )
    {
      case U4_Mage_Class_CustomID:
      case U4_Druid_Class_CustomID:
        eU1PlayerClassID = U1_Wizard_Class_CustomID;
        break;

      case U4_Bard_Class_CustomID:
      case U4_Ranger_Class_CustomID:
        eU1PlayerClassID = U1_Thief_Class_CustomID;
        break;

      case U4_Fighter_Class_CustomID:
      case U4_Tinker_Class_CustomID:
        eU1PlayerClassID = U1_Fighter_Class_CustomID;
        break;

      case U4_Paladin_Class_CustomID:
      case U4_Shepherd_Class_CustomID:
        eU1PlayerClassID = U1_Cleric_Class_CustomID;
        break;
    }

    ciPlayerClass = ::rumGetCustomAsset( eU1PlayerClassID );

    // Starting weapon
    eWeaponID = ciPlayerClass.GetProperty( Creature_Weapon_Type_PropertyID, rumInvalidAssetID );
    if( eWeaponID != rumInvalidAssetID )
    {
      AddOrCreateItem( eWeaponID );
    }

    // Starting armour
    eArmourID = ciPlayerClass.GetProperty( Creature_Armour_Type_PropertyID, rumInvalidAssetID );
    if( eArmourID != rumInvalidAssetID )
    {
      AddOrCreateItem( eArmourID );
    }

    // Apply profession bonuses
    iIntelligence += ciPlayerClass.GetProperty( Class_Intelligence_Bonus_PropertyID, 0 );
    iDexterity += ciPlayerClass.GetProperty( Class_Dexterity_Bonus_PropertyID, 0 );
    iStrength += ciPlayerClass.GetProperty( Class_Strength_Bonus_PropertyID, 0 );
    iWisdom += ciPlayerClass.GetProperty( Class_Wisdom_Bonus_PropertyID, 0 );

    // U1 had race bonuses as well, but in this version of the game, everyone's human. Just bump each of the
    // primary stats by two points to compensate.
    iIntelligence += 2;
    iDexterity += 2;
    iStrength += 2;
    iWisdom += 2;

    SetProperty( U1_Graphic_ID_PropertyID,
                 ciPlayerClass.GetProperty( Class_Graphic_ID_PropertyID, rumInvalidAssetID ) );

    SetProperty( U1_PlayerClass_PropertyID, eU1PlayerClassID );
    SetProperty( U1_Experience_PropertyID, 0 );

    SetProperty( U1_Hitpoints_PropertyID, 150 );

    SetProperty( U1_Strength_PropertyID, iStrength );
    SetProperty( U1_Agility_PropertyID, iDexterity );
    SetProperty( U1_Intelligence_PropertyID, iIntelligence );
    SetProperty( U1_Stamina_PropertyID, iStamina );
    SetProperty( U1_Wisdom_PropertyID, iWisdom );
    SetProperty( U1_Charisma_PropertyID, iCharisma );

    SetProperty( U1_Coin_PropertyID, 100 );
    SetProperty( U1_Food_PropertyID, 200 );
    SetProperty( U1_Gems_PropertyID, 0 );

    iMana = CalculateMaxMana( GameType.Ultima1 );
    SetProperty( U1_Mana_PropertyID, iMana );

    if( U1_Cleric_Class_CustomID == eU1PlayerClassID )
    {
      // Give 4 starting heal spells
      SetProperty( U1_Spell_Heal_PropertyID, 4 );
    }
    else
    {
      // Give 4 starting potions
      SetProperty( U1_Potions_PropertyID, 4 );
    }

    //////////
    // Done //
    //////////

    // This also indicates that the player has been fully initialized
    SetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );

    return StartPlaying( eMapID, rumPos( iPosX, iPosY ) );
  }


  function IsAdmin()
  {
    return GetProperty( Admin_PropertyID, false );
  }


  function IsCriminal()
  {
    return GetVersionedProperty( g_eCriminalPropertyVersionArray, false );
  }


  function IsDaemonImmune()
  {
    return GetProperty( U4_Shrine_Daemon_Immunity_PropertyID, false );
  }


  function IsDead()
  {
    return GetVersionedProperty( g_eHitpointsPropertyVersionArray, 1 ) <= 0;
  }


  function IsEightPartsAvatar()
  {
    local iFlags = GetProperty( U4_Virtue_Elevation_PropertyID, 0 );
    local iMaxValue = ::rumGetMaxPropertyValue( U4_Virtue_Elevation_PropertyID );
    return ( iMaxValue == iFlags );
  }


  function IsFlying()
  {
    local ciTransport = GetTransport();
    if( ciTransport != null )
    {
      local eMoveType = ciTransport.GetMoveType();
      if( ( MoveType.Drifts == eMoveType ) || ( MoveType.Flies == eMoveType ) )
      {
        return ciTransport.GetProperty( State_PropertyID, FlyingStateType.Landed ) == FlyingStateType.Flying;
      }
    }

    return false;
  }


  function IsIncapacitated()
  {
    return IsMeditating() || base.IsIncapacitated();
  }


  function IsJailed()
  {
    local bJailed = false;

    local ciMap = GetMap();
    if( ciMap != null && IsCriminal() )
    {
      local ciPos = GetPosition();

      // Is there a jail widget nearby?
      local ciPawnArray = ciMap.GetPawns( ciPos, 1, false /* no LOS check */ );
      foreach( ciPawn in ciPawnArray )
      {
        if( ciPawn.GetAssetID() == Jail_Cell_WidgetID )
        {
          bJailed = true;
          break;
        }
      }
    }

    return bJailed;
  }


  function IsMeditating()
  {
    return GetProperty( U4_Meditating_PropertyID, false );
  }


  function IsPartialAvatar()
  {
    // Return true if the player is *at least* a partial Avatar, so a full 8-parts Avatar will return true as well
    local iFlags = GetProperty( U4_Virtue_Elevation_PropertyID, 0 );
    return ( iFlags > 0 );
  }


  function IsPoisoned()
  {
    return GetVersionedProperty( g_ePoisonedPropertyVersionArray, false );
  }


  function IsStarving()
  {
    return GetVersionedProperty( g_eFoodPropertyVersionArray ) == 0;
  }


  function IsThief()
  {
    local ePlayerClassID = GetVersionedProperty( g_ePlayerClassPropertyVersionArray, rumInvalidAssetID );
    return ( ( U1_Thief_Class_CustomID == ePlayerClassID ) ||
             ( U2_Thief_Class_CustomID == ePlayerClassID ) ||
             ( U3_Thief_Class_CustomID == ePlayerClassID ) );
  }


  function Jail()
  {
    if( IsJailed() )
    {
      return;
    }

    local ciMap = GetMap();
    if( null == ciMap )
    {
      return;
    }

    local eMapID = ciMap.GetJailMap();
    if( rumInvalidAssetID == eMapID )
    {
      return;
    }

    local ciDestMap = GetOrCreateMap( this, eMapID );
    if( null == ciDestMap )
    {
      return;
    }

    local ciPos = GetPosition();

    // Remove player from all transports
    local ciTransport = GetTransport();
    if( ciTransport != null )
    {
      ciTransport.Exit( this, ciPos, MoveType.Incorporeal );
    }

    local ciPositionArray = array( 0 );

    // Build a list of possible jail cells
    local ciPawnArray = ciDestMap.GetPawns( ciPos, 256, false );
    foreach( ciPawn in ciPawnArray )
    {
      if( ciPawn.GetAssetID() == Jail_Cell_WidgetID )
      {
        ciPositionArray.append( ciPawn.GetPosition() );
      }
    }

    // Choose a cell at random
    if( ciPositionArray.len() > 0 )
    {
      local ciCellPos = ciPositionArray[ rand()%ciPositionArray.len() ];
      ciMap.TransferPawn( this, ciDestMap, ciCellPos );
    }
    else
    {
      ::rumLog( "Couldn't find a jail cell destination for map " + ciMap.GetName() );
      return;
    }

    // Negate the player's ability to cast spells
    Negate();

    // Remove all gold temporarily
    local iGold = GetVersionedProperty( g_eGoldPropertyVersionArray, 0 );
    SetVersionedProperty( g_eGoldPropertyVersionArray, 0 );
    SetVersionedProperty( g_eConfiscatedGoldPropertyVersionArray,
                          GetVersionedProperty( g_eConfiscatedGoldPropertyVersionArray, 0 ) + iGold );

    // Remove all keys temporarily
    local iKeys = GetVersionedProperty( g_eKeysPropertyVersionArray, 0 );
    SetVersionedProperty( g_eKeysPropertyVersionArray, 0 );
    SetVersionedProperty( g_eConfiscatedKeysPropertyVersionArray,
                          GetVersionedProperty( g_eConfiscatedKeysPropertyVersionArray, 0 ) + iKeys );

    ActionWarning( msg_jailed_client_StringID );
    ActionWarning( msg_jail_confiscation_client_StringID );

    SendJailUpdate();
  }


  function Negate( i_fDuration = Negate_Effect.s_fDuration )
  {
    if( base.Negate() )
    {
      SetProperty( Last_Negate_Time_PropertyID, ::rumGetSecondsSinceEpoch() );
      return true;
    }

    return false;
  }


  function OnAddedToMap()
  {
    local ciTransport = GetTransport();
    if( ciTransport != null )
    {
      ciTransport.SetPlayerTransportSettings( this );
    }
  }


  function OnCampingDone( i_iCampIndex )
  {
    if( m_bCamping && ( i_iCampIndex == m_iCampIndex ) )
    {
      RemoveProperty( Unconscious_PropertyID );

      // Heal the player
      AffectHitpoints( rand() % 300 + 300 );
    }
  }


  function OnDeath( i_ciSource, i_bKillCredit = true )
  {
    local ciMap = GetMap();

    RemoveEffects();

    // Player gets virtue for dying
    AffectVirtue( VirtueType.Sacrifice, 1, true, true );

    local ciPos = GetPosition();

    // Remove player from all transports
    local ciTransport = GetTransport();
    if( ciTransport != null )
    {
      ciTransport.Exit( this, ciPos, MoveType.Incorporeal );
    }

    // Turn player into ghost and allow incorporeal transit
    SetMoveType( MoveType.Incorporeal );

    // Create a player corpse
    local ciPosData = ciMap.GetPositionData( ciPos );
    local ciTile = ::rumGetTileAsset( ciPosData.GetTileID() );
    if( !ciTile.IsCollision( MoveType.Terrestrial ) )
    {
      local ciCorpse = ::rumCreate( U4_Body_Dead_WidgetID );
      if( ciCorpse != null )
      {
        if( ciMap.AddPawn( ciCorpse, ciPos ) )
        {
          // Set the corpse ID to the player's ID
          ciCorpse.SetProperty( Player_ID_PropertyID, GetID() );

          // The player has a limited time to resurrect to the corpse if able
          ::rumSchedule( ciCorpse, ciCorpse.Expire, s_fDeathInterval );
        }
      }

      ++m_iDeathIndex;
      ::rumSchedule( this, OnDeathTimeout, s_fDeathInterval, m_iDeathIndex );
    }

    if( IsPoisoned() )
    {
      Cure();
    }

    // Ghosts can see in the dark (a little)
    SetLightRange( 2 );

    if( i_ciSource != null && i_ciSource.GetAssetID() == U1_Mondain_CreatureID )
    {
      ActionInfo( ::rumGetString( msg_mondain_doomed_server_StringID, m_iLanguageID ), false );
    }
  }


  function OnDeathTimeout( i_iDeathIndex )
  {
    if( i_iDeathIndex == m_iDeathIndex )
    {
      // Resurrect the player from the void
      Resurrect( ResurrectionType.Void );
    }
  }


  function OnIdleLogout()
  {
    Quit();
  }


  function OnIdleWarn()
  {
    ActionWarning( msg_idle_warn_client_StringID );
  }


  function OnInnCheckout( i_iRoomIndex )
  {
    HandleInnCheckout( this, i_iRoomIndex );
  }


  function OnInnInvasion( i_iRoomIndex )
  {
    HandleInnInvasion( this, i_iRoomIndex );
  }


  function OnInventoryRestored( i_ciItem )
  {
    Equip( i_ciItem );
  }


  function OnLogin( i_strName, i_uiPlayerID )
  {
    // TODO - send notice that player is logging in?
  }


  function OnLogout()
  {
    // TODO - send notice that player is logging out?

    StopDialogue( DialogueTerminationType.Disconnected );

    // Remove the player from their transport
    local ciTransport = GetTransport();
    if( ciTransport != null )
    {
      ciTransport.HandleLogout( this );
    }

    // Leave party if in one
    local ciParty = GetParty();
    if( ciParty != null )
    {
      ciParty.Dismiss( this, PartyBroadcastType.PlayerLeft );
    }
  }


  function OnKilled( i_ciTarget, i_bKillCredit = true )
  {
    base.OnKilled( i_ciTarget, i_bKillCredit );

    // Reward experience and notify the player
    if( i_ciTarget instanceof NPC )
    {
      // It is criminal to kill good creatures and people
      local eAlignment = i_ciTarget.GetAlignment();
      if( AlignmentType.Good == eAlignment )
      {
        AdjustVersionedProperty( g_eNotorietyPropertyVersionArray, 10 );
      }

      local eExpPropertyType = rumInvalidAssetID;
      local eGoldPropertyType = rumInvalidAssetID;

      if( i_ciTarget instanceof U4_NPC )
      {
        eExpPropertyType = U4_Experience_PropertyID;

        if( AlignmentType.Good == eAlignment )
        {
          AffectVirtue( VirtueType.Justice, -1, true, true );
          AffectVirtue( VirtueType.Honor, 1, true, false );
        }
      }
      else if( i_ciTarget instanceof U3_NPC )
      {
        eExpPropertyType = U3_Experience_PropertyID;
      }
      else if( i_ciTarget instanceof U2_NPC )
      {
        eExpPropertyType = U2_Experience_PropertyID;

        if( i_bKillCredit )
        {
          local bRewardsGold = i_ciTarget.GetProperty( Creature_Rewards_Gold_PropertyID, false );
          if( bRewardsGold )
          {
            eGoldPropertyType = U2_Gold_PropertyID;
          }
        }
      }
      else if( i_ciTarget instanceof U1_NPC )
      {
        eExpPropertyType = U1_Experience_PropertyID;

        if( i_bKillCredit )
        {
          local bRewardsCoin = i_ciTarget.GetProperty( Creature_Rewards_Gold_PropertyID, false );
          if( bRewardsCoin )
          {
            eGoldPropertyType = U1_Coin_PropertyID;
          }
        }
      }

      local iExpReward = 0;
      if( i_bKillCredit && eExpPropertyType != rumInvalidAssetID )
      {
        iExpReward = i_ciTarget.GetProperty( Creature_XP_Reward_PropertyID, 0 );
        SetProperty( eExpPropertyType, GetProperty( eExpPropertyType, 0 ) + iExpReward );
      }

      local ciBroadcast = ::rumCreate( Creature_Killed_BroadcastID, i_ciTarget.GetAssetID(), iExpReward );
      SendBroadcast( ciBroadcast );

      if( i_bKillCredit && eGoldPropertyType != rumInvalidAssetID )
      {
        // Also award gold
        local iGold = rand() % 20 + 1;
        AdjustVersionedProperty( g_eGoldPropertyVersionArray, iGold );
      }
    }
    else
    {
      local ciBroadcast = ::rumCreate( Creature_Killed_BroadcastID, i_ciTarget.GetAssetID(), i_bKillCredit ? 12 : 0 );
      SendBroadcast( ciBroadcast );
    }
  }


  function OnManaRestored( i_iUpdateIndex )
  {
    if( !IsDead() && ( i_iUpdateIndex == m_iManaUpdateIndex ) )
    {
      // Increment mana
      local iMana = GetVersionedProperty( g_eManaPropertyVersionArray );
      if( iMana < GetMaxMana() )
      {
        AdjustVersionedProperty( g_eManaPropertyVersionArray, 1 );
      }
    }
  }


  function OnMealRequired( i_iUpdateIndex )
  {
    if( !IsDead() && ( i_iUpdateIndex == m_iMealUpdateIndex ) )
    {
      if( IsStarving() )
      {
        // Players don't starve when imprisoned
        if( !IsJailed() )
        {
          ActionFailed( msg_starving_client_StringID );
          Damage( rand() % 10 + 5, null );
        }
      }
      else
      {
        AdjustVersionedProperty( g_eFoodPropertyVersionArray, -1 );
      }

      ::rumSchedule( this, OnMealRequired, s_fMealInterval, m_iMealUpdateIndex );
    }
  }


  function OnNotorietyChanged( i_iNotoriety )
  {
    if( i_iNotoriety > 0 )
    {
      ScheduleNotorietyUpdate();
    }

    if( i_iNotoriety < s_iNotorietyCriminalLevel )
    {
      ReleaseFromJail();
    }
  }


  function OnNotorietyReduced( i_iNotorietyIndex )
  {
    if( m_iNotorietyIndex == i_iNotorietyIndex )
    {
      local iNotoriety = AdjustVersionedProperty( g_eNotorietyPropertyVersionArray, -1 );
      if( iNotoriety < s_iNotorietyCriminalLevel )
      {
        ReleaseFromJail();
      }
      else
      {
        SendJailUpdate();
      }
    }
  }


  function OnPropertyRemoved( i_ePropertyID )
  {
    base.OnPropertyRemoved( i_ePropertyID );

    if( Unconscious_PropertyID == i_ePropertyID )
    {
      m_bCamping = false;
    }
  }


  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    local eVersion = GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );

    base.OnPropertyUpdated( i_ePropertyID, i_vValue );

    if( ( ( U4_Mana_PropertyID == i_ePropertyID ) && ( GameType.Ultima4 == eVersion ) ) ||
        ( ( U3_Mana_PropertyID == i_ePropertyID ) && ( GameType.Ultima3 == eVersion ) ) ||
        ( ( U2_Mana_PropertyID == i_ePropertyID ) && ( GameType.Ultima2 == eVersion ) ) ||
        ( ( U1_Mana_PropertyID == i_ePropertyID ) && ( GameType.Ultima1 == eVersion ) ) )
    {
      if( i_vValue < GetMaxMana() )
      {
        ::rumSchedule( this, OnManaRestored, s_fManaRestoreInterval, ++m_iManaUpdateIndex );
      }
    }
    else if( Max_Mana_PropertyID == i_ePropertyID )
    {
      if( GetMana() < i_vValue )
      {
        ::rumSchedule( this, OnManaRestored, s_fManaRestoreInterval, ++m_iManaUpdateIndex );
      }
    }
    else if( ( U4_Strength_PropertyID == i_ePropertyID )     ||
             ( U4_Dexterity_PropertyID == i_ePropertyID )    ||
             ( U4_Intelligence_PropertyID == i_ePropertyID ) ||
             ( U3_Strength_PropertyID == i_ePropertyID )     ||
             ( U3_Dexterity_PropertyID == i_ePropertyID )    ||
             ( U3_Intelligence_PropertyID == i_ePropertyID ) ||
             ( U3_Wisdom_PropertyID == i_ePropertyID )       ||
             ( U2_Strength_PropertyID == i_ePropertyID )     ||
             ( U2_Agility_PropertyID == i_ePropertyID )      ||
             ( U2_Intelligence_PropertyID == i_ePropertyID ) ||
             ( U2_Wisdom_PropertyID == i_ePropertyID )       ||
             ( U2_Stamina_PropertyID == i_ePropertyID )      ||
             ( U2_Charisma_PropertyID == i_ePropertyID )     ||
             ( U1_Strength_PropertyID == i_ePropertyID )     ||
             ( U1_Agility_PropertyID == i_ePropertyID )      ||
             ( U1_Intelligence_PropertyID == i_ePropertyID ) ||
             ( U1_Wisdom_PropertyID == i_ePropertyID )       ||
             ( U1_Stamina_PropertyID == i_ePropertyID )      ||
             ( U1_Charisma_PropertyID == i_ePropertyID ) )
    {
      OnStatChange( i_ePropertyID, i_vValue, eVersion );
    }
    else if( ( ( U4_Notoriety_PropertyID == i_ePropertyID ) && ( GameType.Ultima4 == eVersion ) ) ||
             ( ( U3_Notoriety_PropertyID == i_ePropertyID ) && ( GameType.Ultima3 == eVersion ) ) ||
             ( ( U2_Notoriety_PropertyID == i_ePropertyID ) && ( GameType.Ultima2 == eVersion ) ) ||
             ( ( U1_Notoriety_PropertyID == i_ePropertyID ) && ( GameType.Ultima1 == eVersion ) ) )
    {
      OnNotorietyChanged( i_vValue );
      SetVersionedProperty( g_eCriminalPropertyVersionArray, i_vValue >= s_iNotorietyCriminalLevel );
    }
    else if( ( Reagent_Discovery_Blocked_PropertyID == i_ePropertyID ) && ( GameType.Ultima4 == eVersion ) )
    {
      ::rumSchedule( this, ResetReagentDiscovery, Ultima4World.s_fMoonPhaseInterval );
    }
    else if( ( U4_Weary_Mind_Cycle_Count_PropertyID == i_ePropertyID ) && ( GameType.Ultima4 == eVersion ) )
    {
      ::rumSchedule( this, ClearWearyMind, Ultima4World.s_fMoonPhaseInterval );
    }
  }


  function OnRestored( i_eMapID, i_ciPos )
  {
    return StartPlaying( i_eMapID, i_ciPos );
  }


  function OnStatChange( i_ePropertyID, i_eValue, i_eVersion )
  {
    // Honor the stat cap based on class
    local iCap = GetStatCap( i_ePropertyID );
    if( i_eValue > iCap )
    {
      SetProperty( i_ePropertyID, iCap );
    }

    switch( i_ePropertyID )
    {
      case U4_Intelligence_PropertyID:
      case U3_Intelligence_PropertyID:
      case U2_Intelligence_PropertyID:
      case U1_Intelligence_PropertyID:
      case U3_Wisdom_PropertyID:
      case U2_Wisdom_PropertyID:
      case U1_Wisdom_PropertyID:
        local iMaxMana = CalculateMaxMana( i_eVersion );
        SetProperty( Max_Mana_PropertyID, iMaxMana );
        break;
    }
  }


  function OnWeaponThrown( i_eWeaponType, i_iDistance )
  {
    if( i_iDistance > 1 )
    {
      local bRemoved = false;
      local ciItem = GetInventoryByType( i_eWeaponType );
      if( ciItem != null )
      {
        local iQuantity = ciItem.GetProperty( Inventory_Quantity_PropertyID, 1 );
        if( iQuantity > 1 )
        {
          // Reduce the quantity
          ciItem.SetProperty( Inventory_Quantity_PropertyID, iQuantity - 1 );
          bRemoved = true;
        }
        else
        {
          // Remove the equipped item
          RemoveWeapon();

          // Permanently delete the weapon
          if( DeleteInventory( ciItem ) )
          {
            bRemoved = true;
          }
        }
      }

      if( !bRemoved )
      {
        // No weapon was removed, equipped or unequipped!
        IncrementHackAttempts();
      }
    }
  }


  function Poison( i_bForce = false )
  {
    if( base.Poison( i_bForce ) )
    {
      ActionWarning( msg_poisoned_client_StringID );
    }
  }


  function PoisonTrap( i_bForce = false )
  {
    return base.Poison( i_bForce );
  }


  function ProcessNextPacket()
  {
    PopPacket();
  }


  function Quit()
  {
    OnLogout();

    local ciBroadcast = ::rumCreate( Player_Quit_BroadcastID );
    SendBroadcast( ciBroadcast );

    ::rumPlayerLogout( GetSocket() );
  }


  function ReleaseFromJail()
  {
    if( IsJailed() )
    {
      local ciMap = GetMap();
      local ciExitPos = rumPos( 0, 0 );

      // Find the exit position
      local ciPawnArray = ciMap.GetPawns( GetPosition(), 256, false );
      foreach( ciPawn in ciPawnArray )
      {
        if( ciPawn.GetAssetID() == Jail_Free_WidgetID )
        {
          ciExitPos = ciPawn.GetPosition();
          break;
        }
      }

      local ciTransport = GetTransport();
      if( ciTransport != null )
      {
        ciTransport.Portal( ciMap.GetAssetID(), ciExitPos );
      }
      else
      {
        ciMap.MovePawn( this, ciExitPos,
                        rumIgnoreTileCollisionMoveFlag | rumIgnorePawnCollisionMoveFlag | rumIgnoreDistanceMoveFlag );
      }

      ActionInfo( msg_jail_release_client_StringID );

      local eVersion = GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );

      // Give the player back their keys
      local iKeys = GetVersionedProperty( g_eConfiscatedKeysPropertyVersionArray, 0 );
      local ciProperty = ::rumGetPropertyAsset( g_eConfiscatedKeysPropertyVersionArray[eVersion] );
      iKeys = clamp( iKeys, ciProperty.GetMinValue(), ciProperty.GetMaxValue() );
      AdjustVersionedProperty( g_eKeysPropertyVersionArray, iKeys );
      RemoveVersionedProperty( g_eConfiscatedKeysPropertyVersionArray );

      // Give the player back their gold
      local iGold = GetVersionedProperty( g_eConfiscatedGoldPropertyVersionArray, 0 );
      ciProperty = ::rumGetPropertyAsset( g_eConfiscatedGoldPropertyVersionArray[eVersion] );
      iGold = clamp( iGold, ciProperty.GetMinValue(), ciProperty.GetMaxValue() );
      AdjustVersionedProperty( g_eGoldPropertyVersionArray, iGold );
      RemoveVersionedProperty( g_eConfiscatedGoldPropertyVersionArray );

      // Don't release a starving player to the wild
      local iFood = GetVersionedProperty( g_eFoodPropertyVersionArray );
      if( iFood < 5 )
      {
        AdjustVersionedProperty( g_eFoodPropertyVersionArray, 5 );
      }
    }
  }


  function RemoveArmour()
  {
    if( m_eArmourType != rumInvalidAssetID )
    {
      local ciArmour = GetEquippedArmour();
      ciArmour.RemoveProperty( Inventory_Equipped_PropertyID );
      SetProperty( Equipped_Armour_PropertyID, rumInvalidGameID );
      m_eArmourType = rumInvalidAssetID;
    }
  }


  function RemoveEffects()
  {
    base.RemoveEffects();

    RemoveProperty( U4_Shrine_Daemon_Immunity_PropertyID );
    local ciEffect = GetEffect( Daemon_Immunity_Effect );
    if( ciEffect )
    {
      ciEffect.Remove();
    }

    // Remove player lighting
    ExtinguishLight();

    StopPeering();
  }


  function RemoveVersionedProperty( i_ePropertyArray )
  {
    local eVersion = GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
    local ePropertyID = i_ePropertyArray[eVersion];
    if( ePropertyID != null )
    {
      RemoveProperty( ePropertyID );
    }
  }


  function RemoveWeapon()
  {
    if( m_eWeaponType != rumInvalidAssetID )
    {
      local ciWeapon = GetEquippedWeapon();
      ciWeapon.RemoveProperty( Inventory_Equipped_PropertyID );
      SetProperty( Equipped_Weapon_PropertyID, rumInvalidGameID );
      m_eWeaponType = rumInvalidAssetID;
    }
  }


  function ResetReagentDiscovery()
  {
    RemoveProperty( Reagent_Discovery_Blocked_PropertyID );
  }


  function Resurrect( i_eResurrectionType )
  {
    local bResurrected = false;

    if( IsDead() )
    {
      if( ResurrectionType.Body == i_eResurrectionType )
      {
        local bBound = GetVersionedProperty( g_eSpiritBoundPropertyVersionArray );
        if( bBound )
        {
          // The Unique ID we expect to find stamped on the corpse
          local uiPlayerID = GetID();

          // Is the player over a corpse?
          local ciMap = GetMap();
          local ciPosData = ciMap.GetPositionData( GetPosition() );
          local ciCorpse;
          while( ciCorpse = ciPosData.GetNext( rumWidgetPawnType, U4_Body_Dead_WidgetID ) )
          {
            if( ciCorpse.IsVisible() )
            {
              local uiCorpseID = ciCorpse.GetProperty( Player_ID_PropertyID, -1 );
              if( uiCorpseID == uiPlayerID )
              {
                // Consume the binding
                RemoveVersionedProperty( g_eSpiritBoundPropertyVersionArray );

                // Clean up the existing corpse
                ciMap.RemovePawn( ciCorpse );

                SetVersionedProperty( g_eHitpointsPropertyVersionArray, 100 );

                bResurrected = true;
                ciPosData.Stop();
              }
            }
          }
        }
      }
      else if( ResurrectionType.Void == i_eResurrectionType )
      {
        local eVersion = GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );

        // Transfer the player to Lord British
        local ciMap = GetMap();
        local ciDestMap = GetOrCreateMap( this, g_eKingMapVersionArray[eVersion] );
        if( ciMap != null && ciDestMap != null )
        {
          // Determine the starting position based on version
          local eAssetID = g_eKingCreatureVersionArray[eVersion];

          local ciPawnArray = ciDestMap.GetAllPawns();
          foreach( ciPawn in ciPawnArray )
          {
            if( ciPawn.GetAssetID() == eAssetID )
            {
              // The resurrection position is always 1 tile directly south of LB
              local ciDestPos = ciPawn.m_ciOriginPos + GetDirectionVector( Direction.South );
              ciMap.TransferPawn( this, ciDestMap, ciDestPos );

              // Free Heal
              local iHitpoints = 1;
              switch( eVersion )
              {
                case GameType.Ultima4:
                case GameType.Ultima3:
                  iHitpoints = GetMaxHitpoints();
                  break;

                case GameType.Ultima2:
                case GameType.Ultima1:
                  iHitpoints = GetMaxHitpoints() / 10;
                  break;
              }

              SetVersionedProperty( g_eHitpointsPropertyVersionArray, iHitpoints );

              // Show cast effect on LB and affected player
              SendClientEffect( this, ClientEffectType.Cast );

              local ciBroadcast = ::rumCreate( Command_Result_BroadcastID,
                                               msg_lord_british_resurrect_client_StringID,
                                               g_strColorTagArray.Cyan );
              SendBroadcast( ciBroadcast );

              // Handle death penalties, but don't allow players to be completely bankrupted
              local iCurrentGold = GetVersionedProperty( g_eGoldPropertyVersionArray );
              if( iCurrentGold > 200 )
              {
                local fPlayerGold = iCurrentGold * 0.75;
                SetVersionedProperty( g_eGoldPropertyVersionArray, fPlayerGold.tointeger() );
              }

              local iCurrentFood = GetVersionedProperty( g_eFoodPropertyVersionArray );
              if( iCurrentFood > 20 )
              {
                local fPlayerFood = iCurrentFood * 0.75;
                SetVersionedProperty( g_eFoodPropertyVersionArray, fPlayerFood.tointeger() );
              }
              else if( iCurrentFood <= 20 )
              {
                // Give the player a little food
                SetVersionedProperty( g_eFoodPropertyVersionArray, 20 );
              }

              bResurrected = true;

              break;
            }
          }
        }
      }
      else if( ( ResurrectionType.Spell == i_eResurrectionType )       ||
               ( ResurrectionType.EnergyField == i_eResurrectionType ) ||
               ( ResurrectionType.Other == i_eResurrectionType ) )
      {
        SetVersionedProperty( g_eHitpointsPropertyVersionArray, 100 );
        bResurrected = true;
      }
      else
      {
        // Unknown resurrection type
        IncrementHackAttempts();
      }

      if( bResurrected )
      {
        SetMoveType( MoveType.Terrestrial );

        // Reschedule mana recharging
        ::rumSchedule( this, OnManaRestored, s_fManaRestoreInterval, ++m_iManaUpdateIndex );

        // Reschedule food consumption
        ::rumSchedule( this, OnMealRequired, s_fMealInterval, ++m_iMealUpdateIndex );

        // Let all clients know that the player has been resurrected
        local ciMap = GetMap();
        if( ciMap != null )
        {
          local ciBroadcast = ::rumCreate( Player_Resurrect_BroadcastID, GetID() );
          ciMap.SendRegional( ciBroadcast );
        }
      }
    }

    return bResurrected;
  }


  function ScheduleNotorietyUpdate()
  {
    local iNotoriety = GetVersionedProperty( g_eNotorietyPropertyVersionArray );
    if( iNotoriety > 0 )
    {
      // Schedule reduction in notoriety points over time
      ::rumSchedule( this, OnNotorietyReduced, s_fNotorietyReductionInterval, ++m_iNotorietyIndex );
    }
  }


  function SendJailUpdate()
  {
    local eVersion = GetProperty( Ultima_Version_PropertyID, GameType.Invalid );
    local ePropertyID = g_eNotorietyPropertyVersionArray[eVersion];
    local iNotoriety = GetProperty( ePropertyID, 0 );
    if( iNotoriety >= s_iNotorietyCriminalLevel && IsJailed() )
    {
      local ciBroadcast = ::rumCreate( Jail_Update_BroadcastID, iNotoriety - ( s_iNotorietyCriminalLevel - 1 ) );
      SendBroadcast( ciBroadcast );
    }
  }


  function SetVersionedProperty( i_ePropertyArray, i_vValue )
  {
    local eVersion = GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
    local ePropertyID = i_ePropertyArray[eVersion];
    if( ePropertyID != null )
    {
      SetProperty( ePropertyID, i_vValue );
    }
  }


  function StartAxiomTest()
  {
    local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, U4_AbyssCodexPhaseType.Axiom );
    SendBroadcast( ciBroadcast );
  }


  function StartPlaying( i_eMapID, i_ciPos )
  {
    // The player has to be initialized in order to play
    local eVersion = GetProperty( Ultima_Version_PropertyID, GameType.Invalid );
    local bInitialized = ( eVersion != GameType.Invalid );

    local ciBroadcast = ::rumCreate( Player_Initialize_BroadcastID, bInitialized, GetPlayerName() );
    ::rumSendPrivate( GetSocket(), ciBroadcast );

    if( !bInitialized )
    {
      // The player has never been fully initialized
      ::rumPlayerLogout( GetSocket() );
      return false;
    }

    // The player is ready to start the game
    local ciMap = GetOrCreateMap( this, i_eMapID );
    if( ciMap != null )
    {
      // Attempt to add the pawn to the map
      if( !ciMap.AddPawn( this, i_ciPos ) )
      {
        // Try to move the player to their default starting location
        // TODO - determine which Ultima version the player is on, because that will affect where they are moved
        local ciPlayerClass = GetPlayerClass();
        if( null == ciPlayerClass )
        {
          return false;
        }

        local eMapID = ciPlayerClass.GetProperty( Class_Starting_Map_ID_PropertyID, rumInvalidAssetID );
        local iPosX = ciPlayerClass.GetProperty( Class_Starting_Pos_X_PropertyID, 0 );
        local iPosY = ciPlayerClass.GetProperty( Class_Starting_Pos_Y_PropertyID, 0 );

        ciMap = GetOrCreateMap( this, eMapID );
        if( ( ciMap == null ) || !ciMap.AddPawn( this, rumPos( iPosX, iPosY ) ) )
        {
          // Couldn't put the player on a map
          return false;
        }
      }

      HandleWorldUpdate( eVersion );

      // Update moon phases
      local ciBroadcast = ::rumCreate( Moon_Phases_BroadcastID, g_ciServer.m_ciUltima4World.m_eMoonTrammel,
                                       g_ciServer.m_ciUltima4World.m_eMoonFelucca );
      ::rumSendPrivate( GetSocket(), ciBroadcast );

      // Schedule food consumption
      ::rumSchedule( this, OnMealRequired, s_fMealInterval, ++m_iMealUpdateIndex );

      // Resume negation effects
      local iLastNegateTime = GetProperty( Last_Negate_Time_PropertyID, 0 );
      if( iLastNegateTime > 0 )
      {
        local iTimeSinceNegate = ::rumGetSecondsSinceEpoch() - iLastNegateTime;
        local fTimeSinceNegate = iTimeSinceNegate.tofloat();
        if( fTimeSinceNegate < Negate_Effect.s_fDuration )
        {
          // Call base because we just want to resume negation
          base.Negate( Negate_Effect.s_fDuration - fTimeSinceNegate );
        }
        else
        {
          RemoveProperty( Last_Negate_Time_PropertyID );
        }
      }

      // Schedule notoriety updates
      ScheduleNotorietyUpdate();

      // Start processing incomming packets
      PopPacket();

      return true;
    }

    ::rumPlayerLogout( GetSocket() );
    return false;
  }


  function StartPrincipleTest()
  {
    local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, U4_AbyssCodexPhaseType.Principle1 );
    SendBroadcast( ciBroadcast );
  }


  function StopDialogue( i_eTerminationType )
  {
    if( m_uiInteractID != rumInvalidGameID )
    {
      local ciNPC = ::rumFetchPawn( m_uiInteractID );
      if( ciNPC != null )
      {
        ciNPC.TalkEnd( this, i_eTerminationType );
      }
    }

    // TODO - cleanup merchant transactions
    //local ciBroadcast = ::rumCreate( Merchant_Stable_BroadcastID, MerchantStableTransaction.ServerTerminated );
    //i_ciPlayer.SendBroadcast( ciBroadcast );
  }


  // This will stop all active peering effects
  function StopPeering()
  {
    local ciEffect = GetEffect( Peer_Effect );
    if( ciEffect )
    {
      ciEffect.ExpireAll();
    }
  }


  function SummonLastTransport()
  {
    local ciTransport = null;

    // Is there a transport that is associated with the player?
    local uiTransportCode = GetVersionedProperty( g_eTransportCodePropertyVersionArray, 0 );
    if( uiTransportCode != 0 )
    {
      // Attempt to find or create the associated transport
      local eMapID = GetVersionedProperty( g_eTransportMapPropertyVersionArray, rumInvalidAssetID );
      local ciMap = GetMap();
      if( ciMap.GetAssetID() == eMapID )
      {
        local eWidgetID = GetVersionedProperty( g_eTransportWidgetPropertyVersionArray, rumInvalidAssetID );
        if( eWidgetID != rumInvalidAssetID )
        {
          ciTransport = FindTransport( ciMap, uiTransportCode );
          if( null == ciTransport )
          {
            // Recreate the transport
            local ciPos = GetPosition();
            local ciDestPos = rumPos( GetVersionedProperty( g_eTransportPosXPropertyVersionArray, ciPos.x ),
                                      GetVersionedProperty( g_eTransportPosYPropertyVersionArray, ciPos.y ) );
            ciTransport = CreateTransport( eWidgetID, uiTransportCode, ciMap, ciDestPos );
          }
        }
      }
    }

    return ciTransport;
  }


  function WindOverrideExpire()
  {
    // Determine if the player is still affected by a wind direction override
    local eDir = GetProperty( U4_Wind_Direction_Override_PropertyID, Direction.None );
    if( Direction.None != eDir )
    {
      // Has the time limit expired?
      local fExpirationTime = GetProperty( U4_Wind_Expiration_Time_PropertyID, 0.0 );
      local fDiffTime = fExpirationTime - g_ciServer.m_fServerTime;
      if( fDiffTime < 0.0 )
      {
        // The wind override has expired
        SetProperty( U4_Wind_Direction_Override_PropertyID, Direction.None );
      }
      else
      {
        // The wind overrid has not yet expired, re-schedule
        ::rumSchedule( this, WindOverrideExpire, fDiffTime );
      }
    }
  }
}
