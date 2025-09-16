/*

----------------------------------------------------------------------------

ooooo  oooo o888    o8    o88                                ooooo
 888    88   888  o888oo  oooo  oo ooo oooo     ooooooo       888
 888    88   888   888     888   888 888 888    ooooo888      888
 888    88   888   888     888   888 888 888  888    888      888
  888oo88   o888o   888o  o888o o888o888o888o  88ooo88 8o    ooooo

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

Ultima I: The First Age of Darkness
    Developers: Richard Garriott, Origin Systems
    Publishers: California Pacific Computer Co., Origin Systems, Electronic Arts
    Designers:  Richard Garriott, Ken W. Arnold

*/

g_eU1KingQuestPropertyArray <-
[
  U1_Quest_LostKing_PropertyID
  U1_Quest_Rondorin_PropertyID
  U1_Quest_BlackDragon_PropertyID
  U1_Quest_Shamino_PropertyID
  U1_Quest_LordBritish_PropertyID
  U1_Quest_Barataria_PropertyID
  U1_Quest_WhiteDragon_PropertyID
  U1_Quest_Olympus_PropertyID
]

g_eU1SpawnerDungeon12PawnArray <-
[
  U1_Bat_CreatureID,
  U1_Giant_Rat_CreatureID,
  U1_Evil_Ranger_CreatureID,
  U1_Skeleton_CreatureID,
  U1_Thief_CreatureID,
  U1_Chest_WidgetID
]

g_eU1SpawnerDungeon34PawnArray <-
[
  U1_Cyclops_CreatureID,
  U1_Gelatinous_Cube_CreatureID,
  U1_Orc_CreatureID,
  U1_Spider_CreatureID,
  U1_Viper_CreatureID,
  U1_Chest_WidgetID
]

g_eU1SpawnerDungeon56PawnArray <-
[
  U1_Carrion_Creeper_CreatureID,
  U1_Ettin_CreatureID,
  U1_Lizard_Man_CreatureID,
  U1_Mimic_CreatureID,
  U1_Minotaur_CreatureID,
  U1_Chest_WidgetID
]

g_eU1SpawnerDungeon78PawnArray <-
[
  U1_Gremlin_CreatureID,
  U1_Lich_CreatureID,
  U1_Tangler_CreatureID,
  U1_Wandering_Eyes_CreatureID,
  U1_Wraith_CreatureID,
  U1_Chest_WidgetID
]

g_eU1SpawnerDungeon910PawnArray <-
[
  U1_Balron_CreatureID,
  U1_Daemon_CreatureID,
  U1_Invisible_Seeker_CreatureID,
  U1_Mind_Whipper_CreatureID,
  U1_Zorn_CreatureID,
  U1_Chest_WidgetID
]

g_eU1SpawnerSpaceStationPawnArray <-
[
  U1_Shuttle_WidgetID,
  U1_Space_Fighter1_WidgetID,
  U1_Space_Fighter2_WidgetID
]

g_aU1SpawnerWildernessCommonPawnArray <-
[
  U1_Bear_CreatureID,
  U1_Evil_Ranger_CreatureID,
  U1_Hidden_Archer_CreatureID,
  U1_Evil_Trent_CreatureID,
  U1_Hood_CreatureID,
  U1_Knight_CreatureID,
  U1_Orc_CreatureID,
  U1_Thief_CreatureID
]

g_eU1SpawnerWildernessSeaPawnArray <-
[
  U1_Dragon_Turtle_CreatureID,
  U1_Giant_Squid_CreatureID,
  U1_Ness_CreatureID,
  U1_Pirate_Ship_CreatureID
]

g_eU1SpawnerWildernessSpacePawnArray <-
[
  U1_Space_Enemy_CreatureID
]

g_eU1SpawnerWildernessUncommonPawnArray <-
[
  U1_Dark_Knight_CreatureID,
  U1_Necromancer_CreatureID,
  U1_Warlock_CreatureID
]

g_tblU1SignQuestArray <-
[
  // PillarsProtection
  {
    ePropertyID = U1_Quest_Olympus_PropertyID,
    eState = U1_KingQuestState.Bestowed2,
    vReward = U1_Agility_PropertyID
  },

  // TowerKnowledge
  {
    ePropertyID = U1_Quest_LordBritish_PropertyID,
    eState = U1_KingQuestState.Bestowed1,
    vReward = U1_Intelligence_PropertyID
  },

  // PillarsArgonauts
  {
    ePropertyID = U1_Quest_WhiteDragon_PropertyID,
    eState = U1_KingQuestState.Bestowed2,
    vReward = g_eU1WeaponInventoryArray
  },

  // PillarOzymandias
  {
    ePropertyID = U1_Quest_Barataria_PropertyID,
    eState = U1_KingQuestState.Bestowed1,
    vReward = U1_Wisdom_PropertyID
  },

  // GraveLostSoul
  {
    ePropertyID = U1_Quest_WhiteDragon_PropertyID,
    eState = U1_KingQuestState.Bestowed1,
    vReward = U1_Strength_PropertyID
  },

  // EasternSignPost
  {
    ePropertyID = U1_Quest_LordBritish_PropertyID,
    eState = U1_KingQuestState.Bestowed2,
    vReward = null // Random Stat
  },

  // TheSignPost
  {
    ePropertyID = U1_Quest_Barataria_PropertyID,
    eState = U1_KingQuestState.Bestowed2,
    vReward = U1_Stamina_PropertyID
  },

  // SouthernSignPost
  {
    ePropertyID = U1_Quest_Olympus_PropertyID,
    eState = U1_KingQuestState.Bestowed1,
    vReward = U1_Charisma_PropertyID
  }
]


class Ultima1World
{
  function Update()
  {
    // Do nothing
  }
}


function U1_Init()
{
  // This map should always be active
  GetOrCreateMap( null, U1_Sosaria_MapID );
}
