class Transport_Widget extends Widget
{
  m_uiPassengerID = rumInvalidGameID;
  static s_fIdleInterval = 30.0;


  function AddPassenger( i_uiPlayerID )
  {
    m_uiPassengerID = i_uiPlayerID;
  }


  function CanLandOnTile( i_eTileID )
  {
    return false;
  }


  function GetCommander()
  {
    local uiCommanderID = GetCommanderID();
    return uiCommanderID != rumInvalidGameID ? ::rumFetchPawn( uiCommanderID ) : null;
  }


  function GetCommanderID()
  {
    return GetProperty( Transport_Commander_ID_PropertyID, rumInvalidGameID );
  }


  function GetType()
  {
    return GetProperty( Transport_Type_PropertyID, TransportType.None );
  }


  function GetWeapon()
  {
    local eWeaponType = GetWeaponType();
    return eWeaponType ? ::rumGetInventoryAsset( eWeaponType ) : null;
  }


  function GetWeaponType()
  {
    return GetProperty( Transport_Weapon_PropertyID, rumInvalidAssetID );
  }


  function RemovePassenger( i_uiPlayerID )
  {
    m_uiPassengerID = rumInvalidGameID;
  }


  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    switch( i_ePropertyID )
    {
      case State_PropertyID:
        // State drives animation!
        UseAnimationSet( i_vValue );

        // Update the passenger as well - it may be the only thing getting rendered at this point
        local ciPlayer = ::rumFetchPawn( m_uiPassengerID );
        if( ciPlayer != null )
        {
          ciPlayer.UseAnimationSet( i_vValue );
        }
        break;

      case Transport_Commander_ID_PropertyID:
        local ciPlayer = ::rumGetMainPlayer();
        if( ciPlayer.GetID() == i_vValue )
        {
          local ciTransport = ciPlayer.GetTransport();

          // Tell the player they command the transport only if it holds more than one passenger
          if( ciTransport instanceof Passenger_Transport_Widget )
          {
            // Inform the player that they are now the ship commander
            ShowString( ::rumGetString( msg_commander_client_StringID ), g_strColorTagArray.Yellow );
          }
        }
        break;
    }
  }
}


class Passenger_Transport_Widget extends Transport_Widget
{
  constructor()
  {
    base.constructor();
    SetProperty( Transport_Commander_ID_PropertyID, rumInvalidGameID );
  }
}


class Ship_Transport_Widget extends Passenger_Transport_Widget
{
}
