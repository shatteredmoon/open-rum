// Received from client when player bribes a guard
class Player_Bribe_Broadcast extends rumBroadcast
{
  var = 0; // Direction

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local eDir = var;

    if( !( i_ciPlayer instanceof Player ) )
    {
      return
    }

    // Does player have the gold?
    local iPlayerGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
    if( iPlayerGold >= U3_NPC.s_iBribeCost )
    {
      local ciPos = i_ciPlayer.GetPosition() + GetDirectionVector( eDir );
      local ciMap = i_ciPlayer.GetMap();

      local bFound = false;
      local ciGuard;
      local ciPosData = ciMap.GetPositionData( ciPos );
      while( ciGuard = ciPosData.GetNext( rumCreaturePawnType, U3_Guard_CreatureID ) )
      {
        if( ciGuard.IsVisible() )
        {
          bFound = true;

          if( !ciGuard.IsIncapacitated() )
          {
            i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -U3_NPC.s_iBribeCost );
            ciGuard.Bribe();
            i_ciPlayer.ActionSuccess( msg_bribe_client_StringID );
          }
          else
          {
            i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
          }
        }
      }

      if( !bFound )
      {
        i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
      }
    }
    else
    {
      // Not enough gold
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
    }

    ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, ActionDelay.Short );
  }
}
