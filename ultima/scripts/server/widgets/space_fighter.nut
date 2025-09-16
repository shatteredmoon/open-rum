class U1_Space_Fighter1_Widget extends Ship_Transport_Widget
{
  function Board( i_ciPlayer, i_bForce = false )
  {
    local bBoarded = Passenger_Transport_Widget.Board.call( this, i_ciPlayer, i_bForce );
    if( bBoarded && !i_bForce )
    {
      i_ciPlayer.ActionSuccess( msg_board_space_ship_client_StringID );
    }
  }


  function Exit( i_ciPlayer, i_ciPos, i_eMoveType )
  {
    local bSuccess = base.Exit( i_ciPlayer, i_ciPos, i_eMoveType );
    if( !bSuccess )
    {
      // Allow player to exit into space
      bSuccess = base.Exit( i_ciPlayer, i_ciPos, MoveType.Celestial );
    }

    return bSuccess;
  }


  function OnAddedToMap()
  {
    local ciMap = GetMap();
    local eMapID = ciMap.GetAssetID();
    if( U1_Space_Station_Alpha_MapID == eMapID )
    {
      Repair( msg_space_craft_repaired_client_StringID );
    }
  }


  function OnCreated()
  {
    local iMaxHitpoints = GetProperty( U1_Hitpoints_PropertyID, 1 );
    SetProperty( Max_Hitpoints_PropertyID, iMaxHitpoints );
    SetProperty( Hitpoints_PropertyID, iMaxHitpoints );
  }


  function TryMove( i_ciPlayer, i_ciPos, i_eDir )
  {
    local eResult = rumFailMoveResultType;

    // Make sure that the player is the transport commander
    if( IsCommander( i_ciPlayer ) )
    {
      if( m_eHeading != i_eDir )
      {
        ChangeHeading( i_ciPlayer, i_eDir );
        eResult = rumSuccessMoveResultType;
      }
      else
      {
        eResult = Passenger_Transport_Widget.TryMove.call( this, i_ciPlayer, i_ciPos, i_eDir );
        if( rumSuccessMoveResultType == eResult )
        {
          local ciMap = GetMap();
          if( rumSuccessMoveResultType == ciMap.MovePawn( this, i_ciPos ) )
          {
            MovePassengers( i_ciPos );
          }
        }
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_not_commander_client_StringID );
    }

    return eResult;
  }
}


class U1_Space_Fighter2_Widget extends Ship_Transport_Widget
{
  function Board( i_ciPlayer, i_bForce = false )
  {
    local bBoarded = Passenger_Transport_Widget.Board.call( this, i_ciPlayer, i_bForce );
    if( bBoarded && !i_bForce )
    {
      i_ciPlayer.ActionSuccess( msg_board_space_ship_client_StringID );
    }
  }


  function Exit( i_ciPlayer, i_ciPos, i_eMoveType )
  {
    local bSuccess = base.Exit( i_ciPlayer, i_ciPos, i_eMoveType );
    if( !bSuccess )
    {
      // Allow player to exit into space
      bSuccess = base.Exit( i_ciPlayer, i_ciPos, MoveType.Celestial );
    }

    return bSuccess;
  }


  function OnAddedToMap()
  {
    local ciMap = GetMap();
    local eMapID = ciMap.GetAssetID();
    if( U1_Space_Station_Alpha_MapID == eMapID )
    {
      Repair( msg_space_craft_repaired_client_StringID );
    }
  }


  function OnCreated()
  {
    local iMaxHitpoints = GetProperty( U1_Hitpoints_PropertyID, 1 );
    SetProperty( Max_Hitpoints_PropertyID, iMaxHitpoints );
    SetProperty( Hitpoints_PropertyID, iMaxHitpoints );
  }


  function TryMove( i_ciPlayer, i_ciPos, i_eDir )
  {
    local eResult = rumFailMoveResultType;

    // Make sure that the player is the transport commander
    if( IsCommander( i_ciPlayer ) )
    {
      if( m_eHeading != i_eDir )
      {
        ChangeHeading( i_ciPlayer, i_eDir );
        eResult = rumSuccessMoveResultType;
      }
      else
      {
        eResult = Passenger_Transport_Widget.TryMove.call( this, i_ciPlayer, i_ciPos, i_eDir );
        if( rumSuccessMoveResultType == eResult )
        {
          local ciMap = GetMap();
          if( rumSuccessMoveResultType == ciMap.MovePawn( this, i_ciPos ) )
          {
            MovePassengers( i_ciPos );
          }
        }
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_not_commander_client_StringID );
    }

    return eResult;
  }
}
