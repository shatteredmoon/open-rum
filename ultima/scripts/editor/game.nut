::rumLoadScript("shared_enums.nut");
::rumLoadScript("u1_shared_enums.nut");
::rumLoadScript("u2_shared_enums.nut");
::rumLoadScript("u3_shared_enums.nut");
::rumLoadScript("u4_shared_enums.nut");

::rumLoadScript("client/enums.nut");
::rumLoadScript("client/u1_enums.nut");
::rumLoadScript("client/u2_enums.nut");
::rumLoadScript("client/u3_enums.nut");
::rumLoadScript("client/u4_enums.nut");

::rumLoadScript("server/enums.nut");
::rumLoadScript("server/u1_enums.nut");
::rumLoadScript("server/u2_enums.nut");
::rumLoadScript("server/u4_enums.nut");

::rumLoadScript( "u1_shared.nut" );
::rumLoadScript( "u2_shared.nut" );
::rumLoadScript( "u3_shared.nut" );
::rumLoadScript( "u4_shared.nut" );
::rumLoadScript("shared.nut");

class Creature extends rumCreature {}
class NPC extends Creature {}

::rumLoadScript("editor/editor.nut");

class Inventory extends rumInventory {}
class Map extends rumMap {}
class Portal extends rumPortal {}


class Widget extends rumWidget
{
  function OnCreated()
  {
    local eAssetID = GetAssetID();

    switch( eAssetID )
    {
      case U4_Moongate_WidgetID:
      case U2_Timegate_WidgetID:
      case U2_Timegate_Facade_WidgetID:
        local eMoonGateAnims = getconsttable()["MoongateAnimFrame"];
        UseAnimationSet( eMoonGateAnims.FullyOpen );
        break;

      case U4_Trap_WidgetID:
      case U3_Trap_WidgetID:
      case U3_Moongate_WidgetID:
      case U3_Moongate_Alt_WidgetID:
        UseAnimationSet( 1 );
        break;
    }
  }
}


class U1_Widget extends Widget {}
class U2_Widget extends Widget {}
class U3_Widget extends Widget {}
class U4_Widget extends Widget {}

class Transport_Widget extends Widget {}
class Passenger_Transport_Widget extends Transport_Widget {}
class Ship_Transport_Widget extends Passenger_Transport_Widget {}

class U1_Door_Widget extends U1_Widget {}
class U1_Door_Locked_Widget extends U1_Door_Widget {}
class U1_Hotspot_Widget extends U1_Widget {}
class U1_Pond_Widget extends U1_Widget {}
class U1_Spawner_Widget extends U1_Widget {}

class U2_Door_Widget extends U2_Widget {}
class U2_Door_Locked_Widget extends U2_Door_Widget {}
class U2_Spawner_Widget extends U2_Widget {}

class U3_Door_Widget extends U3_Widget {}
class U3_Door_Locked_Widget extends U3_Door_Widget {}
class U3_Spawner_Widget extends U3_Widget {}

class U4_Door_Widget extends U4_Widget {}
class U4_Door_Locked_Widget extends U4_Door_Widget {}
class U4_Magic_Field_Widget extends U4_Widget {}
class U4_Spawner_Widget extends U4_Widget {}

class Ladder_Up_Down_Portal extends Portal {}