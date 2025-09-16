class U1_GameTest
{
  m_strOutput = "";
  m_ciPlayer = null;


  constructor( i_ciPlayer )
  {
    if( g_ciGameTest != null )
    {
      // Test is already underway
      return;
    }

    m_strOutput = "";
    m_ciPlayer = null;

    if( i_ciPlayer != null )
    {
      m_ciPlayer = i_ciPlayer;

      m_ciPlayer.ChangeWorld( GameType.Ultima1 );

      if( m_ciPlayer.IsDead() )
      {
        m_ciPlayer.SetVersionedProperty( g_eHitpointsPropertyVersionArray, 100 );
      }

      // Initialize properties
      m_ciPlayer.SetProperty( U1_Experience_PropertyID, 0 );
      m_ciPlayer.SetProperty( U1_Coin_PropertyID, 0 );
      m_ciPlayer.SetProperty( U1_Keys_PropertyID, 0 );
      m_ciPlayer.RemoveProperty( U1_Gems_PropertyID );
      m_ciPlayer.RemoveProperty( U1_Shuttle_Pass_PropertyID );
      m_ciPlayer.RemoveProperty( Invincible_PropertyID );
      m_ciPlayer.SetProperty( U1_Space_Enemies_Killed_PropertyID, 15 );

      local iFlags = m_ciPlayer.GetProperty( Ultima_Completions_PropertyID, 0 );
      iFlags = ::rumBitClear( iFlags, UltimaCompletions.KilledMondain );
      m_ciPlayer.SetProperty( Ultima_Completions_PropertyID, iFlags );

      // Quests
      foreach( ePropertyID in g_eU1KingQuestPropertyArray )
      {
        m_ciPlayer.RemoveProperty( ePropertyID );
      }

      // Stats
      foreach( ePropertyID in g_eU1StatPropertyArray )
      {
        m_ciPlayer.SetProperty( ePropertyID, 20 );
      }

      // Remove all weapons and armour
      m_ciPlayer.RemoveWeapon();
      m_ciPlayer.RemoveArmour();
      local ciInventory = m_ciPlayer.GetInventory();
      local ciItem;
      while( ciItem = ciInventory.GetNextObject() )
      {
        if( ciItem instanceof rumInventory )
        {
          m_ciPlayer.DeleteInventory( ciItem );
        }
      }

      local ciTransport = m_ciPlayer.GetTransport();
      if( ciTransport != null )
      {
        ciTransport.Exit( m_ciPlayer, ciTransport.GetPosition(), MoveType.Incorporeal );
      }

      ::rumSchedule( this, DeathTest, 0.5 );
    }
  }


  function BaratariaTest()
  {
    // Pillar of Ozymandias
    KingSignQuestTest( U1_Castle_Barataria_MapID, rumPos( 98, 66 ) );

    // The Signpost
    KingSignQuestTest( U1_Castle_Barataria_MapID, rumPos( 14, 89 ) );

    ::rumSchedule( this, LordBritishTest, 0.5, /* phase */ 1 );
  }


  function BlackDragonTest()
  {
    KingGemQuestTest( U1_Castle_Black_Dragon_MapID, rumPos( 25, 5 ),
                      U1_Dungeon_Mines_Mount_Drash_8_MapID, rumPos( 8, 4 ),
                      U1_Lich_CreatureID );

    KingGemQuestTest( U1_Castle_Black_Dragon_MapID, rumPos( 25, 5 ),
                      U1_Dungeon_End_10_MapID, rumPos( 6, 1 ),
                      U1_Zorn_CreatureID );

    // Test results
    local uiGemFlags = m_ciPlayer.GetProperty( U1_Gems_PropertyID, 0 );
    local bHasGem = ::rumBitOn( uiGemFlags, U1_GemType.Blue );
    m_strOutput += "Obtained blue gem: " + ( bHasGem ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, ShaminoTest, 0.5 );
  }


  function DeathTest()
  {
    local ciMap = m_ciPlayer.GetMap();

    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U1_Dungeon_Dead_Mans_Walk_6_MapID ), rumPos( 20, 20 ) );
    ciMap = m_ciPlayer.GetMap();

    m_ciPlayer.SetVersionedProperty( g_eHitpointsPropertyVersionArray, 1 );

    ::rumSchedule( this, LordBritishTest, 10.0, /* phase */ 0 );
  }


  function LordBritishTest( i_iPhase )
  {
    if( 0 == i_iPhase )
    {
      local ciMap = m_ciPlayer.GetMap();
      local iSocket = m_ciPlayer.GetSocket();

      m_strOutput += "Player is dead: " + ( m_ciPlayer.IsDead() ? "PASS" : "FAIL" ) + "\n";

      // Resurrect the player to Lord British
      local ciBroadcast = Player_Resurrect_Broadcast( ResurrectionType.Void );
      ciBroadcast.OnRecv( iSocket, m_ciPlayer );

      m_strOutput += "Player is alive: " + ( !m_ciPlayer.IsDead() ? "PASS" : "FAIL" ) + "\n";

      ::rumSchedule( this, LostKingTest, 0.5 );
    }
    if( 1 == i_iPhase )
    {
      // Tower of Knowledge
      KingSignQuestTest( U1_Castle_Lord_British_MapID, rumPos( 70, 10 ) );

      // Eastern Signpost
      KingSignQuestTest( U1_Castle_Lord_British_MapID, rumPos( 132, 87 ) );

      ::rumSchedule( this, OlympusTest, 0.5 );
    }
  }


  function LostKingTest()
  {
    KingGemQuestTest( U1_Castle_Lost_King_MapID, rumPos( 25, 5 ),
                      U1_Dungeon_Metal_Twister_4_MapID, rumPos( 3, 14 ),
                      U1_Gelatinous_Cube_CreatureID );

    KingGemQuestTest( U1_Castle_Lost_King_MapID, rumPos( 25, 5 ),
                      U1_Dungeon_Deaths_Awakening_10_MapID, rumPos( 27, 2 ),
                      U1_Daemon_CreatureID );

    // Test results
    local uiGemFlags = m_ciPlayer.GetProperty( U1_Gems_PropertyID, 0 );
    local bHasGem = ::rumBitOn( uiGemFlags, U1_GemType.Red );
    m_strOutput += "Obtained red gem: " + ( bHasGem ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, RondorinTest, 0.5 );
  }


  function KingGemQuestTest( i_ciKingMapID, i_ciKingPosition, i_ciQuestMapID, i_ciQuestPosition, i_eQuestTypeID )
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    // Travel to quest giver
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, i_ciKingMapID ), i_ciKingPosition );
    ciMap = m_ciPlayer.GetMap();

    // Acquire quest
    local ciBroadcast = Player_Talk_Broadcast( Direction.North );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciBroadcast = ::rumCreate( Player_Talk_U1_King_BroadcastID, U1_KingTalkType.Service );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciBroadcast = ::rumCreate( Player_Talk_Bye_BroadcastID, DialogueTerminationType.Standard );
    ::rumSendPrivate( iSocket, ciBroadcast );

    // Kill quest target
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, i_ciQuestMapID ), i_ciQuestPosition );
    ciMap = m_ciPlayer.GetMap();

    local ciPos = m_ciPlayer.GetPosition() + GetDirectionVector( Direction.West );
    local ciPosData = ciMap.GetPositionData( ciPos );
    local ciCreature = ciPosData.GetNext( rumCreaturePawnType, i_eQuestTypeID );
    if( ciCreature != null )
    {
      ciCreature.Damage( 999, m_ciPlayer, m_ciPlayer.GetWeaponType() );
    }

    // Return to quest giver
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, i_ciKingMapID ), i_ciKingPosition );

    // Complete quest
    ciBroadcast = Player_Talk_Broadcast( Direction.North );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciBroadcast = ::rumCreate( Player_Talk_U1_King_BroadcastID, U1_KingTalkType.Service );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciBroadcast = ::rumCreate( Player_Talk_Bye_BroadcastID, DialogueTerminationType.Standard );
    ::rumSendPrivate( iSocket, ciBroadcast );
  }


  function KingSignQuestTest( i_ciKingMapID, i_ciQuestPosition )
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    // Travel to quest giver
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, i_ciKingMapID ), rumPos( 33, 5 ) );
    ciMap = m_ciPlayer.GetMap();

    // Acquire quest
    local ciBroadcast = Player_Talk_Broadcast( Direction.North );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciBroadcast = ::rumCreate( Player_Talk_U1_King_BroadcastID, U1_KingTalkType.Service );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciBroadcast = ::rumCreate( Player_Talk_Bye_BroadcastID, DialogueTerminationType.Standard );
    ::rumSendPrivate( iSocket, ciBroadcast );

    // Use sign
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U1_Sosaria_MapID ), i_ciQuestPosition );
    ciMap = m_ciPlayer.GetMap();

    local ciPos = m_ciPlayer.GetPosition() + GetDirectionVector( Direction.West );
    local ciPosData = ciMap.GetPositionData( ciPos );
    local ciWidget = ciPosData.GetNext( rumWidgetPawnType, U1_Signpost_WidgetID );
    if( ciWidget != null )
    {
      ciWidget.Use( m_ciPlayer );
    }

    // Return to quest giver
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, i_ciKingMapID ), rumPos( 33, 5 ) );
    ciMap = m_ciPlayer.GetMap();

    // Complete quest
    ciBroadcast = Player_Talk_Broadcast( Direction.North );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciBroadcast = ::rumCreate( Player_Talk_U1_King_BroadcastID, U1_KingTalkType.Service );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciBroadcast = ::rumCreate( Player_Talk_Bye_BroadcastID, DialogueTerminationType.Standard );
    ::rumSendPrivate( iSocket, ciBroadcast );
  }


  function LaunchTest()
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    m_ciPlayer.SetVersionedProperty( g_eGoldPropertyVersionArray, 2000 );

    // Travel to transport merchant
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U1_Town_Britain_MapID ), rumPos( 6, 3 ) );
    ciMap = m_ciPlayer.GetMap();

    // Purchase shuttle pass
    TransactTransportRecv( m_ciPlayer, MerchantTransportTransaction.Purchase, U1_Shuttle_WidgetID );

    // Launch the shuttle
    local ciBroadcast = ::rumCreate( Player_Transport_BroadcastID, TransportCommandType.Movement,
                                     VerticalDirectionType.Up );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    // Create and equip necessary gear for space
    local bAdded = m_ciPlayer.AddOrCreateItem( U1_Vacuum_Suit_Armour_InventoryID, 1 );
    if( bAdded )
    {
      local ciInventory = m_ciPlayer.GetInventory();
      local ciItem;
      while( ciItem = ciInventory.GetNextObject() )
      {
        local eAssetID = ciItem.GetAssetID();
        local iItemID = ciItem.GetID();
        if( U1_Vacuum_Suit_Armour_InventoryID == eAssetID )
        {
          m_ciPlayer.EquipArmour( iItemID );
          ciInventory.Stop();
        }
      }
    }

    ::rumSchedule( this, SpaceAceTest, 11.0 );
  }


  function MondainTest( i_iPhase )
  {
    local ciMap = m_ciPlayer.GetMap();
    local ciDestMap = GetOrCreateMap( m_ciPlayer, U1_Lair_Mondain_MapID );

    local ciMondain = null;
    local ciGem = null;
    local ciTimeMachine = null;

    local ciPawnArray = ciDestMap.GetAllPawns();
    foreach( ciPawn in ciPawnArray )
    {
      local eAssetID = ciPawn.GetAssetID();
      if( U1_Gem_Immortality_WidgetID == eAssetID )
      {
        ciGem = ciPawn;
      }
      else if( U1_Mondain_CreatureID == eAssetID )
      {
        ciMondain = ciPawn;
      }
      else if( U1_Time_Machine_WidgetID == eAssetID )
      {
        ciTimeMachine = ciPawn;
      }
    }

    if( 0 == i_iPhase )
    {
      // Give the player a blaster
      local bAdded = m_ciPlayer.AddOrCreateItem( U1_Blaster_Weapon_InventoryID, 1 );
      if( bAdded )
      {
        local ciInventory = m_ciPlayer.GetInventory();
        local ciItem;
        while( ciItem = ciInventory.GetNextObject() )
        {
          local eAssetID = ciItem.GetAssetID();
          local uiItemID = ciItem.GetID();
          if( U1_Blaster_Weapon_InventoryID == eAssetID )
          {
            local uiMinAgility = ciItem.GetProperty( Inventory_Agility_Min_PropertyID, 0 );
            local uiCurrentAgility = m_ciPlayer.GetProperty( U1_Agility_PropertyID, 0 );
            if( uiCurrentAgility < uiMinAgility )
            {
              m_ciPlayer.SetProperty( U1_Agility_PropertyID, uiMinAgility );
            }

            m_ciPlayer.EquipWeapon( uiItemID );
            ciInventory.Stop();
          }
        }
      }

      // Travel to Mondain and inflict damage
      ciMap.TransferPawn( m_ciPlayer, ciDestMap, ciMondain.GetPosition() );
      ciMap = m_ciPlayer.GetMap();
      ciMondain.Damage( 9999, m_ciPlayer );

      // Destroy the gem
      ciMap.TransferPawn( m_ciPlayer, ciMap, ciGem.GetPosition() );
      ciGem.Damage( 9999, m_ciPlayer );

      ::rumSchedule( this, MondainTest, 0.5, /* phase */ 1 );
      return;
    }
    else if( 1 == i_iPhase )
    {
      // Force instant resurrection
      ciMondain.OnResurrect();

      // Travel to Mondain and inflict damage
      ciMap.TransferPawn( m_ciPlayer, ciMap, ciMondain.GetPosition() );
      ciMap = ciDestMap;
      ciMondain.Damage( 9999, m_ciPlayer );

      // Results
      local bGemDestroyed = !ciGem.IsVisible();
      m_strOutput += "Destroyed Gem of Immortality: " + ( bGemDestroyed ? "PASS" : "FAIL" ) + "\n";

      local bMondainKilled = ciMondain.m_bPermaDeath;
      m_strOutput += "Killed Mondain: " + ( bMondainKilled ? "PASS" : "FAIL" ) + "\n";

      // Travel to the time machine
      ciMap.TransferPawn( m_ciPlayer, ciMap, ciTimeMachine.GetPosition() );
      ciTimeMachine.Board( m_ciPlayer );

      ::rumSchedule( this, MondainTest, 0.5, /* phase */ 2 );
      return;
    }
    else
    {
      if( ciMap != null && ciMap.GetAssetID() != U1_Castle_Lord_British_MapID )
      {
        ::rumSchedule( this, MondainTest, 1.0, i_iPhase + 1 );
        return;
      }
    }

    ::rumSchedule( this, TestDone, 0.5 );
  }


  function OlympusTest()
  {
    // Southern Signpost
    KingSignQuestTest( U1_Castle_Olympus_MapID, rumPos( 13, 122 ) );

    // Pillars of Protection
    KingSignQuestTest( U1_Castle_Olympus_MapID, rumPos( 37, 9 ) );

    ::rumSchedule( this, WhiteDragonTest, 0.5 );
  }


  function PrincessTest( i_uiPhase )
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U1_Castle_Lord_British_MapID ), rumPos( 33, 5 ) );
    ciMap = m_ciPlayer.GetMap();

    // Find and steal from the jester
    local ciPawnArray = ciMap.GetAllPawns();
    foreach( ciPawn in ciPawnArray )
    {
      local eAssetID = ciPawn.GetAssetID();
      if( U1_Jester_CreatureID == eAssetID )
      {
        ciMap.TransferPawn( m_ciPlayer, ciMap, ciPawn.GetPosition() );
        if( m_ciPlayer.AttemptSteal( ciPawn, 0, true /* force */ ) )
        {
          ciPawn.OnSteal( m_ciPlayer );
        }
        break;
      }
    }

    local uiNumKeys = m_ciPlayer.GetProperty( U1_Keys_PropertyID, 0 );
    local bHasKey = ( uiNumKeys > 0 );
    m_strOutput += "Key stolen from jester: " + ( bHasKey ? "PASS" : "FAIL" ) + "\n";

    local ciTargetPos = rumPos( 35, 15 );

    // Gain access to the princess
    ciMap.TransferPawn( m_ciPlayer, ciMap, ciTargetPos );
    local ciPosData = ciMap.GetPositionData( m_ciPlayer.GetPosition() + GetDirectionVector( Direction.North ) );
    local ciWidget = null;
    while( ciWidget = ciPosData.GetNext( rumWidgetPawnType ) )
    {
      local eAssetID = ciWidget.GetAssetID();
      if( U1_Door_Princess_WidgetID == eAssetID )
      {
        ciWidget.Unlock( m_ciPlayer );
        ciPosData.Stop();
      }
    }

    local bEnteredCell = ( m_ciPlayer.GetPosition().y != ciTargetPos.y );
    m_strOutput += "Entered princess' cell: " + ( bEnteredCell ? "PASS" : "FAIL" ) + "\n";

    // Move fully into the cell
    ciMap.TransferPawn( m_ciPlayer, ciMap, m_ciPlayer.GetPosition() + GetDirectionVector( Direction.North ) );

    local ciPrincessPos = m_ciPlayer.GetPosition() + GetDirectionVector( Direction.North );
    Player_Talk_Broadcast.TryTalk( m_ciPlayer, ciMap, ciPrincessPos, 0 );

    if( 0 == i_uiPhase )
    {
      ciMap = m_ciPlayer.GetMap();
      local bRejected = ( ciMap.GetAssetID() == U1_Sosaria_MapID );
      m_strOutput += "Player rejected by princess: " + ( bRejected ? "PASS" : "FAIL" ) + "\n";

      m_ciPlayer.SetProperty( U1_Experience_PropertyID, 99999 );

      ::rumSchedule( this, PrincessTest, 0.5, /* phase */ 1 );
    }
    else if( 1 == i_uiPhase )
    {
      ciMap = m_ciPlayer.GetMap();
      local bAccepted = ( ciMap.GetAssetID() == U1_Time_Machine_MapID );
      m_strOutput += "Player accepted by princess: " + ( bAccepted ? "PASS" : "FAIL" ) + "\n";

      ::rumSchedule( this, TimeMachineTest, 0.5, /* phase */ 0 );
    }
  }


  function RondorinTest()
  {
    KingGemQuestTest( U1_Castle_Rondorin_MapID, rumPos( 25, 5 ),
                      U1_Dungeon_Lost_Caverns_6_MapID, rumPos( 25, 7 ),
                      U1_Carrion_Creeper_CreatureID );

    KingGemQuestTest( U1_Castle_Rondorin_MapID, rumPos( 25, 5 ),
                      U1_Dungeon_End_10_MapID, rumPos( 21, 17 ),
                      U1_Invisible_Seeker_CreatureID );

    // Test results
    local uiGemFlags = m_ciPlayer.GetProperty( U1_Gems_PropertyID, 0 );
    local bHasGem = ::rumBitOn( uiGemFlags, U1_GemType.Green );
    m_strOutput += "Obtained green gem: " + ( bHasGem ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, BlackDragonTest, 0.5 );
  }


  function ShaminoTest()
  {
    KingGemQuestTest( U1_Castle_Shamino_MapID, rumPos( 25, 5 ),
                      U1_Dungeon_Doom_10_MapID, rumPos( 3, 2 ),
                      U1_Balron_CreatureID );

    KingGemQuestTest( U1_Castle_Shamino_MapID, rumPos( 25, 5 ),
                      U1_Dungeon_Mondains_Gate_Hell_10_MapID, rumPos( 27, 2 ),
                      U1_Mind_Whipper_CreatureID );

    // Test results
    local uiGemFlags = m_ciPlayer.GetProperty( U1_Gems_PropertyID, 0 );
    local bHasGem = ::rumBitOn( uiGemFlags, U1_GemType.White );
    m_strOutput += "Obtained white gem: " + ( bHasGem ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, BaratariaTest, 0.5 );
  }


  function SpaceAceTest()
  {
    local ciMap = m_ciPlayer.GetMap();

    local ciTransport = m_ciPlayer.GetTransport();
    local ciSosaria = null;

    local ciPawnArray = ciMap.GetAllPawns();
    foreach( ciPawn in ciPawnArray )
    {
      local eAssetID = ciPawn.GetAssetID();
      if( U1_Spawner_Wilderness_Space_WidgetID == eAssetID )
      {
        ciMap.Spawn( ciPawn, true /* force spawn */ );

        if( ciTransport.Portal( ciMap.GetAssetID(), ciPawn.GetPosition() ) )
        {
          local ciEnemyArray = ciMap.GetPawns( ciTransport.GetPosition(), 5, false /* no LOS check */ );
          foreach( ciEnemy in ciEnemyArray )
          {
            eAssetID = ciEnemy.GetAssetID();
            if( U1_Space_Enemy_CreatureID == eAssetID )
            {
              ciEnemy.Damage( ciEnemy.GetHitpoints() + 1, m_ciPlayer );
            }
          }
        }
      }
      else if( U1_Sosaria_PortalID == eAssetID )
      {
        ciSosaria = ciPawn;
      }
    }

    // Move back to Sosaria
    if( ciSosaria != null )
    {
      ciSosaria.Use( m_ciPlayer, PortalUsageType.Enter );
    }

    local iNumKilled = m_ciPlayer.GetProperty( U1_Space_Enemies_Killed_PropertyID, 0 );
    local bSpaceAce = ( iNumKilled >= 20 );
    m_strOutput += "Achieved Space Ace: " + ( bSpaceAce ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, PrincessTest, 0.5, 0 );
  }


  function StatTest()
  {
    // Verify that all stats have been raised
    foreach( ePropertyID in g_eU1StatPropertyArray )
    {
      local iStatValue = m_ciPlayer.GetProperty( ePropertyID, 0 );
      local bStatRaised = ( iStatValue > 20 )

      local ciAsset = ::rumGetPropertyAsset( ePropertyID );
      if( ciAsset != null )
      {
        local strStat = ::rumGetStringByName( ciAsset.GetName() + "_Property_client_StringID" );
        m_strOutput += strStat + " raised: " + ( bStatRaised ? "PASS" : "FAIL" ) + "\n";
      }
    }

    ::rumSchedule( this, LaunchTest, 0.5 );
  }


  function TestDone()
  {
    m_strOutput += "Completed Ultima 1\n";

    // Write contents to file
    ::rumWriteStringToFile( "U1_Test.txt", m_strOutput );
    g_ciGameTest = null;
  }


  function TimeMachineTest( i_iPhase )
  {
    local ciMap = m_ciPlayer.GetMap();

    if( 0 == i_iPhase )
    {
      // Move to the time machine and enter it
      local ciPawnArray = ciMap.GetAllPawns();
      foreach( ciPawn in ciPawnArray )
      {
        local eAssetID = ciPawn.GetAssetID();
        if( U1_Time_Machine_WidgetID == eAssetID )
        {
          ciPawn.Board( m_ciPlayer );
          break;
        }
      }

      ::rumSchedule( this, TimeMachineTest, 0.5, i_iPhase + 1 );
      return;
    }
    else
    {
      if( ciMap.GetAssetID() != U1_Lair_Mondain_MapID )
      {
        ::rumSchedule( this, TimeMachineTest, 1.0, i_iPhase + 1 );
        return;
      }
    }

    ::rumSchedule( this, MondainTest, 0.5, /* phase */ 0 );
  }


  function WhiteDragonTest()
  {
    // Grave of Lost Souls
    KingSignQuestTest( U1_Castle_White_Dragon_MapID, rumPos( 99, 88 ) );

    // Pillars of the Argonauts
    KingSignQuestTest( U1_Castle_White_Dragon_MapID, rumPos( 97, 33 ) );

    ::rumSchedule( this, StatTest, 0.5 );
  }
}
