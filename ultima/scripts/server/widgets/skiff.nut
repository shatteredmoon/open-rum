class U4_Skiff_Widget extends Transport_Widget
{
  static s_fIdleInterval = 0.0;


  function Board( i_ciPlayer, i_bForce = false )
  {
    local bBoarded = base.Board( i_ciPlayer, i_bForce );
    if( bBoarded && !i_bForce )
    {
      i_ciPlayer.ActionSuccess( msg_board_skiff_client_StringID );
    }
  }


  function CheckDeepWater( i_ciPlayer )
  {
    // Player takes damage in deep water
    local ciMap = i_ciPlayer.GetMap();
    local ciPosData = ciMap.GetPositionData( GetPosition() );
    local eTileID = ciPosData.GetTileID();
    if( U4_Water_Deep_TileID == eTileID )
    {
      i_ciPlayer.Damage( rand() % 10 + 1, null );
    }
  }


  function Destroy()
  {
    // Instant death for all passengers
    foreach( uiPlayerID in m_uiPassengerTable )
    {
      local ciPlayer = ::rumFetchPawn( uiPlayerID );
      if( ciPlayer != null )
      {
        ciPlayer.AffectHitpoints( -ciPlayer.GetMaxHitpoints() );
        ciPlayer.OnDeath( null, false );
      }
    }

    base.Destroy();
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
        eResult = base.TryMove( i_ciPlayer, i_ciPos, i_eDir );
        CheckDeepWater( i_ciPlayer );
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_not_commander_client_StringID );
    }

    return eResult;
  }
}


class U1_Raft_Widget extends Transport_Widget
{
  function Exit( i_ciPlayer, i_ciPos, i_eMoveType )
  {
    if( base.Exit( i_ciPlayer, i_ciPos, i_eMoveType ) )
    {
      // Stamp the exit time on the player
      i_ciPlayer.SetProperty( U1_Raft_Deboard_Time_PropertyID, ::rumGetSecondsSinceEpoch() );

      return true;
    }

    return false;
  }
}
