// Received from client when player wants to board a transport
// Sent from server when a transport is boarded
class Player_Board_Broadcast extends rumBroadcast
{
  var = 0;

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var = vargv[0]; // Target ID
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local bFound = false;
    local eDelay = ActionDelay.Short;
    local uiTargetID = var;

    if( i_ciPlayer instanceof Player )
    {
      local ciTarget = ::rumFetchPawn( uiTargetID );
      if( ciTarget != null && ciTarget.IsVisible() )
      {
        local ciPos = i_ciPlayer.GetPosition();
        local ciMap = i_ciPlayer.GetMap();

        // Distance check
        if( ciMap.IsPositionWithinTileDistance( ciPos, ciTarget.GetPosition(), 1 ) )
        {
          if( ciTarget instanceof Transport_Widget )
          {
            bFound = true;
            if( ciTarget.Board( i_ciPlayer ) )
            {
              eDelay = ActionDelay.Long;
            }
          }
          else if( ciTarget instanceof Creature )
          {
            // Is the player trying to steal a horse?
            bFound = true;

            if( ciTarget.m_iDialogueID != 0 )
            {
              // Player is trying to steal Smith
              i_ciPlayer.ActionFailed( msg_help_client_StringID );
              ciMap.OffsetPawn( ciTarget, GetRandomDirectionVector() );
            }
            else if( PostureType.Attack != ciTarget.m_eDefaultPosture )
            {
              if( ciTarget.GetProperty( Creature_Is_Horse_PropertyID, false ) )
              {
                ciTarget.OnStolen( i_ciPlayer );
                eDelay = ActionDelay.Long;
              }
            }
          }
        }
      }
    }

    if( bFound )
    {
      local fDelay = i_ciPlayer.GetActionDelay( eDelay );
      ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
    }
  }
}
