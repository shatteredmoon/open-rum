/*

----------------------------------------------------------------------------

ooooo  oooo o888    o8    o88                                ooooo ooooo ooooo
 888    88   888  o888oo  oooo  oo ooo oooo     ooooooo       888   888   888
 888    88   888   888     888   888 888 888    ooooo888      888   888   888
 888    88   888   888     888   888 888 888  888    888      888   888   888
  888oo88   o888o   888o  o888o o888o888o888o  88ooo88 8o    ooooo ooooo ooooo

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

Ultima III: Exodus
    Developer:  Richard Garriott
    Publisher:  Origin Systems
    Designers:  Richard Garriott, Ken W. Arnold

*/

g_uiU3ExodusOrderIndex0Array <- [ 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3 ];
g_uiU3ExodusOrderIndex1Array <- [ 1, 1, 2, 2, 3, 3, 0, 0, 2, 2, 3, 3, 0, 0, 1, 1, 3, 3, 0, 0, 1, 1, 2, 2 ];
g_uiU3ExodusOrderIndex2Array <- [ 2, 3, 1, 3, 1, 2, 2, 3, 0, 3, 0, 2, 1, 3, 0, 3, 0, 1, 1, 2, 0, 2, 0, 1 ];
g_uiU3ExodusOrderIndex3Array <- [ 3, 2, 3, 1, 2, 1, 3, 2, 3, 0, 2, 0, 3, 1, 3, 0, 1, 0, 2, 1, 2, 0, 1, 0 ];

g_uiU3ExoticPosXArray <- [ 25, 26, 27, 29, 32, 33, 52, 50, 51, 19, 20,  9, 10, 11 ];
g_uiU3ExoticPosYArray <- [  4,  4,  4,  4,  3,  3, 14, 17, 17, 44, 44, 62, 62, 62 ];

g_eU3CardStringArray <-
[
  msg_card_0_client_StringID,
  msg_card_1_client_StringID,
  msg_card_2_client_StringID,
  msg_card_3_client_StringID,
]

g_eU3SpawnerDungeon1PawnArray <-
[
  U3_Orc_CreatureID,
  U3_Goblin_CreatureID,
  U3_Troll_CreatureID,
  U3_Skeleton_CreatureID,
  U3_Ghoul_CreatureID,
  U3_Zombie_CreatureID
]

g_eU3SpawnerDungeon2PawnArray <-
[
  U3_Orc_CreatureID,
  U3_Goblin_CreatureID,
  U3_Troll_CreatureID,
  U3_Skeleton_CreatureID,
  U3_Ghoul_CreatureID,
  U3_Zombie_CreatureID,
  U3_Giant_CreatureID,
  U3_Golem_CreatureID,
  U3_Titan_CreatureID
]

g_eU3SpawnerDungeon3PawnArray <-
[
  U3_Skeleton_CreatureID,
  U3_Ghoul_CreatureID,
  U3_Zombie_CreatureID,
  U3_Giant_CreatureID,
  U3_Golem_CreatureID,
  U3_Titan_CreatureID,
  U3_Daemon_CreatureID,
  U3_Gargoyle_CreatureID,
  U3_Mane_CreatureID
]

g_eU3SpawnerDungeon4PawnArray <-
[
  U3_Giant_CreatureID,
  U3_Golem_CreatureID,
  U3_Titan_CreatureID,
  U3_Daemon_CreatureID,
  U3_Gargoyle_CreatureID,
  U3_Mane_CreatureID,
  U3_Pincher_CreatureID,
  U3_Bradle_CreatureID,
  U3_Snatch_CreatureID
]

g_eU3SpawnerDungeon5PawnArray <-
[
  U3_Daemon_CreatureID,
  U3_Gargoyle_CreatureID,
  U3_Mane_CreatureID,
  U3_Pincher_CreatureID,
  U3_Bradle_CreatureID,
  U3_Snatch_CreatureID,
  U3_Dragon_CreatureID,
  U3_Griffon_CreatureID,
  U3_Wyvern_CreatureID
]

g_eU3SpawnerDungeon6PawnArray <-
[
  U3_Daemon_CreatureID,
  U3_Gargoyle_CreatureID,
  U3_Mane_CreatureID,
  U3_Pincher_CreatureID,
  U3_Bradle_CreatureID,
  U3_Snatch_CreatureID,
  U3_Dragon_CreatureID,
  U3_Griffon_CreatureID,
  U3_Wyvern_CreatureID,
  U3_Balron_CreatureID,
  U3_Devil_CreatureID,
  U3_Orcus_CreatureID
]

