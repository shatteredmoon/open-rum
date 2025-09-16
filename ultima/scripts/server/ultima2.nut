/*

----------------------------------------------------------------------------

ooooo  oooo o888    o8    o88                                ooooo ooooo
 888    88   888  o888oo  oooo  oo ooo oooo     ooooooo       888   888
 888    88   888   888     888   888 888 888    ooooo888      888   888
 888    88   888   888     888   888 888 888  888    888      888   888
  888oo88   o888o   888o  o888o o888o888o888o  88ooo88 8o    ooooo ooooo

----------------------------------------------------------------------------

Classic Ultima Online (CUO)

MIT License

Copyright 2015 Jonathon Blake Wood-Brooks

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

Ultima is a registered trademark of Electronic Arts (EA)

Ultima II: The Revenge of the Enchantress
    Developer:  Richard Garriott
    Publishers: Sierra On-Line, Origin Systems (re-release)
    Designer:   Richard Garriott

*/

g_eU2RingMapArray <-
[
  U2_Jupiter_Dungeon_15_MapID,
  U2_Jupiter_Tower_15_MapID,
  U2_Mercury_Dungeon_15_MapID,
  U2_Pluto_Dungeon_15_MapID
]

g_eU2SpaceMapArray <-
[
  U2_Space_Sun_MapID,
  U2_Space_Mercury_MapID,
  U2_Space_Venus_MapID,
  U2_Space_Earth_MapID,
  U2_Space_Mars_MapID,
  U2_Space_Jupiter_MapID,
  U2_Space_Saturn_MapID,
  U2_Space_Uranus_MapID,
  U2_Space_Neptune_MapID,
  U2_Space_Pluto_MapID,
  U2_Space_Planet_X_MapID
]

g_eU2SpawnerDungeon01PawnArray <-
[
  U2_Orc_CreatureID
]

g_eU2SpawnerDungeon23PawnArray <-
[
  U2_Orc_CreatureID,
  U2_Ghost_CreatureID
]

g_eU2SpawnerDungeon45PawnArray <-
[
  U2_Orc_CreatureID,
  U2_Ghost_CreatureID,
  U2_Carrion_Creeper_CreatureID
]

g_eU2SpawnerDungeon67PawnArray <-
[
  U2_Orc_CreatureID,
  U2_Ghost_CreatureID,
  U2_Carrion_Creeper_CreatureID,
  U2_Viper_CreatureID
]

g_eU2SpawnerDungeon89PawnArray <-
[
  U2_Orc_CreatureID,
  U2_Ghost_CreatureID,
  U2_Carrion_Creeper_CreatureID,
  U2_Viper_CreatureID,
  U2_Gremlin_CreatureID
]

g_eU2SpawnerDungeon1011PawnArray <-
[
  U2_Orc_CreatureID,
  U2_Ghost_CreatureID,
  U2_Carrion_Creeper_CreatureID,
  U2_Viper_CreatureID,
  U2_Gremlin_CreatureID,
  U2_Daemon_CreatureID
]

g_eU2SpawnerDungeon1215PawnArray <-
[
  U2_Orc_CreatureID,
  U2_Ghost_CreatureID,
  U2_Carrion_Creeper_CreatureID,
  U2_Viper_CreatureID,
  U2_Gremlin_CreatureID,
  U2_Daemon_CreatureID,
  U2_Balron_CreatureID
]

g_eU2SpawnerWildernessCommonETPawnArray <-
[
  U2_Daemon_CreatureID,
  U2_Orc_CreatureID
]

g_eU2SpawnerWildernessCommonPawnArray <-
[
  U2_Cleric_CreatureID,
  U2_Daemon_CreatureID,
  U2_Fighter_CreatureID,
  U2_Orc_CreatureID,
  U2_Thief_CreatureID,
  U2_Wizard_CreatureID
]

g_eU2SpawnerWildernessUncommonPawnArray <-
[
  U2_Balron_CreatureID,
  U2_Devil_CreatureID
]

g_eU2SpawnerWildernessSeaETPawnArray <-
[
  U2_Sea_Monster_CreatureID
]

g_eU2SpawnerWildernessSeaPawnArray <-
[
  U2_Pirate_Ship_CreatureID,
  U2_Sea_Monster_CreatureID
]

g_iU2ArmourInventoryPercentageArray <-
[
  50, // Cloth
  75, // Leather
  90, // Chain
  95, // Plate
  98, // Reflect
  100 // Power
]

g_iU2WeaponInventoryPercentageArray <-
[
  20,  // Dagger
  35,  // Mace
  50,  // Axe
  65,  // Bow
  80,  // Sword
  95,  // Great Sword
  98,  // Light Sword
  100, // Phaser
  999  // Quick Sword
]


class Ultima2World
{
  m_uiTimeGateTable = null;
  m_eTimeGatePeriod = 0;

  static s_fTimeGatePeriodInterval = 30.0;


  constructor()
  {
    m_uiTimeGateTable = {};

    // Schedule TimeGate period updates
    ::rumSchedule( this, UpdateTimeGatePeriod, s_fTimeGatePeriodInterval );
  }


  function UpdateTimeGatePeriod()
  {
    ++m_eTimeGatePeriod;

    // See if the period should start over
    if( m_eTimeGatePeriod >= U2_TimegatePeriods.NumPeriods )
    {
      m_eTimeGatePeriod = U2_TimegatePeriods.Pangea;
    }

    // Update clients
    local ciBroadcast = ::rumCreate( TimeGate_Period_BroadcastID, m_eTimeGatePeriod );
    rumSendGlobal( ciBroadcast );

    foreach( uiWidgetID in m_uiTimeGateTable )
    {
      local ciTimeGate = ::rumFetchPawn( uiWidgetID );
      if( ciTimeGate != null )
      {
        ciTimeGate.OnPeriodChange();
      }
    }

    // Reschedule
    ::rumSchedule( this, UpdateTimeGatePeriod, s_fTimeGatePeriodInterval );
  }


  function Update()
  {
    // Do nothing
  }
}


function U2_Init()
{
  // These maps should always be active
  GetOrCreateMap( null, U2_Earth_AD_1990_World_MapID );
  GetOrCreateMap( null, U2_Earth_Aftermath_World_MapID );
  GetOrCreateMap( null, U2_Earth_BC_1423_World_MapID );
  GetOrCreateMap( null, U2_Earth_Legends_World_MapID );
  GetOrCreateMap( null, U2_Earth_Pangea_World_MapID );
}
