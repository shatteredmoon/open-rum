class U4_Air_Balloon_Widget extends Passenger_Transport_Widget
{
  static s_fDriftTime = 1.0;

  m_ciBalloonOrigin = null;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );
    SetProperty( State_PropertyID, FlyingStateType.Landed );
  }


  function Ascend( i_ciPlayer )
  {
    if( IsCommander( i_ciPlayer ) )
    {
      if( GetMoveType() != MoveType.Drifts )
      {
        SetMoveType( MoveType.Drifts );
        SetProperty( State_PropertyID, FlyingStateType.Flying );

        // Schedule a movement update
        ::rumSchedule( this, Drift, s_fDriftTime );
      }

      i_ciPlayer.ActionSuccess( command_klimb_altitude_client_StringID );
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_not_commander_client_StringID );
    }
  }


  function Board( i_ciPlayer, i_bForce = false )
  {
    local bBoarded = false;

    local eState = GetProperty( State_PropertyID, FlyingStateType.Landed );
    if( i_bForce || ( FlyingStateType.Landed == eState ) )
    {
      bBoarded = base.Board( i_ciPlayer, i_bForce );
      if( bBoarded && !i_bForce )
      {
        i_ciPlayer.ActionSuccess( msg_board_balloon_client_StringID );
      }
    }
    else
    {
      // Balloon must be landed
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
    }

    return bBoarded;
  }


  function CanLandOnTile( i_eTileID )
  {
    return ( U4_Grass_TileID == i_eTileID );
  }


  function Cleanup( i_iIdleStamp )
  {
    if( ( i_iIdleStamp == m_iIdleStamp ) && ( m_uiPassengerTable.len() == 0 ) )
    {
      // TODO - steer the balloon back to its origin instead?
      local ciMap = GetMap();
      ciMap.TransferPawn( this, ciMap, m_ciBalloonOrigin );

      SetMoveType( MoveType.Stationary );
      SetProperty( State_PropertyID, FlyingStateType.Landed );
    }
  }


  function Descend( i_ciPlayer )
  {
    if( IsCommander( i_ciPlayer ) )
    {
      if( GetMoveType() != MoveType.Stationary )
      {
        local ciMap = i_ciPlayer.GetMap();
        local ciPosData = ciMap.GetPositionData( i_ciPlayer.GetPosition() );
        local eTileID = ciPosData.GetTileID();
        if( CanLandOnTile( eTileID ) )
        {
          SetMoveType( MoveType.Stationary );
          SetProperty( State_PropertyID, FlyingStateType.Landed );
          i_ciPlayer.ActionSuccess( u4_command_land_balloon_client_StringID );

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
    else
    {
      i_ciPlayer.ActionFailed( msg_not_commander_client_StringID );
    }
  }


  function Drift()
  {
    if( GetMoveType() == MoveType.Drifts )
    {
      local eDir = Direction.None;

      // Fetch the transport commander
      local ciCommander = GetCommander();
      if( ciCommander != null )
      {
        eDir = ciCommander.GetProperty( U4_Wind_Direction_Override_PropertyID, Direction.None );
      }

      local vDir;
      if( Direction.None != eDir )
      {
        vDir = GetDirectionVector( eDir );
      }
      else
      {
        vDir = GetDirectionVector( g_ciServer.m_ciUltima4World.m_eWindDirection );
      }

      // Move in the direction of the wind
      local ciPos = GetPosition() + vDir;

      local ciMap = GetMap();
      local eResult = ciMap.MovePawn( this, ciPos );
      if( rumSuccessMoveResultType == eResult )
      {
        MovePassengers( ciPos );
      }

      // Schedule another movement update
      ::rumSchedule( this, Drift, s_fDriftTime );
    }
  }


  function Exit( i_ciPlayer, i_ciPos, i_eMoveType )
  {
    local bExited = false;

    local eState = GetProperty( State_PropertyID, FlyingStateType.Landed );
    if( ( FlyingStateType.Landed == eState ) || i_ciPlayer.IsDead() )
    {
      bExited = base.Exit( i_ciPlayer, i_ciPos, i_eMoveType )
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_must_land_client_StringID );
    }

    return bExited;
  }


  function OnAddedToMap()
  {
    // There should only be one balloon in the entire game. When it loads, its position is stored and made
    // accessible so that the balloon can return to its origin position and so that players who log out on
    // the balloon can be restarted at the balloon's origin position as well.
    m_ciBalloonOrigin = GetPosition();
  }


  function TryMove( i_ciPlayer, i_ciPos, i_eDir )
  {
    local eResult = rumFailMoveResultType;

    // Make sure that the player is the transport commander
    if( IsCommander( i_ciPlayer ) )
    {
      local eMoveType = GetMoveType();
      if( MoveType.Stationary == eMoveType )
      {
        i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_drift_only_client_StringID );
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_not_commander_client_StringID );
    }

    return eResult;
  }
}
