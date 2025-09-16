class U3_GameTest
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

      m_ciPlayer.ChangeWorld( GameType.Ultima3 );

      if( m_ciPlayer.IsDead() )
      {
        m_ciPlayer.SetVersionedProperty( g_eHitpointsPropertyVersionArray, 100 );
      }

      if( m_ciPlayer.IsPoisoned() )
      {
        m_ciPlayer.RemoveVersionedProperty( g_ePoisonedPropertyVersionArray );
      }

      // Clear everything
      m_ciPlayer.SetProperty( U3_Cards_PropertyID, 0 );
      m_ciPlayer.SetProperty( U3_Experience_PropertyID, 0 );
      m_ciPlayer.SetProperty( U3_Gold_PropertyID, 0 );
      m_ciPlayer.SetProperty( U3_Level_PropertyID, 1 );
      m_ciPlayer.SetProperty( U3_Marks_PropertyID, 0 );
      m_ciPlayer.SetProperty( U3_Dexterity_PropertyID, 20 );
      m_ciPlayer.SetProperty( U3_Intelligence_PropertyID, 20 );
      m_ciPlayer.SetProperty( U3_Strength_PropertyID, 20 );
      m_ciPlayer.SetProperty( U3_Wisdom_PropertyID, 20 );
      m_ciPlayer.RemoveProperty( Invincible_PropertyID );

      local iFlags = m_ciPlayer.GetProperty( Ultima_Completions_PropertyID, 0 );
      iFlags = ::rumBitClear( iFlags, UltimaCompletions.DestroyedExodus );
      m_ciPlayer.SetProperty( Ultima_Completions_PropertyID, iFlags );

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

      ::rumSchedule( this, LordBritishTest, 0.5, /* phase */ 1 );
    }
  }


  function CompletionTest()
  {
    local iFlags = m_ciPlayer.GetProperty( Ultima_Completions_PropertyID, 0 );
    local bComplete = ::rumBitOn( iFlags, UltimaCompletions.DestroyedExodus );
    m_strOutput += "Destroyed Exodus: " + ( bComplete ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, TestDone, 0.5 );
  }


  function ExodusTest()
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( m_ciPlayer, U3_Castle_Fire_2_MapID ), rumPos( 6, 15 ) );
    ciMap = m_ciPlayer.GetMap();

    local iExodusIndex = m_ciPlayer.GetProperty( U3_Exodus_Card_Index_PropertyID );

    // First card
    local ciBroadcast = Player_Insert_Broadcast( g_uiU3ExodusOrderIndex0Array[iExodusIndex], Direction.North );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    ciMap.TransferPawn( m_ciPlayer, ciMap, rumPos( 7, 15 ) );

    // Second card
    local ciBroadcast = Player_Insert_Broadcast( g_uiU3ExodusOrderIndex1Array[iExodusIndex], Direction.North );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    ciMap.TransferPawn( m_ciPlayer, ciMap, rumPos( 8, 15 ) );

    // Third card
    local ciBroadcast = Player_Insert_Broadcast( g_uiU3ExodusOrderIndex2Array[iExodusIndex], Direction.North );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    ciMap.TransferPawn( m_ciPlayer, ciMap, rumPos( 9, 15 ) );

    // Fourth card
    local ciBroadcast = Player_Insert_Broadcast( g_uiU3ExodusOrderIndex3Array[iExodusIndex], Direction.North );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    ::rumSchedule( this, CompletionTest, 0.5 );
  }


  function ExoticsTest()
  {
    local ciMap = m_ciPlayer.GetMap();

    local iWeaponIndex = m_ciPlayer.GetProperty( U3_Exotic_Weapon_Pos_Index_PropertyID, -1 );
    local iArmourIndex = m_ciPlayer.GetProperty( U3_Exotic_Armour_Pos_Index_PropertyID, -1 );

    local ciWeaponPosition = rumPos( 0, 0 );
    local ciArmourPosition = rumPos( 0, 0 );

    if( iWeaponIndex != -1 )
    {
      ciWeaponPosition = rumPos( g_uiU3ExoticPosXArray[iWeaponIndex], g_uiU3ExoticPosYArray[iWeaponIndex] );
    }

    if( iArmourIndex != -1 )
    {
      ciArmourPosition = rumPos( g_uiU3ExoticPosXArray[iArmourIndex], g_uiU3ExoticPosYArray[iArmourIndex] );
    }

    // Exotic Weapon
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U3_Sosaria_MapID ), ciWeaponPosition );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // Exotic Armour
    ciMap.TransferPawn( m_ciPlayer, ciMap, ciArmourPosition );
    ciMap.Search( m_ciPlayer );

    // Equip Exotics
    local ciInventory = m_ciPlayer.GetInventory();
    local ciItem;
    while( ciItem = ciInventory.GetNextObject() )
    {
      local eAssetID = ciItem.GetAssetID();
      local iItemID = ciItem.GetID();
      if( U3_Exotic_Weapon_InventoryID == eAssetID )
      {
        m_ciPlayer.EquipWeapon( iItemID );
      }
      else if( U3_Exotic_Armour_InventoryID == eAssetID )
      {
        m_ciPlayer.EquipArmour( iItemID );
      }
    }

    // Verify Exotic Weapon found and equipped
    local ciWeapon = m_ciPlayer.GetEquippedWeapon();
    local eWeaponType = ciWeapon != null ? ciWeapon.GetAssetID() : rumInvalidAssetID;
    local bExoticWeaponEquipped = ( U3_Exotic_Weapon_InventoryID == eWeaponType );
    m_strOutput += "Player equipped Exotic Weapon: " + ( bExoticWeaponEquipped ? "PASS" : "FAIL" ) + "\n";

    // Verify Exotic Armour found and equipped
    local ciArmour = m_ciPlayer.GetEquippedArmour();
    local eArmourType = ciArmour != null ? ciArmour.GetAssetID() : rumInvalidAssetID;
    local bExoticArmourEquipped = ( U3_Exotic_Armour_InventoryID == eArmourType );
    m_strOutput += "Player equipped Exotic Armour: " + ( bExoticArmourEquipped ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, SerpentTest, 0.5, /* phase */ 2 );
  }


  function LordBritishTest( i_iPhase )
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    if( 1 == i_iPhase )
    {
      // Travel to LB
      ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U3_Castle_Lord_British_MapID ), rumPos( 32, 15 ) );
      ciMap = m_ciPlayer.GetMap();

      local iLevel = m_ciPlayer.GetProperty( U3_Level_PropertyID, 0 );
      m_strOutput += "Player is level 1: " + ( iLevel == 1 ? "PASS" : "FAIL" ) + "\n";

      // Meet LB
      local ciBroadcast = Player_Talk_Broadcast( Direction.North );
      ciBroadcast.OnRecv( iSocket, m_ciPlayer );
      m_ciPlayer.SetProperty( U3_Experience_PropertyID, 99999 );
      ciBroadcast = Player_Talk_U3_LordBritish_Broadcast( U3_LBTalkType.Report );
      ciBroadcast.OnRecv( iSocket, m_ciPlayer );
      ciBroadcast = ::rumCreate( Player_Talk_Bye_BroadcastID, DialogueTerminationType.Standard );
      ::rumSendPrivate( iSocket, ciBroadcast );

      local iLevel = m_ciPlayer.GetProperty( U3_Level_PropertyID, 0 );
      m_strOutput += "Promoted to level 5: " + ( iLevel == 5 ? "PASS" : "FAIL" ) + "\n";

      ::rumSchedule( this, MarkTest, 0.5, /* phase */ 1 );
    }
    else if( 2 == i_iPhase )
    {
      m_strOutput += "Player is dead: " + ( m_ciPlayer.IsDead() ? "PASS" : "FAIL" ) + "\n";

      // Resurrect the player to Lord British
      local ciBroadcast = Player_Resurrect_Broadcast( ResurrectionType.Void );
      ciBroadcast.OnRecv( iSocket, m_ciPlayer );
      ciMap = m_ciPlayer.GetMap();
      m_strOutput += "Player is alive: " + ( !m_ciPlayer.IsDead() ? "PASS" : "FAIL" ) + "\n";

      // Talk to LB
      local ciBroadcast = Player_Talk_Broadcast( Direction.North );
      ciBroadcast.OnRecv( iSocket, m_ciPlayer );
      ciBroadcast = Player_Talk_U3_LordBritish_Broadcast( U3_LBTalkType.Report );
      ciBroadcast.OnRecv( iSocket, m_ciPlayer );
      ciBroadcast = ::rumCreate( Player_Talk_Bye_BroadcastID, DialogueTerminationType.Standard );
      ::rumSendPrivate( iSocket, ciBroadcast );

      local iLevel = m_ciPlayer.GetProperty( U3_Level_PropertyID, 0 );
      m_strOutput += "Promoted to level 25: " + ( iLevel == 25 ? "PASS" : "FAIL" ) + "\n";

      ::rumSchedule( this, ShrineTest, 0.5 );
    }
  }


  function MarkTest( i_iPhase )
  {
    local ciMap = m_ciPlayer.GetMap();

    if( 1 == i_iPhase )
    {
      // Obtain the Mark of Kings
      ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U3_Dungeon_Perinian_Depths_8_MapID ), rumPos( 32, 33 ) );
      ciMap = m_ciPlayer.GetMap();
      local ciPawnArray = ciMap.GetPawns( m_ciPlayer.GetPosition(), 1, false );
      foreach( ciPawn in ciPawnArray )
      {
        if( ciPawn.GetAssetID() == U3_Mark_Rod_WidgetID )
        {
          // Damage the player so that touching the Mark will kill them
          m_ciPlayer.Damage( m_ciPlayer.GetHitpoints() - 1, ciPawn );

          ciPawn.Use( m_ciPlayer );
          break;
        }
      }

      // Verify mark obtained
      local iMarkFlags = m_ciPlayer.GetProperty( U3_Marks_PropertyID, 0 );
      local bHasMark = ::rumBitOn( iMarkFlags, U3_MarkType.King );
      m_strOutput += "Acquired Mark of Kings: " + ( bHasMark ? "PASS" : "FAIL" ) + "\n";

      ::rumSchedule( this, LordBritishTest, 0.5, /* phase */ 2 );
    }
    else if( 2 == i_iPhase )
    {
      // Obtain the Mark of Fire
      ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U3_Dungeon_Fire_8_MapID ), rumPos( 14, 42 ) );
      ciMap = m_ciPlayer.GetMap();
      local ciPawnArray = ciMap.GetPawns( m_ciPlayer.GetPosition(), 1, false );
      foreach( ciPawn in ciPawnArray )
      {
        if( ciPawn.GetAssetID() == U3_Mark_Rod_WidgetID )
        {
          ciPawn.Use( m_ciPlayer );
          break;
        }
      }

      // Obtain the Mark of Force
      ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U3_Dungeon_Doom_8_MapID ), rumPos( 23, 27 ) );
      ciMap = m_ciPlayer.GetMap();
      local ciPawnArray = ciMap.GetPawns( m_ciPlayer.GetPosition(), 1, false );
      foreach( ciPawn in ciPawnArray )
      {
        if( ciPawn.GetAssetID() == U3_Mark_Rod_WidgetID )
        {
          ciPawn.Use( m_ciPlayer );
          break;
        }
      }

      // Obtain the Mark of the Snake
      ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U3_Dungeon_Snake_8_MapID ), rumPos( 17, 45 ) );
      ciMap = m_ciPlayer.GetMap();
      local ciPawnArray = ciMap.GetPawns( m_ciPlayer.GetPosition(), 1, false );
      foreach( ciPawn in ciPawnArray )
      {
        if( ciPawn.GetAssetID() == U3_Mark_Rod_WidgetID )
        {
          ciPawn.Use( m_ciPlayer );
          break;
        }
      }

      // Verify all marks obtained
      local iMarkFlags = m_ciPlayer.GetProperty( U3_Marks_PropertyID, 0 );
      local bHasAllMarks = ::rumBitOn( iMarkFlags, U3_MarkType.King )  &&
                           ::rumBitOn( iMarkFlags, U3_MarkType.Force ) &&
                           ::rumBitOn( iMarkFlags, U3_MarkType.Fire )  &&
                           ::rumBitOn( iMarkFlags, U3_MarkType.Snake );
      m_strOutput += "Acquired all marks: " + ( bHasAllMarks ? "PASS" : "FAIL" ) + "\n";

      ::rumSchedule( this, ExoticsTest, 0.5 );
    }
  }


  function SerpentTest( i_iPhase )
  {
    local ciMap = m_ciPlayer.GetMap();
    local ciSerpentPos = rumPos( 10, 59 );

    if( 1 == i_iPhase )
    {
      ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U3_Sosaria_MapID ), ciSerpentPos );
      ciMap = m_ciPlayer.GetMap();

      // Create a ship
      local ciTransport = ::rumCreate( U3_Ship_WidgetID );
      if( ciTransport != null )
      {
        if( ciMap.AddPawn( ciTransport, ciSerpentPos ) )
        {
          ciTransport.Board( m_ciPlayer );
        }
      }

      Player_Chat_Broadcast.PlayerCommandSay( m_ciPlayer, "evocare" );

      // Verify that the player/ship didn't move
      local ciPlayerPos = m_ciPlayer.GetPosition();
      local bFailed = ( ciPlayerPos.x == ciSerpentPos.x ) && ( ciPlayerPos.y == ciSerpentPos.y );
      m_strOutput += "Player failed to move past serpent: " + ( bFailed ? "PASS" : "FAIL" ) + "\n";

      if( ciTransport != null )
      {
        ciTransport.Exit( m_ciPlayer, ciPlayerPos, MoveType.Incorporeal );
      }

      ::rumSchedule( this, MarkTest, 0.5, /* phase */ 2 );
    }
    else if( 2 == i_iPhase )
    {
      ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U3_Sosaria_MapID ), ciSerpentPos );
      ciMap = m_ciPlayer.GetMap();

      local ciPosData = ciMap.GetPositionData( m_ciPlayer.GetPosition() );
      local ciTransport = ciPosData.GetNext( rumWidgetPawnType, U3_Ship_WidgetID );
      if( ciTransport != null )
      {
        ciTransport.Board( m_ciPlayer );
      }

      Player_Chat_Broadcast.PlayerCommandSay( m_ciPlayer, "evocare" );

      // Verify that the moved past the serpent
      local ciPlayerPos = m_ciPlayer.GetPosition();
      local bMoved = ( ciPlayerPos.y != ciSerpentPos.y );
      m_strOutput += "Player moved past serpent: " + ( bMoved ? "PASS" : "FAIL" ) + "\n";

      if( ciTransport != null )
      {
        ciTransport.Exit( m_ciPlayer, ciPlayerPos, MoveType.Incorporeal );
      }

      ::rumSchedule( this, ExodusTest, 0.5 );
    }
  }


  function ShrineTest()
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    // Give the player enough gold for the donations
    m_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, 400 );

    // Dexterity (Sol)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U3_Shrine_Dexterity_MapID ), rumPos( 4, 5 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );
    local iDexterity = m_ciPlayer.GetProperty( U3_Dexterity_PropertyID, 0 );
    local ciBroadcast = Player_Meditate_Broadcast();
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    // Intelligence (Moons)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U3_Shrine_Intelligence_MapID ), rumPos( 4, 5 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );
    local iIntelligence = m_ciPlayer.GetProperty( U3_Intelligence_PropertyID, 0 );
    ciBroadcast = Player_Meditate_Broadcast();
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    // Strength (Love)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U3_Shrine_Strength_MapID ), rumPos( 4, 5 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );
    local iStrength = m_ciPlayer.GetProperty( U3_Strength_PropertyID, 0 );
    ciBroadcast = Player_Meditate_Broadcast();
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    // Wisdom (Death)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U3_Shrine_Wisdom_MapID ), rumPos( 4, 5 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );
    local iWisdom = m_ciPlayer.GetProperty( U3_Wisdom_PropertyID, 0 );
    ciBroadcast = Player_Meditate_Broadcast();
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    // Verify all cards discovered
    local iCardFlags = m_ciPlayer.GetProperty( U3_Cards_PropertyID, 0 );
    local bHasAllCards = ::rumBitOn( iCardFlags, U3_CardType.Death ) &&
                         ::rumBitOn( iCardFlags, U3_CardType.Love )  &&
                         ::rumBitOn( iCardFlags, U3_CardType.Sol )   &&
                         ::rumBitOn( iCardFlags, U3_CardType.Moons );
    m_strOutput += "Acquired all cards: " + ( bHasAllCards ? "PASS" : "FAIL" ) + "\n";

    // Verify all stats increased
    local bIncreased = ( m_ciPlayer.GetProperty( U3_Dexterity_PropertyID, 0 ) > iDexterity );
    m_strOutput += "Dexterity increased: " + ( bIncreased ? "PASS" : "FAIL" ) + "\n";
    bIncreased = ( m_ciPlayer.GetProperty( U3_Intelligence_PropertyID, 0 ) > iIntelligence );
    m_strOutput += "Intelligence increased: " + ( bIncreased ? "PASS" : "FAIL" ) + "\n";
    bIncreased = ( m_ciPlayer.GetProperty( U3_Strength_PropertyID, 0 ) > iStrength );
    m_strOutput += "Strength increased: " + ( bIncreased ? "PASS" : "FAIL" ) + "\n";
    bIncreased = ( m_ciPlayer.GetProperty( U3_Wisdom_PropertyID, 0 ) > iWisdom );
    m_strOutput += "Wisdom increased: " + ( bIncreased ? "PASS" : "FAIL" ) + "\n";

    // Verify shrine donation cost
    local bDonated = ( m_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray, 1 ) == 0 );
    m_strOutput += "All gold donated: " + ( bDonated ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, SerpentTest, 0.5, /* phase */ 1 );
  }


  function TestDone()
  {
    m_strOutput += "Completed Ultima 3\n";

    // Write contents to file
    ::rumWriteStringToFile( "U3_Test.txt", m_strOutput );
    g_ciGameTest = null;
  }
}
