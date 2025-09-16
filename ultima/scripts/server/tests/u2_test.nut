class U2_GameTest
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

      m_ciPlayer.ChangeWorld( GameType.Ultima2 );

      if( m_ciPlayer.IsDead() )
      {
        m_ciPlayer.SetVersionedProperty( g_eHitpointsPropertyVersionArray, 100 );
      }

      if( m_ciPlayer.IsPoisoned() )
      {
        m_ciPlayer.RemoveVersionedProperty( g_ePoisonedPropertyVersionArray );
      }

      local ciMap = GetOrCreateMap( null, U2_Earth_Legends_Castle_Shadow_Guard_MapID );
      local ciPawnArray = ciMap.GetAllPawns();
      foreach( ciPawn in ciPawnArray )
      {
        if( ciPawn instanceof U2_Minax_Creature )
        {
          ciPawn.Respawn();
          break;
        }
      }

      // Clear everything
      m_ciPlayer.SetProperty( U2_Experience_PropertyID, 0 );
      m_ciPlayer.SetProperty( U2_Gold_PropertyID, 0 );
      m_ciPlayer.RemoveProperty( U2_Oracle_Flags_PropertyID );
      m_ciPlayer.RemoveProperty( U2_Ring_Quest_State_PropertyID );
      m_ciPlayer.RemoveProperty( U2_Magic_Ring_PropertyID );
      m_ciPlayer.RemoveProperty( U2_Quicksword_Quest_State_PropertyID );
      m_ciPlayer.RemoveProperty( U2_Item_Quicksword_Materials_PropertyID );
      m_ciPlayer.RemoveProperty( Invincible_PropertyID );

      local iFlags = m_ciPlayer.GetProperty( Ultima_Completions_PropertyID, 0 );
      iFlags = ::rumBitClear( iFlags, UltimaCompletions.KilledMinax );
      m_ciPlayer.SetProperty( Ultima_Completions_PropertyID, iFlags );

      // Stats
      foreach( eStatID in g_eU2StatPropertyArray )
      {
        m_ciPlayer.SetProperty( eStatID, 20 );
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


  function CompletionTest()
  {
    local iFlags = m_ciPlayer.GetProperty( Ultima_Completions_PropertyID, 0 );
    local bComplete = ::rumBitOn( iFlags, UltimaCompletions.KilledMinax );
    m_strOutput += "Killed Minax: " + ( bComplete ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, TestDone, 0.5 );
  }


  function DeathTest()
  {
    local ciMap = m_ciPlayer.GetMap();

    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U2_Earth_BC_Dungeon_15_MapID ), rumPos( 8, 8 ) );
    ciMap = m_ciPlayer.GetMap();

    m_ciPlayer.SetVersionedProperty( g_eHitpointsPropertyVersionArray, 1 );

    ::rumSchedule( this, LordBritishTest, 10.0 );
  }


  function FindTalkTarget( i_iSocket, i_ciMap )
  {
    local bFound = false;
    local ciPos = m_ciPlayer.GetPosition();

    for( local eDir = Direction.Start; !bFound && eDir < Direction.End; ++eDir )
    {
      local ciBroadcast = Player_Talk_Broadcast( eDir );
      ciBroadcast.OnRecv( i_iSocket, m_ciPlayer );

      bFound = m_ciPlayer.m_uiInteractID != rumInvalidGameID;
    }

    return bFound;
  }


  function HotelTest()
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U2_Earth_AD_Towne_New_San_Antonio_MapID ), rumPos( 13, 57 ) );
    ciMap = m_ciPlayer.GetMap();

    local ciPos = m_ciPlayer.GetPosition();

    m_ciPlayer.SetVersionedProperty( g_eGoldPropertyVersionArray, 150 );
    m_ciPlayer.SetVersionedProperty( g_eHitpointsPropertyVersionArray, 1 );

    // Test hitpoint tribute
    local iHitpoints = m_ciPlayer.GetVersionedProperty( g_eHitpointsPropertyVersionArray, 999 );

    local ciBroadcast = Player_Talk_Broadcast( Direction.South );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciBroadcast = Player_Talk_U2_HotelClerk_Broadcast( U2_ClerkTalkType.RequestRoom );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    local iUpdatedHitpoints = m_ciPlayer.GetVersionedProperty( g_eHitpointsPropertyVersionArray, 0 );
    local bHitpointsIncreased = iUpdatedHitpoints > iHitpoints;

    m_strOutput += "Hitpoints increased from hotel stay: " + ( bHitpointsIncreased ? "PASS" : "FAIL" ) + "\n";

    local ciNewPos = m_ciPlayer.GetPosition();
    local bMovedToRoom = ciNewPos.y < ciPos.y;
    m_strOutput += "Player moved to hotel room: " + ( bMovedToRoom ? "PASS" : "FAIL" ) + "\n";

    ciMap.TransferPawn( m_ciPlayer, ciMap, rumPos( 13, 57 ) );

    // Test stat tribute
    local iStatTotal = 0;
    foreach( ePropertyID in g_eU2StatPropertyArray )
    {
      iStatTotal += m_ciPlayer.GetProperty( ePropertyID, 0 );
    }

    ciBroadcast = Player_Talk_U2_HotelClerk_Broadcast( U2_ClerkTalkType.RequestRoomTip );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    ciBroadcast = ::rumCreate( Player_Talk_Bye_BroadcastID, DialogueTerminationType.Standard );
    ::rumSendPrivate( iSocket, ciBroadcast );

    local iNewStatTotal = 0;
    foreach( ePropertyID in g_eU2StatPropertyArray )
    {
      iNewStatTotal += m_ciPlayer.GetProperty( ePropertyID, 0 );
    }

    local bStatIncreased = iNewStatTotal > iStatTotal;
    m_strOutput += "Stat increased from hotel stay: " + ( bStatIncreased ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, OracleTest, 0.5 );
  }


  function LordBritishTest()
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    m_strOutput += "Player is dead: " + ( m_ciPlayer.IsDead() ? "PASS" : "FAIL" ) + "\n";

    // Resurrect the player to Lord British
    local ciBroadcast = Player_Resurrect_Broadcast( ResurrectionType.Void );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciMap = m_ciPlayer.GetMap();

    m_strOutput += "Player is alive: " + ( !m_ciPlayer.IsDead() ? "PASS" : "FAIL" ) + "\n";

    m_ciPlayer.SetVersionedProperty( g_eGoldPropertyVersionArray, 150 );

    // Test hitpoint tribute
    local iHitpoints = m_ciPlayer.GetVersionedProperty( g_eHitpointsPropertyVersionArray, 999 );

    ciBroadcast = Player_Talk_Broadcast( Direction.North );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciBroadcast = Player_Talk_U2_LordBritish_Broadcast( U2_LBTalkType.HitpointTribute );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    local iUpdatedHitpoints = m_ciPlayer.GetVersionedProperty( g_eHitpointsPropertyVersionArray, 0 );
    local bHitpointsIncreased = iUpdatedHitpoints > iHitpoints;

    m_strOutput += "Hitpoints increased from tribute: " + ( bHitpointsIncreased ? "PASS" : "FAIL" ) + "\n";

    // Test stat tribute
    local iStatTotal = 0;
    foreach( ePropertyID in g_eU2StatPropertyArray )
    {
      iStatTotal += m_ciPlayer.GetProperty( ePropertyID, 0 );
    }

    ciBroadcast = Player_Talk_U2_LordBritish_Broadcast( U2_LBTalkType.StatTribute );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    local iNewStatTotal = 0;
    foreach( ePropertyID in g_eU2StatPropertyArray )
    {
      iNewStatTotal += m_ciPlayer.GetProperty( ePropertyID, 0 );
    }

    ciBroadcast = ::rumCreate( Player_Talk_Bye_BroadcastID, DialogueTerminationType.Standard );
    ::rumSendPrivate( iSocket, ciBroadcast );

    local bStatIncreased = iNewStatTotal > iStatTotal;
    m_strOutput += "Stat increased from tribute: " + ( bStatIncreased ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, HotelTest, 0.5 );
  }


  function MinaxTest()
  {
    local ciMap = m_ciPlayer.GetMap();

    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U2_Earth_Legends_Castle_Shadow_Guard_MapID ),
                        ::rumPos( 60, 4) );
    ciMap = m_ciPlayer.GetMap();

    m_ciPlayer.SetProperty( U2_Experience_PropertyID, 99999 );
    m_ciPlayer.SetVersionedProperty( g_eHitpointsPropertyVersionArray, 9999 );

    // Determine Minax's position
    local ciPos = m_ciPlayer.GetPosition() + GetDirectionVector( Direction.North );
    local ciPosData = ciMap.GetPositionData( ciPos );
    local ciMinax = ciPosData.GetNext( rumCreaturePawnType, U2_Minax_CreatureID );
    if( ciMinax != null )
    {
      ::rumSchedule( this, MinaxTestAttack, 0.5, ciMinax );
    }
    else
    {
      ::rumSchedule( this, TestDone, 0.5 );
    }
  }


  function MinaxTestAttack( i_ciMinax )
  {
    if( !i_ciMinax.IsDead() )
    {
      local ciPos = m_ciPlayer.GetPosition();
      local eResult = m_ciPlayer.Attack( i_ciMinax, m_ciPlayer.GetWeaponType() );
      if( ( AttackReturnType.Success == eResult ) && !i_ciMinax.IsDead() )
      {
        local ciMap = m_ciPlayer.GetMap();
        ciPos = rumPos( i_ciMinax.s_iPosXArray[i_ciMinax.m_iPosIndex], i_ciMinax.s_iPosYArray[i_ciMinax.m_iPosIndex] );
        ciMap.TransferPawn( m_ciPlayer, ciMap, ciPos );
      }

      ::rumSchedule( this, MinaxTestAttack, 0.5, i_ciMinax );
    }
    else
    {
      ::rumSchedule( this, CompletionTest, 6.0 );
    }
  }


  function OracleInteractionTest( i_iSocket, io_ciMap, i_eDestMapID, i_ciDestPos )
  {
    io_ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, i_eDestMapID ), i_ciDestPos );
    io_ciMap = m_ciPlayer.GetMap();

    if( FindTalkTarget( i_iSocket, io_ciMap ) )
    {
      local ciTarget = ::rumFetchPawn( m_ciPlayer.m_uiInteractID );
      if( ciTarget != null )
      {
        local ciBroadcast = Player_Talk_U2_Oracle_Broadcast( U2_OracleTalkType.Pay );
        ciBroadcast.OnRecv( i_iSocket, m_ciPlayer );

        ciTarget.InterruptInteractions();
      }
    }

    return io_ciMap;
  }


  function OracleTest()
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    m_ciPlayer.SetVersionedProperty( g_eGoldPropertyVersionArray, 4500 );

    ciMap = OracleInteractionTest( iSocket, ciMap, U2_Earth_AD_Village_Port_Bonifice_MapID, rumPos( 14, 15 ) );
    ciMap = OracleInteractionTest( iSocket, ciMap, U2_Earth_Aftermath_Village_Pirates_Harbour_MapID, rumPos( 7, 23 ) );
    ciMap = OracleInteractionTest( iSocket, ciMap, U2_Earth_BC_Village_Le_Jester_MapID, rumPos( 16, 17 ) );
    ciMap = OracleInteractionTest( iSocket, ciMap, U2_Jupiter_Village_Preppies_MapID, rumPos( 7, 9 ) );
    ciMap = OracleInteractionTest( iSocket, ciMap, U2_Neptune_Village_Computer_Camp_MapID, rumPos( 12, 8 ) );
    ciMap = OracleInteractionTest( iSocket, ciMap, U2_Pluto_Village_Tommersville_MapID, rumPos( 12, 10 ) );
    ciMap = OracleInteractionTest( iSocket, ciMap, U2_Uranus_Village_New_Jester_MapID, rumPos( 6, 13 ) );
    ciMap = OracleInteractionTest( iSocket, ciMap, U2_Uranus_Village_New_Jester_MapID, rumPos( 12, 13 ) );
    ciMap = OracleInteractionTest( iSocket, ciMap, U2_Uranus_Village_New_Jester_MapID, rumPos( 20, 5 ) );

    local iOracleFlags = m_ciPlayer.GetProperty( U2_Oracle_Flags_PropertyID, 0 );
    local ciProperty = ::rumGetPropertyAsset( U2_Oracle_Flags_PropertyID );
    local bPurchasedAllClues = ( iOracleFlags == ciProperty.GetMaxValue() );

    m_strOutput += "Purchased all Oracle clues: " + ( bPurchasedAllClues ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, RingTest, 0.5 );
  }


  function QuickswordTest()
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    // Loot the dungeon cache
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U2_Earth_Pangea_Dungeon_15_MapID ), rumPos( 5, 5 ) );
    ciMap = m_ciPlayer.GetMap();

    local ciBroadcast = Player_Get_Broadcast();
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    // Loot the 1st tower cache
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U2_Earth_AD_Tower_15_MapID ), rumPos( 32, 32 ) );
    ciMap = m_ciPlayer.GetMap();

    ciBroadcast = Player_Get_Broadcast();
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    // Loot the 2nd tower cache
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U2_Earth_BC_Tower_15_MapID ), rumPos( 32, 32 ) );
    ciMap = m_ciPlayer.GetMap();

    ciBroadcast = Player_Get_Broadcast();
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    local uiMaterialFlags = m_ciPlayer.GetProperty( U2_Item_Quicksword_Materials_PropertyID, 0 );
    local bHasAllMaterials = ::rumBitAllOn( uiMaterialFlags, QuickswordMaterialFlags.All );
    m_strOutput += "Gathered all Quicksword materials: " + ( bHasAllMaterials ? "PASS" : "FAIL" ) + "\n";

    // Visit Sentri
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U2_Earth_AD_Towne_New_San_Antonio_MapID ), rumPos( 54, 6 ) );
    ciMap = m_ciPlayer.GetMap();

    local ciPos = m_ciPlayer.GetPosition();
    Player_Talk_Broadcast.TryTalk( m_ciPlayer, ciMap, ciPos + GetDirectionVector( Direction.West ), 0 );
    Player_Talk_Broadcast.TryTalk( m_ciPlayer, ciMap, ciPos + GetDirectionVector( Direction.West ), 0 );

    // Equip Quicksword
    local ciInventory = m_ciPlayer.GetInventory();
    local ciItem;
    while( ciItem = ciInventory.GetNextObject() )
    {
      local eAssetID = ciItem.GetAssetID();
      local iItemID = ciItem.GetID();
      if( U2_Quick_Sword_Weapon_InventoryID == eAssetID )
      {
        local uiMinAgility = ciItem.GetProperty( Inventory_Agility_Min_PropertyID, 0 );
        local uiCurrentAgility = m_ciPlayer.GetProperty( U2_Agility_PropertyID, 0 );
        if( uiCurrentAgility < uiMinAgility )
        {
          m_ciPlayer.SetProperty( U2_Agility_PropertyID, uiMinAgility );
        }

        m_ciPlayer.EquipWeapon( iItemID );
        ciInventory.Stop();
      }
    }

    // Verify Quicksword found and equipped
    local ciWeapon = m_ciPlayer.GetEquippedWeapon();
    local eWeaponType = ciWeapon != null ? ciWeapon.GetAssetID() : rumInvalidAssetID;
    local bEquipped = ( U2_Quick_Sword_Weapon_InventoryID == eWeaponType );
    m_strOutput += "Player equipped Quicksword: " + ( bEquipped ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, MinaxTest, 0.5 );
  }


  function RingTest()
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    local iRingIndex = m_ciPlayer.GetProperty( U2_Ring_Index_PropertyID, 0 );
    local eMapID = g_eU2RingMapArray[iRingIndex];
    local ciDestMap = GetOrCreateMap( null, eMapID );

    local ciDestPos = null;

    local ciPawnArray = ciDestMap.GetAllPawns();
    foreach( ciPawn in ciPawnArray )
    {
      if( ciPawn != null && ( ciPawn instanceof U2_Shield_Widget ) )
      {
        ciDestPos = ciPawn.GetPosition();
        break;
      }
    }

    ciMap.TransferPawn( m_ciPlayer, ciDestMap, ciDestPos );
    ciMap = m_ciPlayer.GetMap();

    // Try to get the ring
    local ciBroadcast = Player_Get_Broadcast();
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    local bHasRing = m_ciPlayer.GetProperty( U2_Magic_Ring_PropertyID, false );
    m_strOutput += "Failed to take Magic Ring: " + ( !bHasRing ? "PASS" : "FAIL" ) + "\n";

    // Get blessing from Father Antos
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U2_PlanetX_Castle_Barataria_MapID ), rumPos( 2, 48 ) );
    ciMap = m_ciPlayer.GetMap();

    local ciPos = m_ciPlayer.GetPosition();
    Player_Talk_Broadcast.TryTalk( m_ciPlayer, ciMap, ciPos + GetDirectionVector( Direction.West ), 0 );

    local uiRingQuestState = m_ciPlayer.GetProperty( U2_Ring_Quest_State_PropertyID, RingQuestState.Started );
    local bHasBlessing = ( uiRingQuestState == RingQuestState.ReceivedAntosBlessing );
    m_strOutput += "Visited Father Antos: " + ( bHasBlessing ? "PASS" : "FAIL" ) + "\n";

    // Get directions from the old man
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U2_Earth_AD_Towne_New_San_Antonio_MapID ), rumPos( 26, 2 ) );
    ciMap = m_ciPlayer.GetMap();

    ciPos = m_ciPlayer.GetPosition();
    Player_Talk_Broadcast.TryTalk( m_ciPlayer, ciMap, ciPos + GetDirectionVector( Direction.West ), 0 );

    uiRingQuestState = m_ciPlayer.GetProperty( U2_Ring_Quest_State_PropertyID, RingQuestState.Started );
    local bHasDirections = ( uiRingQuestState == RingQuestState.ReceivedDirections );
    m_strOutput += "Received Magic Ring directions: " + ( bHasDirections ? "PASS" : "FAIL" ) + "\n";

    // Return to the ring
    ciMap.TransferPawn( m_ciPlayer, ciDestMap, ciDestPos );
    ciMap = m_ciPlayer.GetMap();

    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    bHasRing = m_ciPlayer.GetProperty( U2_Magic_Ring_PropertyID, false );
    m_strOutput += "Obtained Magic Ring: " + ( bHasRing ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, QuickswordTest, 0.5 );
  }


  function TestDone()
  {
    m_strOutput += "Completed Ultima 2\n";

    // Write contents to file
    ::rumWriteStringToFile( "U2_Test.txt", m_strOutput );
    g_ciGameTest = null;
  }
}
