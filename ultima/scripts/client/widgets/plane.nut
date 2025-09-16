class U2_Plane_Widget extends Passenger_Transport_Widget
{
  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Terrestrial );
    SetProperty( State_PropertyID, FlyingStateType.Landed );
  }


  function CanLandOnTile( i_eTileID )
  {
    return ( ( U2_Grass_TileID == i_eTileID ) || ( U2_Floor_Brick_TileID == i_eTileID ) );
  }


  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    base.OnPropertyUpdated( i_ePropertyID, i_vValue );

    local ciPlayer = ::rumGetMainPlayer();
    local uiTransportID = ciPlayer.GetTransportID();
    if( uiTransportID != GetID() )
    {
      // We only care about state changes for the local player
      return;
    }

    switch( i_ePropertyID )
    {
      case State_PropertyID:
      {
        local ciMap = GetMap();
        g_ciCUO.m_bAloft = ( FlyingStateType.Flying == i_vValue );
        ciMap.SetPeeringAttributes();
        break;
      }
    }
  }
}
