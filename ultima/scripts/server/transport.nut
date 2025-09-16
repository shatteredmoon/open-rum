class Transport_Widget extends Widget
{
  static s_iMaxCapacity = 1;
  static s_bIdleCleanup = true;
  static s_fIdleInterval = 30.0;

  m_eHeading = Direction.West;

  m_iIdleStamp = 0;

  m_uiTransportCode = 0;
  m_uiPassengerTable = null;


  constructor()
  {
    base.constructor();

    SetMoveType( MoveType.Terrestrial );
    local iMaxHitpoints = GetProperty( Max_Hitpoints_PropertyID, 99 );
    SetProperty( Hitpoints_PropertyID, iMaxHitpoints );

    while( 0 == m_uiTransportCode )
    {
      m_uiTransportCode = ::rumGetRandom64();
    }

    m_uiPassengerTable = {};
  }


  function Board( i_ciPlayer, i_bForce = false )
  {
    local ciCommander = GetCommander();

    if( !i_bForce )
    {
      if( m_uiPassengerTable.len() >= s_iMaxCapacity )
      {
        i_ciPlayer.ActionFailed( msg_no_room_client_StringID );
        return false;
      }

      // Fetch the current commander
      if( ciCommander != null )
      {
        // Is the boarding player a member of the commander's party?
        local ciParty = ciCommander.GetParty();
        if( ciParty != null )
        {
          if( !ciParty.HasMember( i_ciPlayer.GetID() ) )
          {
            i_ciPlayer.ActionFailed( msg_not_in_party_client_StringID );
            return false;
          }
        }
        else
        {
          i_ciPlayer.ActionFailed( msg_not_in_party_client_StringID );
          return false;
        }
      }
    }

    local ciPos = GetPosition();

    // If the player is already on a transport, exit that transport
    local ciTransport = i_ciPlayer.GetTransport();
    if( ciTransport != null )
    {
      local bExited = ciTransport.Exit( i_ciPlayer, ciPos, MoveType.Incorporeal );
      if( !bExited )
      {
        return false;
      }
    }

    local ciMap = GetMap();
    ciMap.MovePawn( i_ciPlayer, ciPos,
                    rumIgnoreTileCollisionMoveFlag | rumIgnorePawnCollisionMoveFlag | rumIgnoreDistanceMoveFlag );

    SetVisibility( false );

    local uiTransportID = GetID();
    local uiPlayerID = i_ciPlayer.GetID();

    // Notify the client of the transport/player link
    local ciBroadcast = ::rumCreate( Player_Board_BroadcastID, uiTransportID );
    i_ciPlayer.SendBroadcast( ciBroadcast );

    // Add the player as a passenger
    m_uiPassengerTable[uiPlayerID] <- uiPlayerID;

    i_ciPlayer.SetVersionedProperty( g_eTransportIDPropertyVersionArray, uiTransportID );
    i_ciPlayer.SetVersionedProperty( g_eTransportBoardedWidgetPropertyVersionArray, GetAssetID() );

    if( ( null == ciCommander ) || ( ciCommander.GetID() == rumInvalidGameID ) )
    {
      // This player is the new commander
      SetProperty( Transport_Commander_ID_PropertyID, uiPlayerID );
    }

    return true;
  }


  function CanLandOnTile( i_eTileID )
  {
    return false;
  }


  function ChangeHeading( i_ciPlayer, i_eDir )
  {
    local iClockwiseTurns = 0;
    local iCounterClockwiseTurns = 0;
    if( m_eHeading > i_eDir )
    {
      iClockwiseTurns = ( i_eDir + 8 ) - m_eHeading;
      iCounterClockwiseTurns = m_eHeading - i_eDir;
    }
    else
    {
      iClockwiseTurns = i_eDir - m_eHeading;
      iCounterClockwiseTurns = m_eHeading - ( i_eDir - 8 );
    }

    if( iClockwiseTurns < iCounterClockwiseTurns )
    {
      local eNewHeading;

      // Turn clockwise
      for( local i = 0; i < iClockwiseTurns; ++i )
      {
        ++m_eHeading;
        if( m_eHeading > Direction.Southwest )
        {
          m_eHeading = Direction.West;
        }

        DetermineState();
      }
    }
    else
    {
      // Turn counter-clockwise
      for( local i = 0; i < iCounterClockwiseTurns; ++i )
      {
        --m_eHeading;
        if( m_eHeading < Direction.West )
        {
          m_eHeading = Direction.Southwest;
        }

        DetermineState();
      }
    }

    local ciBroadcast = ::rumCreate( Command_Result_BroadcastID, msg_turning_client_StringID,
                                     g_strColorTagArray.White );
    i_ciPlayer.SendBroadcast( ciBroadcast );
  }


  function Cleanup( i_iIdleStamp )
  {
    if( ( i_iIdleStamp == m_iIdleStamp ) && ( m_uiPassengerTable.len() == 0 ) )
    {
      Destroy();
    }
  }


  function Damage( i_iAmount, i_ciSource, i_eWeaponType = rumInvalidAssetID, i_bSendClientEffect = true )
  {
    if( i_iAmount <= 0 )
    {
      return false;
    }

    local iHitpoints = GetHitpoints() - i_iAmount;
    SetProperty( Hitpoints_PropertyID, iHitpoints );

    if( i_bSendClientEffect )
    {
      SendClientEffect( this, ClientEffectType.Damage );
    }

    if( iHitpoints <= 0 )
    {
      if( i_ciSource instanceof Creature )
      {
        // Inform the attacker
        i_ciSource.OnKilled( this, false );
      }

      Destroy();
    }

    return true;
  }


  function Destroy()
  {
    local i_ciPos = GetPosition();
    local ciCommander = null;
    local uiCommanderID = GetCommanderID();

    foreach( uiPlayerID in m_uiPassengerTable )
    {
      local ciPlayer = ::rumFetchPawn( uiPlayerID );
      if( uiCommanderID != uiPlayerID )
      {
        if( !Exit( ciPlayer, i_ciPos, ciPlayer.s_eDefaultMoveType ) )
        {
          // Force exit and kill the player
          Exit( ciPlayer, i_ciPos, MoveType.Incorporeal );
          ciPlayer.Kill( this );
        }

        RemovePlayerTransportSettings( ciPlayer );
      }
      else
      {
        ciCommander = ciPlayer;
      }
    }

    if( ciCommander != null )
    {
      if( !Exit( ciCommander, i_ciPos, ciCommander.s_eDefaultMoveType ) )
      {
        // Force exit and kill the player
        Exit( ciCommander, i_ciPos, MoveType.Incorporeal );
        ciCommander.Kill( this );
      }

      RemovePlayerTransportSettings( ciCommander );
    }

    // Destroy the transport
    local ciMap = GetMap();
    ciMap.RemovePawn( this );
  }


  function DetermineState()
  {
    // State_PropertyID drives client animation
    if( Direction.West == m_eHeading )
    {
      // Replicates to clients
      SetProperty( State_PropertyID, ShipStateType.FacingWest );
    }
    else if( Direction.North == m_eHeading )
    {
      // Replicates to clients
      SetProperty( State_PropertyID, ShipStateType.FacingNorth );
    }
    else if( Direction.East == m_eHeading )
    {
      // Replicates to clients
      SetProperty( State_PropertyID, ShipStateType.FacingEast );
    }
    else if( Direction.South == m_eHeading )
    {
      // Replicates to clients
      SetProperty( State_PropertyID, ShipStateType.FacingSouth );
    }
  }


  function Exit( i_ciPlayer, i_ciPos, i_eMoveType )
  {
    if( null == i_ciPlayer )
    {
      return false;
    }

    local bExited = false;
    local ciMap = GetMap();
    local ciPosData = ciMap.GetPositionData( i_ciPos );
    local ciTile = ::rumGetTileAsset( ciPosData.GetTileID() );
    if( !ciTile.IsCollision( i_eMoveType ) || i_ciPlayer.IsDead() )
    {
      SetVisibility( true );

      RemoveFromPassengerList( i_ciPlayer );
      if( m_uiPassengerTable.len() == 0 )
      {
        RemoveProperty( Transport_Commander_ID_PropertyID );
      }

      // Update the client's transport/player link
      local ciBroadcast = ::rumCreate( Player_Xit_BroadcastID, GetID(), i_ciPlayer.GetID() );
      i_ciPlayer.SendBroadcast( ciBroadcast );

      i_ciPlayer.RemoveVersionedProperty( g_eTransportIDPropertyVersionArray );
      i_ciPlayer.RemoveVersionedProperty( g_eTransportBoardedWidgetPropertyVersionArray );

      if( i_ciPlayer.IsDead() )
      {
        i_ciPlayer.SetMoveType( MoveType.Incorporeal );
      }
      else
      {
        i_ciPlayer.SetMoveType( i_ciPlayer.s_eDefaultMoveType );
      }

      ciMap.MovePawn( i_ciPlayer, i_ciPos,
                      rumIgnoreTileCollisionMoveFlag | rumIgnorePawnCollisionMoveFlag | rumIgnoreDistanceMoveFlag );

      if( s_bIdleCleanup && m_uiPassengerTable.len() == 0 )
      {
        ScheduleCleanup();
      }

      bExited = true;
    }

    return bExited;
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


  function GetHitpoints()
  {
    return GetProperty( Hitpoints_PropertyID, 1 );
  }


  function GetNewCommander()
  {
    RemoveProperty( Transport_Commander_ID_PropertyID );
  }


  function GetNumPassengers()
  {
    return m_uiPassengerTable.len();
  }


  function GetPassengers()
  {
    return m_uiPassengerTable;
  }


  function GetType()
  {
    return GetProperty( Transport_Type_PropertyID, TransportType.None );
  }


  function GetVersionedProperty( i_ePropertyArray, i_vDefault = 0 )
  {
    local eVersion = GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
    local ePropertyID = i_ePropertyArray[eVersion];
    return ePropertyID != null ? GetProperty( ePropertyID, i_vDefault ) : i_vDefault;
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


  function HandleLogout( i_ciPlayer )
  {
    if( null == i_ciPlayer )
    {
      return false;
    }

    SetVisibility( true );

    RemoveFromPassengerList( i_ciPlayer );
    if( m_uiPassengerTable.len() == 0 )
    {
      RemoveProperty( Transport_Commander_ID_PropertyID );
    }

    if( s_bIdleCleanup && m_uiPassengerTable.len() == 0 )
    {
      ScheduleCleanup();
    }
  }


  function IsCommander( i_ciPlayer )
  {
    return i_ciPlayer != null ? ( GetCommanderID() == i_ciPlayer.GetID() ) : false;
  }


  function IsDead()
  {
    return GetHitpoints() < 1;
  }


  function MovePassengers( i_ciPos )
  {
    local bPersonal = GetProperty( Transport_Personal_PropertyID, false );

    local ciMap = GetMap();
    foreach( uiPlayerID in m_uiPassengerTable )
    {
      local ciPassenger = uiPlayerID != rumInvalidGameID ? ::rumFetchPawn( uiPlayerID ) : null;
      if( ciPassenger != null )
      {
        ciMap.MovePawn( ciPassenger, i_ciPos,
                        rumIgnoreTileCollisionMoveFlag | rumIgnorePawnCollisionMoveFlag | rumIgnoreDistanceMoveFlag );

        if( !bPersonal )
        {
          ciPassenger.SetVersionedProperty( g_eTransportPosXPropertyVersionArray, i_ciPos.x );
          ciPassenger.SetVersionedProperty( g_eTransportPosYPropertyVersionArray, i_ciPos.y );
        }
      }
    }
  }


  function OnCreated()
  {
    if( GetProperty( Widget_Destructible_PropertyID, false ) )
    {
      local iHitpoints = GetVersionedProperty( g_eHitpointsPropertyVersionArray, 200 );
      SetProperty( Max_Hitpoints_PropertyID, iHitpoints );
      SetProperty( Hitpoints_PropertyID, iHitpoints );
    }
  }


  function Portal( i_eDestMapID, i_ciDestPos )
  {
    local bSuccess = false;

    local ciMap = GetMap();

    local ciCommander = GetCommander();
    if( ciCommander != null && ( ciCommander instanceof Player ) )
    {
      // Transfer the transport ahead of all players
      local ciDestMap = GetOrCreateMap( ciCommander, i_eDestMapID );
      if( ciDestMap != null )
      {
        bSuccess = ciMap.TransferPawn( this, ciDestMap, i_ciDestPos );
        if( bSuccess )
        {
          local bPersonal = GetProperty( Transport_Personal_PropertyID, false );

          local uiCommanderID = ciCommander.GetID();
          foreach( uiPlayerID in m_uiPassengerTable )
          {
            if( uiCommanderID != uiPlayerID )
            {
              local ciPlayer = ::rumFetchPawn( uiPlayerID );
              if( ciMap.TransferPawn( ciPlayer, ciDestMap, i_ciDestPos ) )
              {
                // Update the client passenger manifest
                local ciBroadcast = ::rumCreate( Player_Board_BroadcastID, GetID() );
                ciPlayer.SendBroadcast( ciBroadcast );
              }
            }
          }

          // The commander is always transferred last
          if( ciMap.TransferPawn( ciCommander, ciDestMap, i_ciDestPos ) )
          {
            // Update the client passenger manifest
            local ciBroadcast = ::rumCreate( Player_Board_BroadcastID, GetID() );
            ciCommander.SendBroadcast( ciBroadcast );
          }
        }
      }
    }

    return bSuccess;
  }


  function RemoveFromPassengerList( i_ciPlayer )
  {
    local uiPlayerID = i_ciPlayer.GetID();
    if( uiPlayerID in m_uiPassengerTable )
    {
      delete m_uiPassengerTable[uiPlayerID];
      return true;
    }

    return false;
  }


  function RemovePlayerTransportSettings( i_ciPlayer )
  {
    if( i_ciPlayer != null && ( i_ciPlayer instanceof Player ) )
    {
      i_ciPlayer.RemoveVersionedProperty( g_eTransportCodePropertyVersionArray );
      i_ciPlayer.RemoveVersionedProperty( g_eTransportMapPropertyVersionArray );
      i_ciPlayer.RemoveVersionedProperty( g_eTransportPosXPropertyVersionArray );
      i_ciPlayer.RemoveVersionedProperty( g_eTransportPosYPropertyVersionArray );
      i_ciPlayer.RemoveVersionedProperty( g_eTransportWidgetPropertyVersionArray );
    }
  }


  function Repair( i_eMessageID )
  {
    local iHitpoints = GetHitpoints();
    local iMaxHitpoints = GetProperty( Max_Hitpoints_PropertyID, 1 );
    if( iHitpoints < iMaxHitpoints )
    {
      SetProperty( Hitpoints_PropertyID, iMaxHitpoints );

      local ciPlayer = GetCommander();
      if( ciPlayer != null )
      {
        ciPlayer.ActionInfo( i_eMessageID );
      }
    }
  }


  function ScheduleCleanup()
  {
    ::rumSchedule( this, Cleanup, s_fIdleInterval, ++m_iIdleStamp );
  }


  function SetPlayerTransportSettings( i_ciPlayer )
  {
    if( i_ciPlayer != null && ( i_ciPlayer instanceof Player ) )
    {
      if( !GetProperty( Transport_Personal_PropertyID, false ) )
      {
        local ciMap = GetMap();
        local ciPosition = GetPosition();

        i_ciPlayer.SetVersionedProperty( g_eTransportCodePropertyVersionArray, m_uiTransportCode );
        i_ciPlayer.SetVersionedProperty( g_eTransportMapPropertyVersionArray, ciMap.GetAssetID() );
        i_ciPlayer.SetVersionedProperty( g_eTransportPosXPropertyVersionArray, ciPosition.x );
        i_ciPlayer.SetVersionedProperty( g_eTransportPosYPropertyVersionArray, ciPosition.y );
        i_ciPlayer.SetVersionedProperty( g_eTransportWidgetPropertyVersionArray, GetAssetID() );
      }
    }
  }


  function TryMove( i_ciPlayer, i_ciPos, i_eDir )
  {
    local eResult = rumFailMoveResultType;

    if( GetMoveType() == MoveType.Stationary )
    {
      return eResult;
    }

    // Make sure that the player is the transport commander
    if( IsCommander( i_ciPlayer ) )
    {
      local bUsesHeading = GetProperty( Transport_Uses_Heading_PropertyID, false );
      if( bUsesHeading && m_eHeading != i_eDir )
      {
        ChangeHeading( i_ciPlayer, i_eDir );
        eResult = rumSuccessMoveResultType;
      }
      else
      {
        local ciMap = GetMap();
        eResult = ciMap.MovePawn( this, i_ciPos );
        if( rumSuccessMoveResultType == eResult )
        {
          MovePassengers( i_ciPos );
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


class Passenger_Transport_Widget extends Transport_Widget
{
  static s_iMaxCapacity = 8;


  function Board( i_ciPlayer, i_bForce = false )
  {
    local bBoarded = base.Board( i_ciPlayer, i_bForce );
    if( bBoarded )
    {
      SetPlayerTransportSettings( i_ciPlayer );
    }

    return bBoarded;
  }


  function Exit( i_ciPlayer, i_ciPos, i_eMoveType )
  {
    local bExited = base.Exit( i_ciPlayer, i_ciPos, i_eMoveType );
    if( bExited )
    {
      SetPlayerTransportSettings( i_ciPlayer );
    }

    return bExited;
  }


  function GetNewCommander()
  {
    local bNewCommander = false;

    // Find a new transport commander
    if( m_uiPassengerTable.len() > 0 )
    {
      foreach( i, uiPlayerID in m_uiPassengerTable )
      {
        SetProperty( Transport_Commander_ID_PropertyID, uiPlayerID );
        bNewCommander = true;
        break;
      }
    }

    if( !bNewCommander )
    {
      RemoveProperty( Transport_Commander_ID_PropertyID );
    }
  }


  function RemoveFromPassengerList( i_ciPlayer )
  {
    local bCommander = IsCommander( i_ciPlayer );

    if( base.RemoveFromPassengerList( i_ciPlayer ) )
    {
      i_ciPlayer.SetVersionedProperty( g_eTransportCodePropertyVersionArray, m_uiTransportCode );

      if( bCommander )
      {
        GetNewCommander();
      }

      return true;
    }

    return false;
  }
}


class Ship_Transport_Widget extends Passenger_Transport_Widget
{
  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Sails );

    SetProperty( State_PropertyID, ShipStateType.FacingWest );
  }


  function Board( i_ciPlayer, i_bForce = false )
  {
    local bBoarded = base.Board( i_ciPlayer, i_bForce );
    if( bBoarded && !i_bForce )
    {
      i_ciPlayer.ActionSuccess( msg_board_ship_client_StringID );
    }

    return bBoarded;
  }


  function Exit( i_ciPlayer, i_ciPos, i_eMoveType )
  {
    local bExited = base.Exit( i_ciPlayer, i_ciPos, i_eMoveType );
    if( !bExited )
    {
      // Player attempting to exit on something other than land
      local ciMap = GetMap();
      local ciPosData = ciMap.GetPositionData( i_ciPos );
      local eTileID = ciPosData.GetTileID();
      local ciTile = ::rumGetTileAsset( eTileID );
      if( IsWaterTile( ciTile ) )
      {
        bExited = base.Exit( i_ciPlayer, i_ciPos, MoveType.Aquatic );
        if( bExited )
        {
          local eTransport = ( U1_Water_TileID == eTileID ) ? U1_Raft_WidgetID : U4_Skiff_WidgetID;

          local ciTransport = ::rumCreate( eTransport );
          if( ciTransport )
          {
            if( ciMap.AddPawn( ciTransport, i_ciPos ) )
            {
              local ciBroadcast = ::rumCreate( Player_Xit_BroadcastID, GetID(), i_ciPlayer.GetID() );
              i_ciPlayer.SendBroadcast( ciBroadcast );

              ciTransport.Board( i_ciPlayer );
            }
            else
            {
              // Put the player back on the ship
              Board( i_ciPlayer );
            }
          }
          else
          {
            // Put the player back on the ship
            Board( i_ciPlayer );
          }
        }
      }
    }

    return bExited;
  }
}
