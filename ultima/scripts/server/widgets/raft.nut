class U1_Raft_Widget extends Transport_Widget
{
  static s_fIdleInterval = 0.0;


  function Board( i_ciPlayer, i_bForce = false )
  {
    local bBoarded = base.Board( i_ciPlayer, i_bForce );
    if( bBoarded && !i_bForce )
    {
      i_ciPlayer.ActionSuccess( msg_board_raft_client_StringID );
    }

    return bBoarded;
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
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_not_commander_client_StringID );
    }

    return eResult;
  }
}
