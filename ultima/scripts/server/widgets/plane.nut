class U2_Plane_Widget extends Passenger_Transport_Widget
{
  static s_fMoveInterval = 0.1;

  m_eLastDir = Direction.North;


  constructor()
  {
    base.constructor();
    SetProperty( State_PropertyID, FlyingStateType.Landed );
  }


  function Ascend( i_ciPlayer )
  {
    if( IsCommander( i_ciPlayer ) )
    {
      if( GetMoveType() != MoveType.Flies )
      {
        // Must have a brass button to start flying!
        local iButtons = i_ciPlayer.GetProperty( U2_Brass_Buttons_PropertyID, 0 );
        if( iButtons > 0 )
        {
          SetMoveType( MoveType.Flies );
          SetProperty( State_PropertyID, FlyingStateType.Flying );

          // Nothing can collide with the plane when it is flying
          SetCollisionFlags( MoveType.Incorporeal );

          // Schedule a movement update
          ::rumSchedule( this, Fly, s_fMoveInterval );

          i_ciPlayer.SetProperty( U2_Brass_Buttons_PropertyID, iButtons - 1 );
        }
        else
        {
          i_ciPlayer.ActionFailed( msg_brass_button_req_client_StringID );
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
    local bBoarded = false;

    // Players must own a skull key to board a plane
    local iSkullKeys = i_ciPlayer.GetProperty( U2_Skull_Keys_PropertyID, 0 );
    if( i_bForce || iSkullKeys > 0 )
    {
      if( base.Board( i_ciPlayer, i_bForce ) )
      {
        if( !i_bForce )
        {
          i_ciPlayer.SetProperty( U2_Skull_Keys_PropertyID, iSkullKeys - 1 );
        }

        bBoarded = true;
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_skull_key_req_client_StringID );
    }

    return bBoarded;
  }


  function CanLandOnTile( i_eTileID )
  {
    return( ( U2_Grass_TileID == i_eTileID ) || ( U2_Floor_Brick_TileID == i_eTileID ) );
  }


  function Descend( i_ciPlayer )
  {
    if( IsCommander( i_ciPlayer ) )
    {
      if( GetMoveType() != MoveType.Terrestrial )
      {
        local ciMap = i_ciPlayer.GetMap();
        local ciPosData = ciMap.GetPositionData( i_ciPlayer.GetPosition() );
        local eTileID = ciPosData.GetTileID();
        if( CanLandOnTile( eTileID ) )
        {
          // Plane must always take off to the north
          m_eLastDir = Direction.North;

          SetMoveType( MoveType.Terrestrial );
          SetCollisionFlags( MoveType.Aquatic | MoveType.Sails );
          SetProperty( State_PropertyID, FlyingStateType.Landed );
          i_ciPlayer.ActionSuccess( command_land_client_StringID );

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


  function Fly()
  {
    if( GetMoveType() == MoveType.Flies )
    {
      local vDir = GetDirectionVector( m_eLastDir );
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
      ::rumSchedule( this, Fly, s_fMoveInterval );
    }
  }


  function TryMove( i_ciPlayer, i_ciPos, i_eDir )
  {
    local eResult = rumFailMoveResultType;

    // Make sure that the player is the transport commander
    if( IsCommander( i_ciPlayer ) )
    {
      local eMoveType = GetMoveType();
      if( MoveType.Flies == eMoveType )
      {
        if( Direction.None == i_eDir )
        {
          // Try to land the plane
          Descend( i_ciPlayer );
        }
        else
        {
          m_eLastDir = i_eDir;
        }

        eResult = rumSuccessMoveResultType;
      }
      else
      {
        eResult = base.TryMove( i_ciPlayer, i_ciPos, i_eDir );
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_not_commander_client_StringID );
    }

    return eResult;
  }
}