g_eU3SpawnerDungeon7PawnArray <-
[
  U3_Pincher_CreatureID,
  U3_Bradle_CreatureID,
  U3_Snatch_CreatureID,
  U3_Dragon_CreatureID,
  U3_Griffon_CreatureID,
  U3_Wyvern_CreatureID,
  U3_Balron_CreatureID,
  U3_Devil_CreatureID,
  U3_Orcus_CreatureID
]

g_eU3SpawnerDungeon8PawnArray <-
[
  U3_Dragon_CreatureID,
  U3_Griffon_CreatureID,
  U3_Wyvern_CreatureID,
  U3_Balron_CreatureID,
  U3_Devil_CreatureID,
  U3_Orcus_CreatureID
]

g_eU3SpawnerWildernessDifficultPawnArray <-
[
  U3_Balron_CreatureID,
  U3_Devil_CreatureID,
  U3_Orcus_CreatureID
]

g_eU3SpawnerWildernessEasyPawnArray <-
[
  U3_Orc_CreatureID,
  U3_Goblin_CreatureID,
  U3_Troll_CreatureID,
  U3_Skeleton_CreatureID,
  U3_Ghoul_CreatureID,
  U3_Zombie_CreatureID,
  U3_Fighter_CreatureID,
  U3_Thief_CreatureID
]

g_eU3SpawnerWildernessHardPawnArray <-
[
  U3_Pincher_CreatureID,
  U3_Bradle_CreatureID,
  U3_Snatch_CreatureID,
  U3_Dragon_CreatureID,
  U3_Griffon_CreatureID,
  U3_Wyvern_CreatureID,
  U3_Wizard_CreatureID
]

g_eU3SpawnerWildernessMediumPawnArray <-
[
  U3_Giant_CreatureID,
  U3_Golem_CreatureID,
  U3_Titan_CreatureID,
  U3_Daemon_CreatureID,
  U3_Gargoyle_CreatureID,
  U3_Mane_CreatureID
]

g_eU3SpawnerWildernessSeaPawnArray <-
[
  U3_Sea_Serpent_CreatureID,
  U3_Manowar_CreatureID,
  U3_Pirate_Ship_CreatureID
]


class Ultima3World
{
  m_uiMoongateArray = null;


  constructor()
  {
    m_uiMoongateArray = array( MoonPhase.NumPhases, rumInvalidGameID );
  }


  function UpdateMoonPhase()
  {
    foreach( uiWidgetID in m_uiMoongateArray )
    {
      if( uiWidgetID != rumInvalidGameID )
      {
        local ciMoongate = ::rumFetchPawn( uiWidgetID );
        if( ciMoongate != null )
        {
          ciMoongate.OnMoonPhaseChange();
        }
      }
    }

    // Dawn's visibility is based on the moons
    if( MoonPhase.New == g_ciServer.m_ciUltima4World.m_eMoonTrammel )
    {
      if( MoonPhase.New == g_ciServer.m_ciUltima4World.m_eMoonFelucca )
      {
        local ciMap = GetOrCreateMap( null, U3_Sosaria_MapID );
        if( ciMap != null )
        {
          local ciPawnArray = ciMap.GetAllPawns();
          foreach( ciPawn in ciPawnArray )
          {
            if( ciPawn instanceof Portal )
            {
              local eMapID = ciPawn.GetProperty( Map_ID_PropertyID );
              if( eMapID == U3_Towne_Dawn_MapID )
              {
                ciPawn.SetVisibility( true );
                SendClientEffect( ciPawn, ClientEffectType.Cast );
                break;
              }
            }
          }
        }
      }
      else if( MoonPhase.CrescentWaxing == g_ciServer.m_ciUltima4World.m_eMoonFelucca )
      {
        local ciMap = GetOrCreateMap( null, U3_Sosaria_MapID );
        if( ciMap != null )
        {
          local ciPawnArray = ciMap.GetAllPawns();
          foreach( ciPawn in ciPawnArray )
          {
            if( ciPawn instanceof Portal )
            {
              local eMapID = ciPawn.GetProperty( Map_ID_PropertyID );
              if( eMapID == U3_Towne_Dawn_MapID )
              {
                ciPawn.SetVisibility( false );
                break;
              }
            }
          }
        }
      }
    }
  }


  function Update()
  {
    // Do nothing
  }
}


function U3_Init()
{
  // This map should always be active
  GetOrCreateMap( null, U3_Sosaria_MapID );
}
