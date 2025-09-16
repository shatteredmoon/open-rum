class U4_GameTest
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

      m_ciPlayer.ChangeWorld( GameType.Ultima4 );

      if( m_ciPlayer.IsDead() )
      {
        m_ciPlayer.SetVersionedProperty( g_eHitpointsPropertyVersionArray, 100 );
      }

      if( m_ciPlayer.IsUnconscious() )
      {
        m_ciPlayer.RemoveProperty( Unconscious_PropertyID );
      }

      if( m_ciPlayer.IsFrozen() )
      {
        m_ciPlayer.RemoveProperty( Frozen_PropertyID );
      }

      if( m_ciPlayer.IsMeditating() )
      {
        m_ciPlayer.RemoveProperty( U4_Meditating_PropertyID );
      }

      if( m_ciPlayer.IsPoisoned() )
      {
        m_ciPlayer.RemoveVersionedProperty( g_ePoisonedPropertyVersionArray );
      }

      // Clear everything
      m_ciPlayer.SetProperty( U4_Experience_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Gold_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Item_Bell_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Item_Book_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Item_Candle_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Item_Rune_Materials_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Item_Runes_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Item_Silver_Horn_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Item_Skull_Fragment_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Item_Stones_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Item_Three_Part_Key_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Level_PropertyID, 1 );
      m_ciPlayer.SetProperty( U4_Lord_British_Stage_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Seer_Bestowals_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Shrine_Cycle1_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Shrine_Cycle2_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Shrine_Cycle3_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Virtue_Elevation_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Virtue_Honesty_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Virtue_Compassion_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Virtue_Valor_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Virtue_Sacrifice_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Virtue_Honor_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Virtue_Justice_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Virtue_Spirituality_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Virtue_Humility_PropertyID, 0 );
      m_ciPlayer.SetProperty( U4_Weary_Mind_Cycle_Count_PropertyID, 0 );
      m_ciPlayer.RemoveProperty( Invincible_PropertyID );

      local iFlags = m_ciPlayer.GetProperty( Ultima_Completions_PropertyID, 0 );
      iFlags = ::rumBitClear( iFlags, UltimaCompletions.CodexTest );
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

      ::rumSchedule( this, StoneTest, 0.5 );
    }
  }


  function AbyssNegotiation()
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    // Enter the Abyss
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Britannia_MapID ), rumPos( 233, 233 ) );
    local ciBroadcast = Player_Portal_Broadcast( PortalUsageType.Enter );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciMap = m_ciPlayer.GetMap();

    // Verify entrance
    local bEnteredAbyss = ( ciMap.GetAssetID() == U4_Dungeon_Stygian1_MapID );
    m_strOutput += "Entered Stygian Abyss: " + ( bEnteredAbyss ? "PASS" : "FAIL" ) + "\n";

    // Blue Stone Altar
    ciMap.TransferPawn( m_ciPlayer, ciMap, rumPos( 48, 48 ) );
    ciBroadcast = Abyss_Altar_Test_Broadcast( U4_AbyssAltarPhaseType.Stone, StoneType.Blue );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciMap = m_ciPlayer.GetMap();

    // Verify Blue Stone used
    local bUsedBlueStone = ( ciMap.GetAssetID() == U4_Dungeon_Stygian2_MapID );
    m_strOutput += "Used Blue Stone in Abyss: " + ( bUsedBlueStone ? "PASS" : "FAIL" ) + "\n";

    // Yellow Stone Altar
    ciMap.TransferPawn( m_ciPlayer, ciMap, rumPos( 60, 71 ) );
    ciBroadcast = Abyss_Altar_Test_Broadcast( U4_AbyssAltarPhaseType.Stone, StoneType.Yellow );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciMap = m_ciPlayer.GetMap();

    // Verify Yellow Stone used
    local bUsedYellowStone = ( ciMap.GetAssetID() == U4_Dungeon_Stygian3_MapID );
    m_strOutput += "Used Yellow Stone in Abyss: " + ( bUsedYellowStone ? "PASS" : "FAIL" ) + "\n";

    // Red Stone Altar
    ciMap.TransferPawn( m_ciPlayer, ciMap, rumPos( 38, 38 ) );
    ciBroadcast = Abyss_Altar_Test_Broadcast( U4_AbyssAltarPhaseType.Stone, StoneType.Red );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciMap = m_ciPlayer.GetMap();

    // Verify Red Stone used
    local bUsedRedStone = ( ciMap.GetAssetID() == U4_Dungeon_Stygian4_MapID );
    m_strOutput += "Used Red Stone in Abyss: " + ( bUsedRedStone ? "PASS" : "FAIL" ) + "\n";

    // Green Stone Altar
    ciMap.TransferPawn( m_ciPlayer, ciMap, rumPos( 71, 27 ) );
    ciBroadcast = Abyss_Altar_Test_Broadcast( U4_AbyssAltarPhaseType.Stone, StoneType.Green );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciMap = m_ciPlayer.GetMap();

    // Verify Green Stone used
    local bUsedGreenStone = ( ciMap.GetAssetID() == U4_Dungeon_Stygian5_MapID );
    m_strOutput += "Used Green Stone in Abyss: " + ( bUsedGreenStone ? "PASS" : "FAIL" ) + "\n";

    // Orange Stone Altar
    ciMap.TransferPawn( m_ciPlayer, ciMap, rumPos( 82, 38 ) );
    ciBroadcast = Abyss_Altar_Test_Broadcast( U4_AbyssAltarPhaseType.Stone, StoneType.Orange );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciMap = m_ciPlayer.GetMap();

    // Verify Orange Stone used
    local bUsedOrangeStone = ( ciMap.GetAssetID() == U4_Dungeon_Stygian6_MapID );
    m_strOutput += "Used Orange Stone in Abyss: " + ( bUsedOrangeStone ? "PASS" : "FAIL" ) + "\n";

    // Purple Stone Altar
    ciMap.TransferPawn( m_ciPlayer, ciMap, rumPos( 82, 82 ) );
    ciBroadcast = Abyss_Altar_Test_Broadcast( U4_AbyssAltarPhaseType.Stone, StoneType.Purple );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciMap = m_ciPlayer.GetMap();

    // Verify Purple Stone used
    local bUsedPurpleStone = ( ciMap.GetAssetID() == U4_Dungeon_Stygian7_MapID );
    m_strOutput += "Used Purple Stone in Abyss: " + ( bUsedPurpleStone ? "PASS" : "FAIL" ) + "\n";

    // White Stone Altar
    ciMap.TransferPawn( m_ciPlayer, ciMap, rumPos( 5, 5 ) );
    ciBroadcast = Abyss_Altar_Test_Broadcast( U4_AbyssAltarPhaseType.Stone, StoneType.White );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciMap = m_ciPlayer.GetMap();

    // Verify White Stone used
    local bUsedWhiteStone = ( ciMap.GetAssetID() == U4_Dungeon_Stygian8_MapID );
    m_strOutput += "Used White Stone in Abyss: " + ( bUsedWhiteStone ? "PASS" : "FAIL" ) + "\n";

    // Black Stone Altar
    ciMap.TransferPawn( m_ciPlayer, ciMap, rumPos( 71, 71 ) );
    ciBroadcast = Abyss_Altar_Test_Broadcast( U4_AbyssAltarPhaseType.Stone, StoneType.Black );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciMap = m_ciPlayer.GetMap();

    // Verify Black Stone used
    local bUsedBlackStone = ( ciMap.GetAssetID() == U4_Dungeon_Codex_Chamber_MapID );
    m_strOutput += "Used Black Stone in Abyss: " + ( bUsedBlackStone ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, AbyssTest, 0.5 );
  }


  function AbyssTest()
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    // Codex door challenge
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Dungeon_Codex_Chamber_MapID ), rumPos( 5, 45 ) );
    ciMap = m_ciPlayer.GetMap();
    local ciBroadcast = Abyss_Codex_Test_Broadcast( U4_AbyssCodexPhaseType.WordOfPassage, "veramocor" );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    // Verify door challenge passed
    local ciPos = m_ciPlayer.GetPosition();
    local bPassed = ( ( ciPos.x == 5 ) && ciPos.y < 45 );
    m_strOutput += "Codex Word of Power given: " + ( bPassed ? "PASS" : "FAIL" ) + "\n";

    ciMap.TransferPawn( m_ciPlayer, ciMap, rumPos( 5, 6 ) );

    // Codex Virtue tests
    for( local eVirtue = 0; eVirtue < VirtueType.NumVirtues; ++eVirtue )
    {
      local strVirtue = ::rumGetString( u4_virtue0_shared_StringID + eVirtue, m_ciPlayer.m_iLanguageID );
      m_ciPlayer.SetProperty( U4_Codex_Question_Attempt_PropertyID, 3 );
      ciBroadcast = Abyss_Codex_Test_Broadcast( U4_AbyssCodexPhaseType.Virtue1 + eVirtue, strVirtue );
      ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    }

    // Verify Virtue challenge passed
    ciMap = m_ciPlayer.GetMap();
    bPassed = ( ciMap.GetAssetID() == U4_Dungeon_Codex_Chamber_MapID )
    m_strOutput += "Codex Virtue test passed: " + ( bPassed ? "PASS" : "FAIL" ) + "\n";

    // Codex Principle tests
    for( local ePrinciple = 0; ePrinciple < PrincipleType.NumPrinciples; ++ePrinciple )
    {
      local strPrinciple = ::rumGetString( u4_principle0_shared_StringID + ePrinciple, m_ciPlayer.m_iLanguageID );
      m_ciPlayer.SetProperty( U4_Codex_Question_Attempt_PropertyID, 3 );
      ciBroadcast = Abyss_Codex_Test_Broadcast( U4_AbyssCodexPhaseType.Principle1 + ePrinciple, strPrinciple );
      ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    }

    // Verify Principle challenge passed
    ciMap = m_ciPlayer.GetMap();
    bPassed = ( ciMap.GetAssetID() == U4_Dungeon_Codex_Chamber_MapID )
    m_strOutput += "Codex Principle test passed: " + ( bPassed ? "PASS" : "FAIL" ) + "\n";

    // Codex Axiom test
    local strAxiom = ::rumGetString( u4_codex_axiom_server_StringID, m_ciPlayer.m_iLanguageID );
    m_ciPlayer.SetProperty( U4_Codex_Question_Attempt_PropertyID, 3 );
    ciBroadcast = Abyss_Codex_Test_Broadcast( U4_AbyssCodexPhaseType.Axiom, strAxiom );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    // Avatar test
    local eClass = m_ciPlayer.GetProperty( U4_PlayerClass_PropertyID, U4_Mage_Class_CustomID );
    m_strOutput += "Player achieved Avatarhood: " + ( U4_Avatar_Class_CustomID == eClass ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, CompletionTest, 0.5 );
  }


  function BellBookCandleTest()
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    // Bell of Courage (Cape of Lost Hope)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Keep_Serpents_Hold_MapID ), rumPos( 6, 12 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // Book of Truth (Well of Knowledge)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Keep_Lycaeum_MapID ), rumPos( 6, 6 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // Candle of Love (The Oak Grove)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Village_Cove_MapID ), rumPos( 22, 1 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    local bAllFound = ( m_ciPlayer.GetProperty( U4_Item_Bell_PropertyID, 0 ) == U4_QuestItemState.Found ) &&
                      ( m_ciPlayer.GetProperty( U4_Item_Book_PropertyID, 0 ) == U4_QuestItemState.Found ) &&
                      ( m_ciPlayer.GetProperty( U4_Item_Candle_PropertyID, 0 ) == U4_QuestItemState.Found );
    m_strOutput += "Bell, Book, and Candle found: " + ( bAllFound ? "PASS" : "FAIL" ) + "\n";

    // Bell imbued
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Britannia_MapID ), rumPos( 96, 215 ) );
    ciMap = m_ciPlayer.GetMap();
    local ciBroadcast = Player_Use_Broadcast( U4_Item_Bell_PropertyID );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    // Book imbued
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Britannia_MapID ), rumPos( 176, 208 ) );
    ciMap = m_ciPlayer.GetMap();
    ciBroadcast = Player_Use_Broadcast( U4_Item_Book_PropertyID );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    // Candle imbued
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Keep_Empath_Abbey_MapID ), rumPos( 22, 4 ) );
    ciMap = m_ciPlayer.GetMap();
    ciBroadcast = Player_Use_Broadcast( U4_Item_Candle_PropertyID );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    local bAllImbued = ( m_ciPlayer.GetProperty( U4_Item_Bell_PropertyID, 0 ) == U4_QuestItemState.Imbued ) &&
                       ( m_ciPlayer.GetProperty( U4_Item_Book_PropertyID, 0 ) == U4_QuestItemState.Imbued ) &&
                       ( m_ciPlayer.GetProperty( U4_Item_Candle_PropertyID, 0 ) == U4_QuestItemState.Imbued );
    m_strOutput += "Bell, Book, and Candle imbued: " + ( bAllImbued ? "PASS" : "FAIL" ) + "\n";

    // Bell rang
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Britannia_MapID ), rumPos( 233, 233 ) );
    ciMap = m_ciPlayer.GetMap();
    local ciBroadcast = Player_Use_Broadcast( U4_Item_Bell_PropertyID );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    // Book read
    ciBroadcast = Player_Use_Broadcast( U4_Item_Book_PropertyID );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    // Candle lit
    ciBroadcast = Player_Use_Broadcast( U4_Item_Candle_PropertyID );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    local bAllUsed = ( m_ciPlayer.GetProperty( U4_Item_Bell_PropertyID, 0 ) == U4_QuestItemState.Abyss_Used ) &&
                     ( m_ciPlayer.GetProperty( U4_Item_Book_PropertyID, 0 ) == U4_QuestItemState.Abyss_Used ) &&
                     ( m_ciPlayer.GetProperty( U4_Item_Candle_PropertyID, 0 ) == U4_QuestItemState.Abyss_Used );
    m_strOutput += "Bell, Book, and Candle used: " + ( bAllUsed ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, HornTest, 0.5 );
  }


  function CompletionTest()
  {
    local iFlags = m_ciPlayer.GetProperty( Ultima_Completions_PropertyID, 0 );
    local bComplete = ::rumBitOn( iFlags, UltimaCompletions.CodexTest );
    m_strOutput += "Codex Test Passed: " + ( bComplete ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, TestDone, 0.5 );
  }


  function HornTest()
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Britannia_MapID ), rumPos( 45, 173 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    local bFound = ( m_ciPlayer.GetProperty( U4_Item_Silver_Horn_PropertyID, 0 ) == U4_QuestItemState.Found );
    m_strOutput += "Horn found: " + ( bFound ? "PASS" : "FAIL" ) + "\n";

    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Towne_Magincia_MapID ), rumPos( 17, 17 ) );
    ciMap = m_ciPlayer.GetMap();
    local ciBroadcast = Player_Use_Broadcast( U4_Item_Silver_Horn_PropertyID );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    local bImbued = ( m_ciPlayer.GetProperty( U4_Item_Silver_Horn_PropertyID, 0 ) == U4_QuestItemState.Imbued );
    m_strOutput += "Horn imbued: " + ( bFound ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, ThreePartKeyTest, 0.5 );
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

    local iStage = m_ciPlayer.GetProperty( U4_Lord_British_Stage_PropertyID, -1 );
    m_strOutput += "Lord British not met: " + ( iStage == 0 ? "PASS" : "FAIL" ) + "\n";

    // Speak to LB
    local ciPos = m_ciPlayer.GetPosition() + GetDirectionVector( Direction.North );
    Player_Talk_Broadcast.TryTalk( m_ciPlayer, ciMap, ciPos, 0 );
    Player_Talk_Bye_Broadcast.OnRecv( iSocket, m_ciPlayer );

    iStage = m_ciPlayer.GetProperty( U4_Lord_British_Stage_PropertyID, -1 );
    m_strOutput += "Lord British met: " + ( iStage > 0 ? "PASS" : "FAIL" ) + "\n";

    // Speak again after increasing experience
    m_ciPlayer.SetProperty( U4_Experience_PropertyID, 8000 );
    Player_Talk_Broadcast.TryTalk( m_ciPlayer, ciMap, ciPos, 0 );
    Player_Talk_Bye_Broadcast.OnRecv( iSocket, m_ciPlayer );

    local iLevel = m_ciPlayer.GetProperty( U4_Level_PropertyID, 0 );
    m_strOutput += "Player promoted to level 8: " + ( iLevel == 8 ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, SkullFragmentTest, 0.5 );
  }


  function MysticsTest()
  {
    local ciMap = m_ciPlayer.GetMap();

    // Mystic Robe
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Keep_Serpents_Hold_MapID ), rumPos( 26, 9 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // Mystic Sword
    ciMap.TransferPawn( m_ciPlayer, ciMap, rumPos( 4, 9 ) );
    ciMap.Search( m_ciPlayer );

    // Equip Mystics
    local ciInventory = m_ciPlayer.GetInventory();
    local ciItem;
    while( ciItem = ciInventory.GetNextObject() )
    {
      local eAssetID = ciItem.GetAssetID();
      local iItemID = ciItem.GetID();
      if( U4_Sword_Mystic_Weapon_InventoryID == eAssetID )
      {
        m_ciPlayer.EquipWeapon( iItemID );
      }
      else if( U4_Robe_Mystic_Armour_InventoryID == eAssetID )
      {
        m_ciPlayer.EquipArmour( iItemID );
      }
    }

    // Verify Mystic Sword found and equipped
    local ciWeapon = m_ciPlayer.GetEquippedWeapon();
    local eWeaponType = ciWeapon != null ? ciWeapon.GetAssetID() : rumInvalidAssetID;
    local bMysticSwordEquipped = ( U4_Sword_Mystic_Weapon_InventoryID == eWeaponType );
    m_strOutput += "Player equipped Mystic Sword: " + ( bMysticSwordEquipped ? "PASS" : "FAIL" ) + "\n";

    // Verify Mystic Robe found and equipped
    local ciArmour = m_ciPlayer.GetEquippedArmour();
    local eArmourType = ciArmour != null ? ciArmour.GetAssetID() : rumInvalidAssetID;
    local bMysticRobeEquipped = ( U4_Robe_Mystic_Armour_InventoryID == eArmourType );
    m_strOutput += "Player equipped Mystic Robe: " + ( bMysticRobeEquipped ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, AbyssNegotiation, 0.5 );
  }


  function RuneMaterialsTest()
  {
    local ciMap = m_ciPlayer.GetMap();

    // Gemstone
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Cave_Mine_MapID ), rumPos( 13, 9 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // Bamboo
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Britannia_MapID ), rumPos( 153, 95 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // Dragonscale
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Cave_Dragon_MapID ), rumPos( 6, 9 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // Yew Wood
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Britannia_MapID ), rumPos( 49, 21 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // Soapstone
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Towne_Minoc_MapID ), rumPos( 28, 30 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // Granite
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Towne_Jhelom_MapID ), rumPos( 30, 30 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // Ivory
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Cave_Ghost_MapID ), rumPos( 5, 28 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // Bone (Humility)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Cave_Labyrinth_MapID ), rumPos( 121, 66 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    local iMaterials = m_ciPlayer.GetProperty( U4_Item_Rune_Materials_PropertyID, 0 );
    m_strOutput += "Rune materials found: " + ( iMaterials == 255 ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, RuneTest, 0.5 );
  }


  function RuneTest()
  {
    local ciPos = m_ciPlayer.GetPosition();
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    // Moonglow (Honesty)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Towne_Moonglow_MapID ), rumPos( 4, 31 ) );
    ciMap = m_ciPlayer.GetMap();
    ciPos = m_ciPlayer.GetPosition();
    Player_Talk_Broadcast.TryTalk( m_ciPlayer, ciMap, ciPos + GetDirectionVector( Direction.West ), 0 );
    local ciBroadcast = Player_Talk_Carve_Broadcast( CarveState.SymbolAnswer, "palm" );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    Player_Talk_Bye_Broadcast.OnRecv( iSocket, m_ciPlayer );

    // Britain (Compassion)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Towne_Britain_MapID ), rumPos( 30, 21 ) );
    ciMap = m_ciPlayer.GetMap();
    ciPos = m_ciPlayer.GetPosition();
    Player_Talk_Broadcast.TryTalk( m_ciPlayer, ciMap, ciPos + GetDirectionVector( Direction.North ), 0 );
    ciBroadcast = Player_Talk_Carve_Broadcast( CarveState.SymbolAnswer, "rose" );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    Player_Talk_Bye_Broadcast.OnRecv( iSocket, m_ciPlayer );

    // Jhelom (Valor)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Towne_Jhelom_MapID ), rumPos( 8, 10 ) );
    ciMap = m_ciPlayer.GetMap();
    ciPos = m_ciPlayer.GetPosition();
    Player_Talk_Broadcast.TryTalk( m_ciPlayer, ciMap, ciPos + GetDirectionVector( Direction.West ), 0 );
    ciBroadcast = Player_Talk_Carve_Broadcast( CarveState.SymbolAnswer, "sword" );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    Player_Talk_Bye_Broadcast.OnRecv( iSocket, m_ciPlayer );

    // Minoc (Sacrifice)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Towne_Minoc_MapID ), rumPos( 14, 5 ) );
    ciMap = m_ciPlayer.GetMap();
    ciPos = m_ciPlayer.GetPosition();
    Player_Talk_Broadcast.TryTalk( m_ciPlayer, ciMap, ciPos + GetDirectionVector( Direction.North ), 0 );
    ciBroadcast = Player_Talk_Carve_Broadcast( CarveState.SymbolAnswer, "tear" );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    Player_Talk_Bye_Broadcast.OnRecv( iSocket, m_ciPlayer );

    // Trinsic (Honor)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Towne_Trinsic_MapID ), rumPos( 18, 15 ) );
    ciMap = m_ciPlayer.GetMap();
    ciPos = m_ciPlayer.GetPosition();
    Player_Talk_Broadcast.TryTalk( m_ciPlayer, ciMap, ciPos + GetDirectionVector( Direction.East ), 0 );
    ciBroadcast = Player_Talk_Carve_Broadcast( CarveState.SymbolAnswer, "chalice" );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    Player_Talk_Bye_Broadcast.OnRecv( iSocket, m_ciPlayer );

    // Yew (Justice)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Towne_Yew_MapID ), rumPos( 15, 12 ) );
    ciMap = m_ciPlayer.GetMap();
    ciPos = m_ciPlayer.GetPosition();
    Player_Talk_Broadcast.TryTalk( m_ciPlayer, ciMap, ciPos + GetDirectionVector( Direction.West ), 0 );
    ciBroadcast = Player_Talk_Carve_Broadcast( CarveState.SymbolAnswer, "scales" );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    Player_Talk_Bye_Broadcast.OnRecv( iSocket, m_ciPlayer );

    // Skara Brae (Spirituality)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Towne_Skara_Brae_MapID ), rumPos( 29, 22 ) );
    ciMap = m_ciPlayer.GetMap();
    ciPos = m_ciPlayer.GetPosition();
    Player_Talk_Broadcast.TryTalk( m_ciPlayer, ciMap, ciPos + GetDirectionVector( Direction.North ), 0 );
    ciBroadcast = Player_Talk_Carve_Broadcast( CarveState.SymbolAnswer, "ankh" );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    Player_Talk_Bye_Broadcast.OnRecv( iSocket, m_ciPlayer );

    // Magincia (Humility)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Towne_Magincia_MapID ), rumPos( 30, 24 ) );
    ciMap = m_ciPlayer.GetMap();
    ciPos = m_ciPlayer.GetPosition();
    Player_Talk_Broadcast.TryTalk( m_ciPlayer, ciMap, ciPos + GetDirectionVector( Direction.South ), 0 );
    ciBroadcast = Player_Talk_Carve_Broadcast( CarveState.SymbolAnswer, "crook" );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    Player_Talk_Bye_Broadcast.OnRecv( iSocket, m_ciPlayer );

    local bHasRunes = m_ciPlayer.HasAllRunes();
    m_strOutput += "Runes found: " + ( bHasRunes ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, BellBookCandleTest, 0.5 );
  }


  function SkullFragmentTest()
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    // Find skull fragment
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Britannia_MapID ), rumPos( 197, 245 ) );
    ciMap = m_ciPlayer.GetMap();

    while( g_ciServer.m_ciUltima4World.m_eMoonTrammel != MoonPhase.New || g_ciServer.m_ciUltima4World.m_eMoonFelucca != MoonPhase.New )
    {
      g_ciServer.m_ciUltima4World.UpdateMoonPhase();
    }

    ciMap.Search( m_ciPlayer );

    local bFound = m_ciPlayer.GetProperty( U4_Item_Skull_Fragment_PropertyID, 0 ) == U4_QuestItemState.Found;
    m_strOutput += "Skull fragment found: " + ( bFound ? "PASS" : "FAIL" ) + "\n";

    // Save current virtue levels
    local iVirtueLevels = [ 0, 0, 0, 0, 0, 0, 0, 0 ];
    for( local eVirtue = 0; eVirtue < VirtueType.NumVirtues; ++eVirtue )
    {
      iVirtueLevels[eVirtue] = m_ciPlayer.GetProperty( g_eU4VirtuePropertyArray[eVirtue], 0 );
    }

    // Destroy fragment
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Britannia_MapID ), rumPos( 233, 233 ) );
    ciMap = m_ciPlayer.GetMap();
    local ciBroadcast = Player_Use_Broadcast( U4_Item_Skull_Fragment_PropertyID );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    local bDestroyed = m_ciPlayer.GetProperty( U4_Item_Skull_Fragment_PropertyID, 0 ) == U4_QuestItemState.Abyss_Used;
    m_strOutput += "Skull fragment destroyed: " + ( bDestroyed ? "PASS" : "FAIL" ) + "\n";

    // Verify all virtue levels increased
    local bAllVirtuesIncreased = true;
    for( local eVirtue = 0; bAllVirtuesIncreased && eVirtue < VirtueType.NumVirtues; ++eVirtue )
    {
      local iNewVirtueLevel = m_ciPlayer.GetProperty( g_eU4VirtuePropertyArray[eVirtue], 0 );
      bAllVirtuesIncreased = ( bAllVirtuesIncreased && ( iVirtueLevels[eVirtue] < iNewVirtueLevel ) );
    }

    m_strOutput += "All virtues increased: " + ( bAllVirtuesIncreased ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, VirtueElevationBestowal, 0.5 );
  }


  function StoneTest()
  {
    local ciMap = m_ciPlayer.GetMap();

    // Blue
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Dungeon_Deceit7_MapID ), rumPos( 4, 58 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // Yellow
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Dungeon_Despise5_MapID ), rumPos( 23, 38 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // Red
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Dungeon_Destard7_MapID ), rumPos( 22, 62 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // Green
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Dungeon_Wrong8_MapID ), rumPos( 4, 5 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // Orange
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Dungeon_Covetous7_MapID ), rumPos( 58, 22 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // Purple
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Dungeon_Shame2_MapID ), rumPos( 4, 75 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // White
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Britannia_MapID ), rumPos( 64, 80 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    // Black
    while( g_ciServer.m_ciUltima4World.m_eMoonTrammel != MoonPhase.New || g_ciServer.m_ciUltima4World.m_eMoonFelucca != MoonPhase.New )
    {
      g_ciServer.m_ciUltima4World.UpdateMoonPhase();
    }
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Britannia_MapID ), rumPos( 224, 133 ) );
    ciMap = m_ciPlayer.GetMap();
    ciMap.Search( m_ciPlayer );

    local bHasStones = m_ciPlayer.HasAllStones();
    m_strOutput += "Stones found: " + ( bHasStones ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, RuneMaterialsTest, 0.5 );
  }


  function TestDone()
  {
    m_strOutput += "Completed Ultima 4\n";

    // Write contents to file
    ::rumWriteStringToFile( "U4_Test.txt", m_strOutput );
    g_ciGameTest = null;
  }


  function ThreePartKeyTest()
  {
    local ciMap = m_ciPlayer.GetMap();

    // Truth (blue, green, purple, white)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Altar_Truth_MapID ), rumPos( 16, 16 ) );
    ciMap = m_ciPlayer.GetMap();
    local ciPawnArray = ciMap.GetPawns( m_ciPlayer.GetPosition(), 1, false );
    foreach( ciPawn in ciPawnArray )
    {
      if( ciPawn.GetAssetID() == U4_Altar_WidgetID )
      {
        ciPawn.Use( m_ciPlayer,
                    [ VirtueType.Honesty, VirtueType.Justice, VirtueType.Honor, VirtueType.Spirituality ] );
        break;
      }
    }

    // Love (yellow, green, orange, white)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Altar_Love_MapID ), rumPos( 16, 16 ) );
    ciMap = m_ciPlayer.GetMap();
    local ciPawnArray = ciMap.GetPawns( m_ciPlayer.GetPosition(), 1, false );
    foreach( ciPawn in ciPawnArray )
    {
      if( ciPawn.GetAssetID() == U4_Altar_WidgetID )
      {
        ciPawn.Use( m_ciPlayer,
                    [ VirtueType.Compassion, VirtueType.Justice, VirtueType.Sacrifice, VirtueType.Spirituality ] );
        break;
      }
    }

    // Courage (red, purple, orange, white)
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Altar_Courage_MapID ), rumPos( 16, 16 ) );
    ciMap = m_ciPlayer.GetMap();
    local ciPawnArray = ciMap.GetPawns( m_ciPlayer.GetPosition(), 1, false );
    foreach( ciPawn in ciPawnArray )
    {
      if( ciPawn.GetAssetID() == U4_Altar_WidgetID )
      {
        ciPawn.Use( m_ciPlayer,
                    [ VirtueType.Valor, VirtueType.Sacrifice, VirtueType.Honor, VirtueType.Spirituality ] );
        break;
      }
    }

    local bHasKey = m_ciPlayer.HasThreePartsKey();
    m_strOutput += "Three Part Key obtained: " + ( bHasKey ? "PASS" : "FAIL" ) + "\n";

    // Wait until the player dies
    ::rumSchedule( this, LordBritishTest, 8.0 );
  }


  function VirtueElevation()
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    // Shrine of Honesty
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Britannia_MapID ), rumPos( 233, 66 ) );
    local ciBroadcast = Player_Portal_Broadcast( PortalUsageType.Enter );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    ciMap = m_ciPlayer.GetMap();

    // Verify the shrine was entered
    local bEnteredShrine = ( ciMap.GetAssetID() == U4_Shrine_Honesty_MapID );
    m_strOutput += "Entered Shrine of Honesty: " + ( bEnteredShrine ? "PASS" : "FAIL" ) + "\n";

    // Meditate for 1 cycle
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Shrine_Honesty_MapID ), rumPos( 8, 7 ) );
    ciBroadcast = Player_Meditate_Broadcast( U4_Meditation_Phase.Cycle1, "ahm", 1 );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer )

    // Verify meditation
    local iMeditationFlags = m_ciPlayer.GetProperty( U4_Shrine_Cycle1_PropertyID, 0 );
    local bMeditated = ::rumBitOn( iMeditationFlags, VirtueType.Honesty );
    m_strOutput += "Meditated for 1 cycle: " + ( bMeditated ? "PASS" : "FAIL" ) + "\n";

    // Meditate for 2 cycles
    ciBroadcast = Player_Meditate_Broadcast( U4_Meditation_Phase.Cycle2, "ahm", 2 );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer )

    // Verify meditation
    iMeditationFlags = m_ciPlayer.GetProperty( U4_Shrine_Cycle2_PropertyID, 0 );
    bMeditated = ::rumBitOn( iMeditationFlags, VirtueType.Honesty );
    m_strOutput += "Meditated for 2 cycle: " + ( bMeditated ? "PASS" : "FAIL" ) + "\n";

    // Meditate for 3 cycles
    ciBroadcast = Player_Meditate_Broadcast( U4_Meditation_Phase.Cycle3, "ahm", 3 );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer )

    // Verify meditation
    iMeditationFlags = m_ciPlayer.GetProperty( U4_Shrine_Cycle3_PropertyID, 0 );
    bMeditated = ::rumBitOn( iMeditationFlags, VirtueType.Honesty );
    m_strOutput += "Meditated for 3 cycle: " + ( bMeditated ? "PASS" : "FAIL" ) + "\n";

    // Verify elevation
    local iElevationFlags = m_ciPlayer.GetProperty( U4_Virtue_Elevation_PropertyID, 0 );
    local bElevated = ::rumBitOn( iElevationFlags, VirtueType.Honesty );
    m_strOutput += "Elevated in Honesty: " + ( bElevated ? "PASS" : "FAIL" ) + "\n";

    // Elevate all virtues
    local iMaxValue = ::rumGetMaxPropertyValue( U4_Virtue_Elevation_PropertyID );
    m_ciPlayer.SetProperty( U4_Virtue_Elevation_PropertyID, iMaxValue );
    iElevationFlags = m_ciPlayer.GetProperty( U4_Virtue_Elevation_PropertyID, 0 );
    m_strOutput += "Elevated in all virtues: " + ( iElevationFlags == iMaxValue ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, MysticsTest, 0.5 );
  }


  function VirtueElevationBestowal()
  {
    local ciMap = m_ciPlayer.GetMap();
    local iSocket = m_ciPlayer.GetSocket();

    // Travel to Hawkwind
    ciMap.TransferPawn( m_ciPlayer, GetOrCreateMap( null, U4_Castle_British_1_MapID ), rumPos( 9, 25 ) );
    ciMap = m_ciPlayer.GetMap();

    local ciBroadcast = Player_Talk_Broadcast( Direction.South );
    ciBroadcast.OnRecv( iSocket, m_ciPlayer );

    // Set all virtues to max
    local iMaxValue = ::rumGetMaxPropertyValue( g_eU4VirtuePropertyArray[0] );
    for( local eVirtue = 0; eVirtue < VirtueType.NumVirtues; ++eVirtue )
    {
      m_ciPlayer.SetProperty( g_eU4VirtuePropertyArray[eVirtue], iMaxValue );

      // Talk to the seer
      ciBroadcast = Player_Talk_Seer_Broadcast( U4_SeerTalkType.Response, eVirtue );
      ciBroadcast.OnRecv( iSocket, m_ciPlayer );
    }

    ciBroadcast = ::rumCreate( Player_Talk_Bye_BroadcastID, DialogueTerminationType.Standard );
    ::rumSendPrivate( iSocket, ciBroadcast );

    // Give elevation permission for the affected virtue
    iMaxValue = ::rumGetMaxPropertyValue( U4_Seer_Bestowals_PropertyID );
    local iElevationFlags = m_ciPlayer.GetProperty( U4_Seer_Bestowals_PropertyID, 0 );
    m_strOutput += "All virtues elevated: " + ( iElevationFlags == iMaxValue ? "PASS" : "FAIL" ) + "\n";

    ::rumSchedule( this, VirtueElevation, 0.5 );
  }
}
