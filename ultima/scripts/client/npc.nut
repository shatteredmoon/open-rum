class NPC extends Creature
{
  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    base.OnPropertyUpdated( i_ePropertyID, i_vValue );

    if( Camouflaged_PropertyID == i_ePropertyID )
    {
      UseAnimationSet( i_vValue ? 1 : 0 );
    }
    else if( State_PropertyID == i_ePropertyID )
    {
      UseAnimationSet( i_vValue );
    }
  }
}


class U1_NPC extends NPC
{}


class U2_NPC extends NPC
{
  static s_iHitpointTributeCost = 50;
  static s_iStatTributeCost = 100;
  static s_iRoomCost = 50;
  static s_iRoomCostWithTip = 100;
}


class U3_NPC extends NPC
{
  static s_iBribeCost = 100;
}


class U4_NPC extends NPC
{}
