class U1_Spawner_Widget extends U1_Widget
{
  static s_iScatterDistance = 1;

  m_uiSpawnTable = null;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );

    m_uiSpawnTable = {};

    ::rumSchedule( this, Spawn, Map.s_fSpawnPlayerCheck );
  }


  function CleanupCheck()
  {
    local ciMap = GetMap();
    ciMap.SpawnCleanup( this );
  }


  function Spawn()
  {
    local ciMap = GetMap();
    ciMap.Spawn( this );
  }
}


class U1_Spawner_Dungeon_12_Widget extends U1_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU1SpawnerDungeon12PawnArray;
  }
}


class U1_Spawner_Dungeon_34_Widget extends U1_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU1SpawnerDungeon34PawnArray;
  }
}


class U1_Spawner_Dungeon_56_Widget extends U1_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU1SpawnerDungeon56PawnArray;
  }
}


class U1_Spawner_Dungeon_78_Widget extends U1_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU1SpawnerDungeon78PawnArray;
  }
}


class U1_Spawner_Dungeon_910_Widget extends U1_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU1SpawnerDungeon910PawnArray;
  }
}


class U1_Spawner_Space_Station_Widget extends U1_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU1SpawnerSpaceStationPawnArray;
  }
}


class U1_Spawner_Wilderness_Common_Widget extends U1_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_aU1SpawnerWildernessCommonPawnArray;
  }
}


class U1_Spawner_Wilderness_Sea_Widget extends U1_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU1SpawnerWildernessSeaPawnArray;
  }
}


class U1_Spawner_Wilderness_Space_Widget extends U1_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU1SpawnerWildernessSpacePawnArray;
  }
}


class U1_Spawner_Wilderness_Uncommon_Widget extends U1_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU1SpawnerWildernessUncommonPawnArray;
  }
}


class U2_Spawner_Widget extends U2_Widget
{
  static s_iScatterDistance = 1;

  m_uiSpawnTable = null;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );

    m_uiSpawnTable = {};

    ::rumSchedule( this, Spawn, Map.s_fSpawnPlayerCheck );
  }


  function CleanupCheck()
  {
    local ciMap = GetMap();
    ciMap.SpawnCleanup( this );
  }


  function Spawn()
  {
    local ciMap = GetMap();
    ciMap.Spawn( this );
  }
}


class U2_Spawner_Dungeon_01_Widget extends U2_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU2SpawnerDungeon01PawnArray;
  }
}


class U2_Spawner_Dungeon_23_Widget extends U2_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU2SpawnerDungeon23PawnArray;
  }
}


class U2_Spawner_Dungeon_45_Widget extends U2_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU2SpawnerDungeon45PawnArray;
  }
}


class U2_Spawner_Dungeon_67_Widget extends U2_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU2SpawnerDungeon67PawnArray;
  }
}


class U2_Spawner_Dungeon_89_Widget extends U2_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU2SpawnerDungeon89PawnArray;
  }
}


class U2_Spawner_Dungeon_1011_Widget extends U2_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU2SpawnerDungeon1011PawnArray;
  }
}


class U2_Spawner_Dungeon_1215_Widget extends U2_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU2SpawnerDungeon1215PawnArray;
  }
}


class U2_Spawner_Plane_Widget extends U2_Spawner_Widget
{
  function GetSpawnArray()
  {
    return [U2_Plane_WidgetID];
  }
}


class U2_Spawner_Rocket_Widget extends U2_Spawner_Widget
{
  function GetSpawnArray()
  {
    return [U2_Rocket_WidgetID];
  }
}


class U2_Spawner_Ship_Widget extends U2_Spawner_Widget
{
  function GetSpawnArray()
  {
    return [U2_Ship_WidgetID];
  }
}


class U2_Spawner_Wilderness_Common_Non_Earth_Widget extends U2_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU2SpawnerWildernessCommonETPawnArray;
  }
}


class U2_Spawner_Wilderness_Common_Widget extends U2_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU2SpawnerWildernessCommonPawnArray;
  }
}


class U2_Spawner_Wilderness_Sea_Non_Earth_Widget extends U2_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU2SpawnerWildernessSeaETPawnArray;
  }
}

class U2_Spawner_Wilderness_Sea_Widget extends U2_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU2SpawnerWildernessSeaPawnArray;
  }
}


class U2_Spawner_Wilderness_Uncommon_Widget extends U2_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU2SpawnerWildernessUncommonPawnArray;
  }
}


class U3_Spawner_Widget extends U3_Widget
{
  static s_iScatterDistance = 1;

  m_uiSpawnTable = null;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );

    m_uiSpawnTable = {};

    ::rumSchedule( this, Spawn, Map.s_fSpawnPlayerCheck );
  }


  function CleanupCheck()
  {
    local ciMap = GetMap();
    ciMap.SpawnCleanup( this );
  }


  function Spawn()
  {
    local ciMap = GetMap();
    ciMap.Spawn( this );
  }
}


class U3_Spawner_Dungeon_1_Widget extends U3_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU3SpawnerDungeon1PawnArray;
  }
}


class U3_Spawner_Dungeon_2_Widget extends U3_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU3SpawnerDungeon2PawnArray;
  }
}


