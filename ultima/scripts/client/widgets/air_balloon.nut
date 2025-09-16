class U4_Air_Balloon_Widget extends Passenger_Transport_Widget
{
  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );
    SetProperty( State_PropertyID, FlyingStateType.Landed );
  }


  function CanLandOnTile( i_eTileID )
  {
    return ( U4_Grass_TileID == i_eTileID );
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
