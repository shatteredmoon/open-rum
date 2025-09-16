// Order is important!
::rumLoadScript( "shared_enums.nut" );
::rumLoadScript( "u1_shared_enums.nut" );
::rumLoadScript( "u2_shared_enums.nut" );
::rumLoadScript( "u3_shared_enums.nut" );
::rumLoadScript( "u4_shared_enums.nut" );
::rumLoadScript( "enums.nut" );
::rumLoadScript( "u1_enums.nut" );
::rumLoadScript( "u2_enums.nut" );
::rumLoadScript( "u4_enums.nut" );

::rumLoadScript( "u1_shared.nut" );
::rumLoadScript( "u2_shared.nut" );
::rumLoadScript( "u3_shared.nut" );
::rumLoadScript( "u4_shared.nut" );
::rumLoadScript( "shared.nut" );

::rumLoadScript( "effect.nut" );
::rumLoadFolder( "effects" );

::rumLoadScript( "spell.nut" );
::rumLoadFolder( "spells" );

::rumLoadScript( "widget.nut" );
::rumLoadScript( "transport.nut" );
::rumLoadFolder( "widgets" );

::rumLoadScript( "portal.nut" );
::rumLoadFolder( "portals" );

::rumLoadScript( "map.nut" );
::rumLoadFolder( "maps" );

// These are the main starting maps for each version of the game
g_eWorldStartMapArray <-
[
  null,
  U1_Castle_Lord_British_MapID,
  U2_Earth_BC_Castle_Lord_British_MapID,
  U3_Castle_Lord_British_MapID,
  U4_Castle_British_2_MapID
]

// Last map and position properties
g_eLastMapPropertyVersionArray <-
[
  null,
  U1_LastMap_PropertyID,
  U2_LastMap_PropertyID,
  U3_LastMap_PropertyID,
  U4_LastMap_PropertyID
]

g_eLastPosXPropertyVersionArray <-
[
  null,
  U1_LastPosX_PropertyID,
  U2_LastPosX_PropertyID,
  U3_LastPosX_PropertyID,
  U4_LastPosX_PropertyID
]

g_eLastPosYPropertyVersionArray <-
[
  null,
  U1_LastPosY_PropertyID,
  U2_LastPosY_PropertyID,
  U3_LastPosY_PropertyID,
  U4_LastPosY_PropertyID
]

::rumLoadScript( "creature.nut" );


class AttackTarget
{
  m_uiTargetID = rumInvalidGameID;
  m_ciLastPosition = null;
  m_uiHateLevel = 0;
  m_uiTileDistance = 0;
  m_bInAggroRange = false;
  m_bInLeashRange = false;
  m_bInLOS = false;
  m_bInPrimaryRange = false;
  m_bInSpecialRange = false;
}


::rumLoadScript( "npc.nut" );
::rumLoadFolder( "creatures" );

::rumLoadScript( "player.nut" );

::rumLoadFolder( "broadcasts" );

::rumLoadScript( "party.nut" );

::rumLoadScript( "ultima1.nut" );
::rumLoadScript( "ultima2.nut" );
::rumLoadScript( "ultima3.nut" );
::rumLoadScript( "ultima4.nut" );
::rumLoadScript( "ultima.nut" );

::rumLoadFolder( "merchants" );

::rumLoadScript( "server.nut" );
::rumLoadFolder( "tests" );

class Inventory extends rumInventory {}
