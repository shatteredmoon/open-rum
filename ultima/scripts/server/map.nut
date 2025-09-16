class Map extends rumMap
{
  static s_bPrivateInstance = false;

  // The amount of time that must pass before all objects in a trigger group are restored to their default visibility
  static s_fResetGroupTime = 120.0;
  static s_fSpawnPlayerCheck = 5.0;
  static s_fSpawnCleanupCheck = 60.0;
  static s_iSpawnPlayerDistance = 32;

  static s_cArmourStealModifier = 52;
  static s_cGoldStealModifier = 12;
  static s_cMagicStealModifier = 20;
  static s_cOracleStealModifier = 80;
  static s_cRationStealModifier = 16;
  static s_cWeaponStealModifier = 40;

  static s_fInnOccupancyInterval = 60.0;
  static s_fInnInvasionMinimumTime = 10.0;
  static s_iInnInvasionVariance = 30;

  static s_fUnloadInterval = 300.0; // 5 minutes

  // Associates a group id with the time that group was last triggered
  m_fGroupTriggerTimerTable = null;

  m_iDeactivationIndex = 0;


  constructor()
  {
    base.constructor();
    m_fGroupTriggerTimerTable = {};
  }


  function Deactivate( i_iDeactivationIndex )
  {
    if( m_iDeactivationIndex != i_iDeactivationIndex )
    {
      // A newer deactivation request is pending, so ignore this one
      return;
    }

    // Verify that the map still doesn't have any players
    if( GetNumPlayers() == 0 )
    {
      // Verify that this map can be deactivated
      local bAlwaysActive = GetProperty( Map_Always_Active_PropertyID, false );
      if( !bAlwaysActive )
      {
        ReleaseMap( this );
      }
    }
  }


  function GetJailMap()
  {
    local eMapID = GetProperty( Map_Jail_Destination_PropertyID, rumInvalidAssetID )
    if( rumInvalidAssetID == eMapID )
    {
      local eVersion = GetProperty( Ultima_Version_PropertyID, 0 );
      switch( eVersion )
      {
        case GameType.Ultima4: eMapID = U4_Castle_British_2_MapID;             break;
        case GameType.Ultima3: eMapID = U3_Castle_Lord_British_MapID;          break;
        case GameType.Ultima2: eMapID = U2_Earth_AD_Castle_Lord_British_MapID; break;
        case GameType.Ultima1: eMapID = U1_Castle_Lord_British_MapID;          break;
      }
    }

    return eMapID;
  }


  function GroupTriggered( i_iGroup )
  {
    // Mark the time this group was triggered. By saving the time, scheduled requests to reset a group when there
    // were other triggers to the same group can be ignored.
    m_fGroupTriggerTimerTable[i_iGroup] <- g_ciServer.m_fServerTime;
  }


  function OnFree()
  {
    // TODO - 10/29/2023 This causes a thrown exception during server shutdown
    /*local ciPawnArray = GetAllPawns();
    foreach( ciPawn in ciPawnArray )
    {
      if( ciPawn instanceof NPC )
      {
        // Prevent the scheduler from updating pawn AI while the map is unloading
        ciPawn.m_iActionIndex = -1;
      }
    }*/
  }


  function OnInnVacancy( i_iRoomIndex )
  {
    SetInnRoomVacant( this, i_iRoomIndex, true );
  }


  function OnPlayerAdded( i_ciPlayer )
  {
    if( null == i_ciPlayer )
    {
      return;
    }

    local bSoloInstance = GetProperty( Map_Solo_Instance_PropertyID, false );
    if( bSoloInstance )
    {
      // Remove the player from their party if they're in one
      local ciParty = i_ciPlayer.GetParty();
      if( ciParty != null )
      {
        ciParty.Dismiss( i_ciPlayer, PartyBroadcastType.PlayerLeft );
      }
    }
  }


  function OnPlayerRemoved( i_ciPlayer )
  {
    if( null == i_ciPlayer )
    {
      return;
    }

    if( GetNumPlayers() == 0 )
    {
      local bAlwaysActive = GetProperty( Map_Always_Active_PropertyID, false );
      if( !bAlwaysActive )
      {
        // Schedule an unload for the map after a timeout interval
        ::rumSchedule( this, Deactivate, s_fUnloadInterval, ++m_iDeactivationIndex );

        local strInfo = "Map " + GetName() + " has reached 0 players and will unload in " + s_fUnloadInterval +
                        " second(s)";
        ::rumLog( strInfo );
      }
    }
  }


  function ResetExpiredTriggerGroups()
  {
    // Visit all trigger group and only reset object in that group if the reset interval has passed
    foreach( iGroupID, fTimeTriggered in m_fGroupTriggerTimerTable )
    {
      // Check the last time the group was triggered against the rest interval
      if( g_ciServer.m_fServerTime - m_fGroupTriggerTimerTable[iGroupID] > s_fResetGroupTime )
      {
        // Restore default visibility of all items in this group
        local ciPawnArray = GetAllPawns();
        foreach( ciPawn in ciPawnArray )
        {
          if( ( ciPawn instanceof Widget ) && ( ciPawn.m_iGroupID == iGroupID ) )
          {
            // Restore default visibility
            ciPawn.SetVisibility( ciPawn.m_bDefaultVisibility );
          }
        }
      }
    }
  }


  function Search( i_ciPlayer )
  {
    local bFound = false;

    local ciMap = i_ciPlayer.GetMap();
    local ciPos = i_ciPlayer.GetPosition();
    local ciPosData = ciMap.GetPositionData( ciPos );
    local ciWidget;
    while( ciWidget = ciPosData.GetNext( rumWidgetPawnType ) )
    {
      if( ( null == ciWidget ) || ciWidget.GetAssetID() != ItemCache_WidgetID )
      {
        continue;
      }

      // Reagents
      local bGrantsReagent = ciWidget.GetProperty( ItemCache_Grants_Reagent_PropertyID, false );
      if( bGrantsReagent && g_ciServer.m_ciUltima4World.IsDarkestNight() )
      {
        local bDiscoveryBlocked = i_ciPlayer.GetProperty( Reagent_Discovery_Blocked_PropertyID, false );
        if( !bDiscoveryBlocked )
        {
          local eReagentType = ciWidget.GetProperty( ItemCache_Reagent_Type_PropertyID, Reagents.Sulphurous_Ash );
          i_ciPlayer.DiscoverReagent( eReagentType );
          bFound = true;
        }
      }

      // Rune Materials
      local bGrantsMaterials = ciWidget.GetProperty( ItemCache_Grants_Rune_Materials_PropertyID, false );
      if( bGrantsMaterials )
      {
        local eMaterialType = ciWidget.GetProperty( ItemCache_Rune_Material_Type_PropertyID,
                                                    RuneMaterialType.Gemstone );
        local iMaterialFlags = i_ciPlayer.GetProperty( U4_Item_Rune_Materials_PropertyID, 0 );
        if( !::rumBitOn( iMaterialFlags, eMaterialType ) )
        {
          // Add the rune material to the player
          iMaterialFlags = ::rumBitSet( iMaterialFlags, eMaterialType );
          i_ciPlayer.SetProperty( U4_Item_Rune_Materials_PropertyID, iMaterialFlags );

          // Notify the client
          i_ciPlayer.ActionInfo( g_eU4RuneMaterialStringArray[eMaterialType] );

          bFound = true;
        }
      }

      // Stones
      local bGrantsStone = ciWidget.GetProperty( ItemCache_Grants_Stone_PropertyID, false );
      if( bGrantsStone )
      {
        local eStoneType = ciWidget.GetProperty( ItemCache_Stone_Type_PropertyID, StoneType.Blue );
        local iStoneFlags = i_ciPlayer.GetProperty( U4_Item_Stones_PropertyID, 0 );
        if( !::rumBitOn( iStoneFlags, eStoneType ) )
        {
          // The black stone can only be found on the darkest night
          if( eStoneType != StoneType.Black || g_ciServer.m_ciUltima4World.IsDarkestNight() )
          {
            // Add the stone to the player
            iStoneFlags = ::rumBitSet( iStoneFlags, eStoneType );
            i_ciPlayer.SetProperty( U4_Item_Stones_PropertyID, iStoneFlags );

            // Notify the client
            i_ciPlayer.ActionInfo( g_eU4StoneStringArray[eStoneType] );

            bFound = true;
          }
        }
      }

      // Bell of Courage
      local bGrantsBell = ciWidget.GetProperty( ItemCache_Grants_Bell_PropertyID, false );
      if( bGrantsBell )
      {
        local eState = i_ciPlayer.GetProperty( U4_Item_Bell_PropertyID, U4_QuestItemState.NotFound );
        if( U4_QuestItemState.NotFound == eState )
        {
          // Add the bell to the player
          i_ciPlayer.SetProperty( U4_Item_Bell_PropertyID, U4_QuestItemState.Found );

          // Notify the client
          i_ciPlayer.ActionInfo( msg_bell_found_client_StringID );

          bFound = true;
        }
      }

      // Book of Truth
      local bGrantsBook = ciWidget.GetProperty( ItemCache_Grants_Book_PropertyID, false );
      if( bGrantsBook )
      {
        local eState = i_ciPlayer.GetProperty( U4_Item_Book_PropertyID, U4_QuestItemState.NotFound );
        if( U4_QuestItemState.NotFound == eState )
        {
          // Add the book to the player
          i_ciPlayer.SetProperty( U4_Item_Book_PropertyID, U4_QuestItemState.Found );

          // Notify the client
          i_ciPlayer.ActionInfo( msg_book_found_client_StringID );

          bFound = true;
        }
      }

      // Candle of Love
      local bGrantsCandle = ciWidget.GetProperty( ItemCache_Grants_Candle_PropertyID, false );
      if( bGrantsCandle )
      {
        local eState = i_ciPlayer.GetProperty( U4_Item_Candle_PropertyID, U4_QuestItemState.NotFound );
        if( U4_QuestItemState.NotFound == eState )
        {
          // Add the book to the player
          i_ciPlayer.SetProperty( U4_Item_Candle_PropertyID, U4_QuestItemState.Found );

          // Notify the client
          i_ciPlayer.ActionInfo( msg_candle_found_client_StringID );

          bFound = true;
        }
      }

      // Daemon Horn
      local bGrantsHorn = ciWidget.GetProperty( ItemCache_Grants_Horn_PropertyID, false );
      if( bGrantsHorn )
      {
        local eState = i_ciPlayer.GetProperty( U4_Item_Silver_Horn_PropertyID, U4_QuestItemState.NotFound );
        if( U4_QuestItemState.NotFound == eState )
        {
          // Add the horn to the player
          i_ciPlayer.SetProperty( U4_Item_Silver_Horn_PropertyID, U4_QuestItemState.Found );

          // Notify the client
          i_ciPlayer.ActionInfo( msg_horn_found_client_StringID );

          bFound = true;
        }
      }

      // Mondain Skull Fragment
      local bGrantsSkullFragment = ciWidget.GetProperty( ItemCache_Grants_Skull_PropertyID, false );
      if( bGrantsSkullFragment && g_ciServer.m_ciUltima4World.IsDarkestNight() )
      {
        local eState = i_ciPlayer.GetProperty( U4_Item_Skull_Fragment_PropertyID, U4_QuestItemState.NotFound );
        if( U4_QuestItemState.NotFound == eState )
        {
          // Add the skull fragment to the player
          i_ciPlayer.SetProperty( U4_Item_Skull_Fragment_PropertyID, U4_QuestItemState.Found );

          // Notify the client
          i_ciPlayer.ActionInfo( msg_skull_found_client_StringID );

          bFound = true;
        }
      }

      // Mystic Armour
      local bGrantsMysticArmour = ciWidget.GetProperty( ItemCache_Grants_Mystic_Armour_PropertyID, false );
      if( bGrantsMysticArmour && i_ciPlayer.IsEightPartsAvatar() )
      {
        local ciInventory = i_ciPlayer.GetInventory();
        local ciItem = ciInventory.GetNext( U4_Robe_Mystic_Armour_InventoryID );
        if( null == ciItem )
        {
          i_ciPlayer.AddOrCreateItem( U4_Robe_Mystic_Armour_InventoryID );
          i_ciPlayer.ActionInfo( msg_mystic_robe_client_StringID );

          bFound = true;
        }
      }

      // Mystic Weapon
      local bGrantsMysticWeapon = ciWidget.GetProperty( ItemCache_Grants_Mystic_Weapon_PropertyID, false );
      if( bGrantsMysticWeapon && i_ciPlayer.IsEightPartsAvatar() )
      {
        local ciInventory = i_ciPlayer.GetInventory();
        local ciItem = ciInventory.GetNext( U4_Sword_Mystic_Weapon_InventoryID );
        if( null == ciItem )
        {
          i_ciPlayer.AddOrCreateItem( U4_Sword_Mystic_Weapon_InventoryID );
          i_ciPlayer.ActionInfo( msg_mystic_sword_client_StringID );

          bFound = true;
        }
      }

      // HMS Cape Wheel
      local bGrantsHMSWheel = ciWidget.GetProperty( ItemCache_Grants_HMS_Wheel_PropertyID, false );
      if( bGrantsHMSWheel )
      {
        i_ciPlayer.ActionInfo( msg_imbued_wheel_client_StringID );
        local ciTransport = i_ciPlayer.GetTransport();
        if( ciTransport != null )
        {
          ciTransport.SetProperty( U4_HMS_Cape_Imbued_PropertyID, true );
        }

        bFound = true;
      }

      // Cards
      local bGrantsCard = ciWidget.GetProperty( ItemCache_Grants_Card_PropertyID, false );
      if( bGrantsCard )
      {
        local eCardType = ciWidget.GetProperty( ItemCache_Card_Type_PropertyID, U3_CardType.Death );
        local eCardFlags = i_ciPlayer.GetProperty( U3_Cards_PropertyID, 0 );
        if( !::rumBitOn( eCardFlags, eCardType ) )
        {
          // Add the card to the player
          eCardFlags = ::rumBitSet( eCardFlags, eCardType );
          i_ciPlayer.SetProperty( U3_Cards_PropertyID, eCardFlags );

          // Notify the client
          i_ciPlayer.ActionInfo( g_eU3CardStringArray[eCardType] );

          bFound = true;
        }
      }
    }

    if( !bFound && ( ciMap.GetAssetID() == U3_Sosaria_MapID ) )
    {
      // Is the player searching for Exotics?
      local ciExoticWeapon = i_ciPlayer.GetInventoryByType( U3_Exotic_Weapon_InventoryID );
      local bHasExoticWeapon = ( ciExoticWeapon != null );
      if( !bHasExoticWeapon )
      {
        local iWeaponIndex = i_ciPlayer.GetProperty( U3_Exotic_Weapon_Pos_Index_PropertyID, -1 );
        if( iWeaponIndex != -1 )
        {
          local ciWeaponPosition = rumPos( g_uiU3ExoticPosXArray[iWeaponIndex], g_uiU3ExoticPosYArray[iWeaponIndex] );
          if( ( ciPos.x == ciWeaponPosition.x ) && ( ciPos.y == ciWeaponPosition.y ) )
          {
            i_ciPlayer.AddOrCreateItem( U3_Exotic_Weapon_InventoryID );
            i_ciPlayer.ActionInfo( msg_exotic_arms_client_StringID );
            bFound = true;
          }
        }
      }

      if( !bFound )
      {
        local ciExoticArmour = i_ciPlayer.GetInventoryByType( U3_Exotic_Armour_InventoryID );
        local bHasExoticArmour = ( ciExoticArmour != null );
        if( !bHasExoticArmour )
        {
          local iArmourIndex = i_ciPlayer.GetProperty( U3_Exotic_Armour_Pos_Index_PropertyID, -1 );
          if( iArmourIndex != -1 )
          {
            local ciArmourPosition = rumPos( g_uiU3ExoticPosXArray[iArmourIndex], g_uiU3ExoticPosYArray[iArmourIndex] );
            if( ( ciPos.x == ciArmourPosition.x ) && ( ciPos.y == ciArmourPosition.y ) )
            {
              i_ciPlayer.AddOrCreateItem( U3_Exotic_Armour_InventoryID );
              i_ciPlayer.ActionInfo( msg_exotic_armour_client_StringID );
              bFound = true;
            }
          }
        }
      }
    }

    if( !bFound )
    {
      i_ciPlayer.ActionFailed( msg_nothing_here_client_StringID );
    }
  }


  function Spawn( i_ciSpawner, i_bForce = false )
  {
    local iNumSpawned = 0;

    local iSpawnArray = i_ciSpawner.GetSpawnArray();

    local iNearestDistance = null;
    local ciNearestPlayer = null;
    local iNumNearbyPlayers = 0;

    local ciPos = i_ciSpawner.GetPosition();

    local ciPlayerArray = GetAllPlayers();
    foreach( ciPlayer in ciPlayerArray )
    {
      local iDistance = GetTileDistance( ciPlayer.GetPosition(), ciPos );
      if( iDistance < s_iSpawnPlayerDistance )
      {
        ++iNumNearbyPlayers;
      }

      if( ( null == iNearestDistance ) || iDistance < iNearestDistance )
      {
        iNearestDistance = iDistance;
        ciNearestPlayer = ciPlayer;
      }
    }

    if( i_bForce || ( iNumNearbyPlayers > 0 && ciNearestPlayer != null ) )
    {
      if( i_bForce )
      {
        iNumNearbyPlayers = 1;
      }
      else
      {
        local ciParty = ciNearestPlayer.GetParty();
        if( ciParty != null )
        {
          // Determine how many objects to spawn based on player group size
          iNumNearbyPlayers = ciParty.NumMembers();
        }
      }

      // Get a random object from the spawner
      local iIndex = rand() % iSpawnArray.len();
      local ePawnID = iSpawnArray[iIndex];

      // Discrete spawners will only spawn a single object at the spawner location
      local iNumToSpawn = 1;
      local bDiscrete = i_ciSpawner.GetProperty( Spawn_Discrete_PropertyID, false );
      if( !bDiscrete )
      {
        // Spawn a minimum of the number of players, or more up to 12 using a scatter plot
        iNumToSpawn = min( 2 * iNumNearbyPlayers, 12 );
        iNumToSpawn = rand() % iNumToSpawn + 1;

        local ciAsset = ::rumGetAsset( ePawnID );
        if( null == ciAsset )
        {
          return;
        }

        // Could be a limit on how many things can spawn
        local iSpawnLimit = ciAsset.GetProperty( Spawn_Limit_PropertyID, 99 );
        iNumToSpawn = min( iNumToSpawn, iSpawnLimit );
      }

      // TODO - leader types? for instance orcs sometimes accompanied by trolls, etc.

      for( local i = 0; i < iNumToSpawn; ++i )
      {
        // Create the new pawn and determine placement
        local ciPawn = ::rumCreate( ePawnID );
        if( null == ciPawn )
        {
          continue;
        }

        local bPlaced = false;

        if( bDiscrete )
        {
          // Add the pawn to the map
          if( rumSuccessMoveResultType == MovePawn( ciPawn, ciPos,
                                                    rumIgnoreDistanceMoveFlag | rumIgnorePawnCollisionMoveFlag |
                                                    rumTestMoveFlag ) )
          {
            bPlaced = AddPawn( ciPawn, ciPos );
          }
        }
        else
        {
          local iNumTries = 0;

          while( !bPlaced && iNumTries < 3 )
          {
            // How far away from the spawner position something can be spawned
            local iScatterDist = i_ciSpawner.s_iScatterDistance * 2 + 1;
            local iOffsetX = rand() % iScatterDist;
            local iOffsetY = rand() % iScatterDist;
            local ciNewPos = rumPos( ciPos.x + iOffsetX - i_ciSpawner.s_iScatterDistance,
                                     ciPos.y + iOffsetY - i_ciSpawner.s_iScatterDistance );

            // Add the pawn to the map
            if( rumSuccessMoveResultType == MovePawn( ciPawn, ciNewPos,
                                                      rumIgnoreDistanceMoveFlag | rumIgnorePawnCollisionMoveFlag |
                                                      rumTestMoveFlag ) )
            {
              bPlaced = AddPawn( ciPawn, ciNewPos );
            }

            ++iNumTries;
          }
        }

        if( bPlaced )
        {
          if( ciPawn instanceof NPC )
          {
            ciPawn.m_ciOriginPos = ciPawn.GetPosition();
            ciPawn.m_eDefaultPosture = PostureType.Attack;
            ciPawn.m_eDefaultNpcType = NPCType.Standard;

            local eAlignment = ciPawn.GetProperty( Alignment_PropertyID, AlignmentType.Good );
            if( AlignmentType.Good == eAlignment )
            {
              ciPawn.SetProperty( Alignment_PropertyID, AlignmentType.Evil );
            }

            ::rumSchedule( ciPawn, ciPawn.AIDetermine, frandVariance( ciPawn.m_fTimeDecideVariance ),
                           ++ciPawn.m_iActionIndex );
          }

          // Add to spawned table
          i_ciSpawner.m_uiSpawnTable[ciPawn.GetID()] <- ciPawn.GetID();

          ++iNumSpawned;
        }
      }
    }

    if( iNumSpawned > 0 )
    {
      ::rumSchedule( i_ciSpawner, i_ciSpawner.CleanupCheck, s_fSpawnCleanupCheck );
    }
    else
    {
      // Schedule future spawn check
      ::rumSchedule( i_ciSpawner, i_ciSpawner.Spawn, s_fSpawnPlayerCheck );
    }
  }


  function SpawnCleanup( i_ciSpawner )
  {
    local bPlayerNear = false;
    local ciPos = i_ciSpawner.GetPosition();

    local ciPlayerArray = GetAllPlayers();
    foreach( ciPlayer in ciPlayerArray )
    {
      local iDistance = GetTileDistance( ciPlayer.GetPosition(), ciPos );
      if( iDistance < s_iSpawnPlayerDistance )
      {
        bPlayerNear = true;
        break;
      }
    }

    if( !bPlayerNear )
    {
      // Clean up this spawner
      foreach( uiPawnID in i_ciSpawner.m_uiSpawnTable )
      {
        if( uiPawnID != rumInvalidGameID )
        {
          local ciPawn = ::rumFetchPawn( uiPawnID );
          if( ciPawn != null )
          {
            // This will also free the object from memory
            RemovePawn( ciPawn );
          }
        }
      }

      i_ciSpawner.m_uiSpawnTable.clear();

      // Schedule future spawn check
      ::rumSchedule( i_ciSpawner, i_ciSpawner.Spawn, s_fSpawnPlayerCheck );
    }
    else
    {
      // Schedule the next cleanup check
      ::rumSchedule( i_ciSpawner, i_ciSpawner.CleanupCheck, s_fSpawnCleanupCheck );
    }
  }


  function Steal( i_ciCreature, i_ciPlayer )
  {
    switch( i_ciCreature.m_iDialogueID )
    {
      case MerchantType.Weapons:  TransactWeaponSteal( i_ciCreature, i_ciPlayer );  break;
      case MerchantType.Armour:   TransactArmourSteal( i_ciCreature, i_ciPlayer );  break;
      //case MerchantType.Healer: TransactHealerSteal(i_ciCreature, i_ciPlayer);    break;
      case MerchantType.Rations:  TransactRationsSteal( i_ciCreature, i_ciPlayer ); break;
      //case MerchantType.Guild:  TransactGuildSteal(i_ciCreature, i_ciPlayer);     break;
      case MerchantType.Magic:    TransactMagicSteal( i_ciCreature, i_ciPlayer );   break;
      default:                    TransactGoldSteal( i_ciCreature, i_ciPlayer );    break;
    }
  }


  function Transact( i_eMerchantType, i_ciPlayer )
  {
    switch( i_eMerchantType )
    {
      case MerchantType.Armour:    TransactArmourStart( i_ciPlayer );    break;
      case MerchantType.Inn:       TransactInnStart( i_ciPlayer );       break;
      case MerchantType.Guild:     TransactGuildStart( i_ciPlayer );     break;
      case MerchantType.Healer:    TransactHealerStart( i_ciPlayer );    break;
      case MerchantType.Magic:     TransactMagicStart( i_ciPlayer );     break;
      case MerchantType.Oracle:    TransactOracleStart( i_ciPlayer );    break;
      case MerchantType.Rations:   TransactRationsStart( i_ciPlayer );   break;
      case MerchantType.Reagents:  TransactReagentsStart( i_ciPlayer );  break;
      case MerchantType.Stable:    TransactStableStart( i_ciPlayer );    break;
      case MerchantType.Tavern:    TransactTavernStart( i_ciPlayer );    break;
      case MerchantType.Transport: TransactTransportStart( i_ciPlayer ); break;
      case MerchantType.Weapons:   TransactWeaponStart( i_ciPlayer );    break;

      default:
        // Attempted transaction with an unsupported MerchantType
        i_ciPlayer.IncrementHackAttempts();
        break;
    }
  }


  function TransactGoldSteal( i_ciCreature, i_ciPlayer )
  {
    local bSuccess = i_ciPlayer.AttemptSteal( i_ciCreature, s_cGoldStealModifier );
    if( bSuccess )
    {
      // Player steals 1-20 gold/coin
      local iStolenAmount = rand() % 20 + 1;
      i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, iStolenAmount );

      local ciBroadcast = ::rumCreate( Player_Steal_BroadcastID, Gold_PropertyID, iStolenAmount );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
  }
}
