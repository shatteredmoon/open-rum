class U2_Rocket_Widget extends Passenger_Transport_Widget
{
  static s_fMoveInterval = 0.05;

  static s_fDriftInterval = 0.5;
  static s_fOrbitInterval = 10.0;

  m_iLaunchIndex = 0;
  m_eDriftDir = Direction.East;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );
    SetProperty( State_PropertyID, FlyingStateType.Landed );
  }


  function Ascend( i_ciPlayer )
  {
    Launch( i_ciPlayer );
  }


  function CanLandOnTile( i_eTileID )
  {
    return( ( U2_Grass_TileID == i_eTileID ) || ( U2_Floor_Brick_TileID == i_eTileID ) ||
            ( GetMoveType() == MoveType.Celestial ) );
  }


  function Descend( i_ciPlayer )
  {
    if( IsCommander( i_ciPlayer ) )
    {
      local ciMap = GetMap();
      local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
      if( MapType.World == eMapType )
      {
        if( GetMoveType() == MoveType.Flies )
        {
          local ciPosData = ciMap.GetPositionData( i_ciPlayer.GetPosition() );
          local eTileID = ciPosData.GetTileID();
          if( CanLandOnTile( eTileID ) )
          {
            SetMoveType( MoveType.Stationary );
            SetProperty( State_PropertyID, FlyingStateType.Landed );

            i_ciPlayer.ActionSuccess( command_land_client_StringID );

            ++m_iLaunchIndex;

            local ciMap = GetMap();
            foreach( i, uiPassengerID in m_uiPassengerTable )
            {
              local ciPassenger = ::rumGetPlayer( uiPassengerID );
              if( ciPassenger != null )
              {
                ciPassenger.ApplyPositionEffects();
              }
            }
          }
          else
          {
            i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
          }
        }
        else
        {
          i_ciPlayer.ActionFailed( msg_already_landed_client_StringID );
        }
      }
      else if( MapType.Space == eMapType )
      {
        local eMapID = ciMap.GetProperty( U2_Orbit_Map_PropertyID, rumInvalidAssetID );
        if( eMapID != rumInvalidAssetID )
        {
          // Fall to atmosphere
          local eDestMapID = ciMap.GetProperty( U2_Orbit_Map_PropertyID, U2_Earth_Aftermath_World_MapID );
          Portal( eDestMapID, rumPos( 0, 0 ) );

          SetMoveType( MoveType.Flies );

          // Reverse direction for landing
          ++m_iLaunchIndex;

          m_eDriftDir = rand() % 2 == 0 ? Direction.West : Direction.East;

          Land( m_iLaunchIndex );
          ::rumSchedule( this, Drift, s_fDriftInterval, m_iLaunchIndex );
        }
        else
        {
          i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
        }
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_not_commander_client_StringID );
    }
  }


  function Drift( i_iLaunchIndex )
  {
    if( i_iLaunchIndex == m_iLaunchIndex )
    {
      local vDir = GetDirectionVector( m_eDriftDir );
      local ciPos = GetPosition() + vDir;

      local ciMap = GetMap();
      local eResult = ciMap.MovePawn( this, ciPos );
      if( rumSuccessMoveResultType == eResult )
      {
        MovePassengers( ciPos );
      }
      else if( rumOffMapMoveResultType == eResult )
      {
        local eExitMapID = ciMap.GetExitMapID();
        local ciPos = ciMap.GetExitPosition();
        Portal( eExitMapID, ciPos );
      }

      // Schedule another drift update
      ::rumSchedule( this, Drift, s_fDriftInterval, i_iLaunchIndex );
    }
  }


  function Exit( i_ciPlayer, i_ciPos, i_eMoveType )
  {
    local bExited = false;

    if( ( GetMoveType() == MoveType.Stationary ) || i_ciPlayer.IsDead() )
    {
      bExited = base.Exit( i_ciPlayer, i_ciPos, i_eMoveType )
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_must_land_client_StringID );
    }

    return bExited;
  }


  function Fly( i_iLaunchIndex )
  {
    if( i_iLaunchIndex == m_iLaunchIndex )
    {
      local vDir = GetDirectionVector( Direction.North );
      local ciPos = GetPosition() + vDir;

      local ciMap = GetMap();
      local eResult = ciMap.MovePawn( this, ciPos );
      if( rumSuccessMoveResultType == eResult )
      {
        MovePassengers( ciPos );
      }
      else if( rumOffMapMoveResultType == eResult )
      {
        local eExitMapID = ciMap.GetExitMapID();
        local ciPos = ciMap.GetExitPosition();
        Portal( eExitMapID, ciPos );
      }

      // Schedule another movement update
      ::rumSchedule( this, Fly, s_fMoveInterval, i_iLaunchIndex );
    }
  }


  function HyperJump( i_ciPlayer, i_iCoordinatesArray )
  {
    if( IsCommander( i_ciPlayer ) )
    {
      // Must have Tri-Lithium to hyperjump!
      local iTriLithium = i_ciPlayer.GetProperty( U2_Tri_Lithium_PropertyID, 0 );
      if( iTriLithium > 0 )
      {
        local bJumped = false;

        // Determine which map to load based on provided coordinates
        foreach( eMapID in g_eU2SpaceMapArray )
        {
          local ciMap = ::rumGetMapAsset( eMapID );
          if( ciMap != null )
          {
            local iXeno = ciMap.GetProperty( U2_Xeno_Coordinate_PropertyID, 0 );
            local iYako = ciMap.GetProperty( U2_Yako_Coordinate_PropertyID, 0 );
            local iZebo = ciMap.GetProperty( U2_Zebo_Coordinate_PropertyID, 0 );
            if( ( iXeno == i_iCoordinatesArray[0] ) &&
                ( iYako == i_iCoordinatesArray[1] ) &&
                ( iZebo == i_iCoordinatesArray[2] ) )
            {
              bJumped = Portal( eMapID, rumPos( 0, 0 ) );
              break;
            }
          }
        }

        if( bJumped )
        {
          foreach( uiPassengerID in m_uiPassengerTable )
          {
            local ciPassenger = ::rumGetPlayer( uiPassengerID );
            if( ciPassenger != null )
            {
              SendClientEffect( ciPassenger, ClientEffectType.ScreenShake );
              SendClientEffect( ciPassenger, ClientEffectType.Cast );
            }
          }
        }
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_trilithium_req_client_StringID );
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_not_commander_client_StringID );
    }
  }


  function Land( i_iLaunchIndex )
  {
    if( ( i_iLaunchIndex == m_iLaunchIndex ) && ( GetMoveType() == MoveType.Flies ) )
    {
      local vDir = GetDirectionVector( Direction.South );
      local ciPos = GetPosition() + vDir;

      local ciMap = GetMap();
      local eResult = ciMap.MovePawn( this, ciPos );
      if( rumSuccessMoveResultType == eResult )
      {
        MovePassengers( ciPos );
      }
      else if( rumOffMapMoveResultType == eResult )
      {
        local eExitMapID = ciMap.GetExitMapID();
        local ciPos = ciMap.GetExitPosition();
        Portal( eExitMapID, ciPos );
      }

      // Schedule another movement update
      ::rumSchedule( this, Land, s_fMoveInterval, i_iLaunchIndex );
    }
  }


  function Launch( i_ciPlayer )
  {
    if( IsCommander( i_ciPlayer ) )
    {
      if( GetMoveType() == MoveType.Stationary )
      {
        // Must have Tri-Lithium to launch!
        local iTriLithium = i_ciPlayer.GetProperty( U2_Tri_Lithium_PropertyID, 0 );
        if( iTriLithium > 0 )
        {
          SetMoveType( MoveType.Flies );
          SetProperty( State_PropertyID, FlyingStateType.Flying );

          ++m_iLaunchIndex;

          m_eDriftDir = rand() % 2 == 0 ? Direction.West : Direction.East;

          // Schedule a movement update
          ::rumSchedule( this, Fly, s_fMoveInterval, m_iLaunchIndex );
          ::rumSchedule( this, Drift, s_fDriftInterval, m_iLaunchIndex );
          ::rumSchedule( this, Orbit, s_fOrbitInterval, m_iLaunchIndex );

          i_ciPlayer.SetProperty( U2_Tri_Lithium_PropertyID, iTriLithium - 1 );
        }
        else
        {
          i_ciPlayer.ActionFailed( msg_trilithium_req_client_StringID );
        }
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_not_commander_client_StringID );
    }
  }


  function Orbit( i_iOrbitIndex )
  {
    if( i_iOrbitIndex == m_iLaunchIndex )
    {
      // Leave the planet
      local ciMap = GetMap();
      local eSpaceMapID = ciMap.GetProperty( Space_Map_PropertyID, rumInvalidAssetID );
      if( eSpaceMapID != rumInvalidAssetID )
      {
        Portal( eSpaceMapID, rumPos( 0, 0 ) );
        SetMoveType( MoveType.Celestial );

        // Kill any passenger not wearing a vacuum or reflect suit
        foreach( uiPassengerID in m_uiPassengerTable )
        {
          local ciPassenger = ::rumGetPlayer( uiPassengerID );
          if( ciPassenger != null )
          {
            local eArmourType = ciPassenger.GetArmourType();
            if( U2_Reflect_Armour_InventoryID != eArmourType )
            {
              ciPassenger.Kill( null );
              ciPassenger.ActionFailed( msg_space_equipment_req_client_StringID );
            }
          }
        }
      }
    }
  }


  function Relaunch()
  {
    // Used when logging back in after logging out in space
    SetMoveType( MoveType.Celestial );
    SetProperty( State_PropertyID, FlyingStateType.Flying );

    ++m_iLaunchIndex;

    m_eDriftDir = rand() % 2 == 0 ? Direction.West : Direction.East;

    // Schedule a movement update
    ::rumSchedule( this, Fly, s_fMoveInterval, m_iLaunchIndex );
    ::rumSchedule( this, Drift, s_fDriftInterval, m_iLaunchIndex );
  }


  function TryMove( i_ciPlayer, i_ciPos, i_eDir )
  {
    local eResult = rumFailMoveResultType;

    // Make sure that the player is the transport commander
    if( !IsCommander( i_ciPlayer ) )
    {
      i_ciPlayer.ActionFailed( msg_not_commander_client_StringID );
    }

    return eResult;
  }
}
