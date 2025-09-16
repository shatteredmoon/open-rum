// Order is important!
::rumLoadScript( "shared_enums.nut" );
::rumLoadScript( "u1_shared_enums.nut" );
::rumLoadScript( "u2_shared_enums.nut" );
::rumLoadScript( "u3_shared_enums.nut" );
::rumLoadScript( "u4_shared_enums.nut" );
::rumLoadScript( "enums.nut" );
::rumLoadScript( "u1_enums.nut" );
::rumLoadScript( "u2_enums.nut" );
::rumLoadScript( "u3_enums.nut" );
::rumLoadScript( "u4_enums.nut" );

::rumLoadFolder( "controls" );
::rumLoadScript( "input.nut" );

::rumLoadScript( "effects.nut" );

::rumLoadScript( "utility.nut" );

::rumLoadScript( "widget.nut" );
::rumLoadScript( "transport.nut" );
::rumLoadFolder( "widgets" );

::rumLoadScript( "portal.nut" );
::rumLoadFolder( "portals" );

::rumLoadScript( "creature.nut" );
::rumLoadScript( "npc.nut" );
::rumLoadFolder( "creatures" );

g_eGraphicIDPropertyVersionArray <-
[
  null,
  U1_Graphic_ID_PropertyID,
  U2_Graphic_ID_PropertyID,
  U3_Graphic_ID_PropertyID,
  U4_Graphic_ID_PropertyID
]

::rumLoadScript( "player.nut" );
::rumLoadScript( "inventory.nut" );

::rumLoadScript( "map.nut" );
::rumLoadFolder( "maps" );

::rumLoadScript( "u1_shared.nut" );
::rumLoadScript( "u2_shared.nut" );
::rumLoadScript( "u3_shared.nut" );
::rumLoadScript( "u4_shared.nut" );
::rumLoadScript( "shared.nut" );

::rumLoadFolder( "broadcasts" );

::rumLoadScript( "ultima1.nut" );
::rumLoadScript( "ultima2.nut" );
::rumLoadScript( "ultima3.nut" );
::rumLoadScript( "ultima4.nut" );
::rumLoadScript( "ultima.nut" );

::rumLoadScript( "client.nut" );

::rumLoadScript( "title.nut" );
::rumLoadFolder( "screens" );
