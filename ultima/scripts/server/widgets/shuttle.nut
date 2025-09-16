class U1_Shuttle_Widget extends Ship_Transport_Widget
{
  static s_fLaunchInterval = 10.0;

  m_iLaunchStamp = 0;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Celestial );

    SetProperty( State_PropertyID, ShipStateType.FacingNorth );
  }


  function AbortLaunch( i_ciPlayer )
  {
    if( IsCommander( i_ciPlayer ) )
    {
      // The commander is leaving the transport, so abort the launch
      foreach( uiPassengerID in m_uiPassengerTable )
      {
        ++m_iLaunchStamp;

        local ciPassenger = ::rumGetPlayer( uiPassengerID );
        if( ciPassenger != null )
        {
          local ciBroadcast = ::rumCreate( Player_Transport_BroadcastID, TransportCommandType.ShuttleLaunchAbort );
          ciPassenger.SendBroadcast( ciBroadcast );
        }
      }
    }
  }


  function Ascend( i_ciPlayer )
  {
    if( IsCommander( i_ciPlayer ) )
    {
      local ciMap = GetMap();
      local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
      if( MapType.Towne == eMapType )
      {
        foreach( uiPassengerID in m_uiPassengerTable )
        {
          local ciPassenger = ::rumGetPlayer( uiPassengerID );
          if( ciPassenger != null )
          {
            local ciBroadcast = ::rumCreate( Player_Transport_BroadcastID, TransportCommandType.ShuttleLaunch );
            ciPassenger.SendBroadcast( ciBroadcast );
          }

          ::rumSchedule( this, Launch, s_fLaunchInterval, ++m_iLaunchStamp );
        }
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_not_commander_client_StringID );
    }
  }


  function Board( i_ciPlayer, i_bForce = false )
  {
    local bHasShuttlePass = true;

    local ciMap = GetMap();
    local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
    if( MapType.Towne == eMapType )
    {
      // The player must have a shuttle pass to board a shuttle
      bHasShuttlePass = i_ciPlayer.GetProperty( U1_Shuttle_Pass_PropertyID, false );
    }

    if( i_bForce || bHasShuttlePass )
    {
      local bBoarded = Passenger_Transport_Widget.Board.call( this, i_ciPlayer, i_bForce );
      if( bBoarded && !i_bForce )
      {
        i_ciPlayer.ActionSuccess( msg_board_shuttle_client_StringID );
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_no_shuttle_pass_client_StringID );
    }
  }


  function Exit( i_ciPlayer, i_ciPos, i_eMoveType )
  {
    AbortLaunch( i_ciPlayer );

    local bSuccess = base.Exit( i_ciPlayer, i_ciPos, i_eMoveType );
    if( !bSuccess )
    {
      // Allow player to exit into space
      bSuccess = base.Exit( i_ciPlayer, i_ciPos, MoveType.Celestial );
    }

    return bSuccess;
  }


  function HandleLogout( i_ciPlayer )
  {
    AbortLaunch( i_ciPlayer );
    base.HandleLogout( i_ciPlayer );
  }


  function Launch( i_iLaunchStamp )
  {
    if( i_iLaunchStamp != m_iLaunchStamp )
    {
      return;
    }

    // Countdown finished, launch the shuttle into space
    local ciMap = GetMap();
    local ciDestMap = GetOrCreateMap( GetCommander(), U1_Space_MapID );
    if( null == ciDestMap )
    {
      return;
    }

    local ciEntryPos = rumPos( 0, 0 );

    local ciPawnArray = ciDestMap.GetAllPawns();
    foreach( ciPawn in ciPawnArray )
    {
      if( ciPawn.GetAssetID() == U1_Sosaria_PortalID )
      {
        ciEntryPos = ciPawn.GetPosition();
        break;
      }
    }

    ciMap.TransferPawn( this, ciDestMap, ciEntryPos );

    // Transfer each passenger and remove their shuttle pass
    foreach( uiPassengerID in m_uiPassengerTable )
    {
      local ciPassenger = ::rumGetPlayer( uiPassengerID );
      if( ciPassenger != null )
      {
        ciPassenger.RemoveProperty( U1_Shuttle_Pass_PropertyID );
        ciMap.TransferPawn( ciPassenger, ciDestMap, ciEntryPos );

        local ciBroadcast = ::rumCreate( Player_Board_BroadcastID, GetID() );
        ciPassenger.SendBroadcast( ciBroadcast );

        // Kill any passenger not wearing a vacuum or reflect suit
        local eArmourType = ciPassenger.GetArmourType();
        if( U1_Reflect_Suit_Armour_InventoryID != eArmourType && U1_Vacuum_Suit_Armour_InventoryID != eArmourType )
        {
          ciPassenger.Kill( null );
          ciPassenger.ActionFailed( msg_space_equipment_req_client_StringID );
        }
      }
    }
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
    local iMaxHitpoints = GetProperty( U1_Hitpoints_PropertyID, 100 );
    SetProperty( Max_Hitpoints_PropertyID, iMaxHitpoints );
    SetProperty( Hitpoints_PropertyID, iMaxHitpoints );
  }


  function TryMove( i_ciPlayer, i_ciPos, i_eDir )
  {
    local eResult = rumFailMoveResultType;

    // Make sure that the player is the transport commander
    if( IsCommander( i_ciPlayer ) )
    {
      local ciMap = GetMap();
      local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
      if( MapType.Towne == eMapType )
      {
        i_ciPlayer.ActionFailed( msg_launch_client_StringID );
      }
      else
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
            if( rumSuccessMoveResultType == ciMap.MovePawn( this, i_ciPos ) )
            {
              MovePassengers( i_ciPos );
            }
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