class U3_Spawner_Dungeon_3_Widget extends U3_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU3SpawnerDungeon3PawnArray;
  }
}


class U3_Spawner_Dungeon_4_Widget extends U3_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU3SpawnerDungeon4PawnArray;
  }
}


class U3_Spawner_Dungeon_5_Widget extends U3_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU3SpawnerDungeon5PawnArray;
  }
}


class U3_Spawner_Dungeon_6_Widget extends U3_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU3SpawnerDungeon6PawnArray;
  }
}


class U3_Spawner_Dungeon_7_Widget extends U3_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU3SpawnerDungeon7PawnArray;
  }
}


class U3_Spawner_Dungeon_8_Widget extends U3_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU3SpawnerDungeon8PawnArray;
  }
}


class U3_Spawner_Wilderness_Easy_Widget extends U3_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU3SpawnerWildernessEasyPawnArray;
  }
}


class U3_Spawner_Wilderness_Hard_Widget extends U3_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU3SpawnerWildernessHardPawnArray;
  }
}


class U3_Spawner_Wilderness_Medium_Widget extends U3_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU3SpawnerWildernessMediumPawnArray;
  }
}


class U3_Spawner_Wilderness_Sea_Widget extends U3_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU3SpawnerWildernessSeaPawnArray;
  }
}


class U3_Spawner_Wilderness_Very_Hard_Widget extends U3_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU3SpawnerWildernessDifficultPawnArray;
  }
}


class U4_Spawner_Widget extends U4_Widget
{
  static s_iScatterDistance = 4;

  m_uiSpawnTable = null;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );

    m_uiSpawnTable = {};

    ::rumSchedule( this, Spawn, Map.s_fSpawnPlayerCheck );
  }


  function CleanupCheck()
  {
    local ciMap = GetMap();
    ciMap.SpawnCleanup( this );
  }


  function Spawn()
  {
    local ciMap = GetMap();
    ciMap.Spawn( this );
  }
}


class U4_Bridge_Troll_Spawner_Widget extends U4_Widget
{
  function AffectCreature( i_ciCreature )
  {
    if( i_ciCreature.GetMoveType() != MoveType.Terrestrial )
    {
      return;
    }

    if( i_ciCreature instanceof Player )
    {
      // There is a 25% chance of spawning a troll
      if( rand() % 4 == 0 )
      {
        local ciPos = GetPosition();
        local ciTroll = ::rumCreate( U4_Troll_CreatureID );
        local ciMap = i_ciCreature.GetMap();
        if( ciMap.AddPawn( ciTroll, ciPos ) )
        {
          i_ciCreature.ActionWarning( msg_bridge_troll_client_StringID );

          ciTroll.m_bRespawns = false;
          ciTroll.m_eDefaultPosture = PostureType.Attack;
          ciTroll.m_eDefaultNpcType = NPCType.Standard;

          ::rumSchedule( ciTroll, ciTroll.AIDetermine, frandVariance( ciTroll.m_fTimeDecideVariance ) );

          // Store the NPCs original position for leashing, etc.
          ciTroll.m_ciOriginPos = ciPos;
        }
      }

      // Schedule a respawn of the spawner
      SetVisibility( false );
      ScheduleRespawn();
    }
  }
}


class U4_Spawner_Dungeon_1_Widget extends U4_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU4SpawnerDungeon1PawnArray;
  }
}


class U4_Spawner_Dungeon_2_Widget extends U4_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU4SpawnerDungeon2PawnArray;
  }
}


class U4_Spawner_Dungeon_3_Widget extends U4_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU4SpawnerDungeon3PawnArray;
  }
}


class U4_Spawner_Dungeon_4_Widget extends U4_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU4SpawnerDungeon4PawnArray;
  }
}


class U4_Spawner_Dungeon_5_Widget extends U4_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU4SpawnerDungeon5PawnArray;
  }
}


class U4_Spawner_Dungeon_6_Widget extends U4_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU4SpawnerDungeon6PawnArray;
  }
}


class U4_Spawner_Dungeon_7_Widget extends U4_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU4SpawnerDungeon7PawnArray;
  }
}


class U4_Spawner_Dungeon_8_Widget extends U4_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU4SpawnerDungeon8PawnArray;
  }
}


class U4_Spawner_Wilderness_Easy_Widget extends U4_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU4SpawnerWildernessEasyPawnArray;
  }
}


class U4_Spawner_Wilderness_Hard_Widget extends U4_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU4SpawnerWildernessHardPawnArray;
  }
}


class U4_Spawner_Wilderness_Medium_Widget extends U4_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU4SpawnerWildernessMediumPawnArray;
  }
}


class U4_Spawner_Wilderness_Sea_Widget extends U4_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU4SpawnerWildernessSeaPawnArray;
  }
}


class U4_Spawner_Wilderness_Very_Easy_Widget extends U4_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU4SpawnerWildernessTrivialPawnArray;
  }
}


class U4_Spawner_Wilderness_Very_Hard_Widget extends U4_Spawner_Widget
{
  function GetSpawnArray()
  {
    return g_eU4SpawnerWildernessDifficultPawnArray;
  }
}
