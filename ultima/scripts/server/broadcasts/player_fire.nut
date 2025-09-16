// Received from client
class Player_Fire_Broadcast extends rumBroadcast
{
  var1 = 0; // Attack type
  var2 = 0; // Direction or TargetID

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local bAttackFailed = false;
    local bFoundTarget = false;
    local eAttackType = var1;
    local eDelayType = ActionDelay.Short;

    local ciTransport = i_ciPlayer.GetTransport();
    if( ciTransport != null )
    {
      if( AttackType.Directional == eAttackType )
      {
        local bBroadsidesOnly = false;
        local eDir = var2;

        local ciWeapon = ciTransport.GetWeapon();
        if( ciWeapon != null )
        {
          bBroadsidesOnly = ciWeapon.GetProperty( Weapon_Broadsides_Only_PropertyID, false );
        }

        if( bBroadsidesOnly )
        {
          local eHeadingFlipped = ( ciTransport.m_eHeading + 4 ) % 8;
          if( ( eDir == ciTransport.m_eHeading ) || ( eDir == eHeadingFlipped ) )
          {
            // Can't fire directly fore or aft
            i_ciPlayer.ActionFailed( msg_broadsides_client_StringID );
            bAttackFailed = true;
          }
        }

        if( !bAttackFailed )
        {
          local bFoundTarget = false;
          local ciPos = i_ciPlayer.GetPosition() + GetDirectionVector( eDir );

          local ciMap = i_ciPlayer.GetMap();
          local ciPosData = ciMap.GetPositionData( ciPos );
          local ciPawn;
          while( ciPawn = ciPosData.GetNextObject() )
          {
            if( ciPawn.IsVisible() && !ciPawn.IsDead() )
            {
              i_ciPlayer.Attack( ciPawn, ciTransport );
              bFoundTarget = true;
              ciPosData.Stop();
            }
          }
        }
      }
      else if( AttackType.Targeted == eAttackType )
      {
        local ciTarget = ::rumFetchPawn( var2 );
        if( ciTarget )
        {
          if( ciTarget.IsVisible() && !ciTarget.IsDead() )
          {
            bFoundTarget = true;

            local bBroadsidesOnly = false;
            local ciWeapon = ciTransport.GetWeapon();
            if( ciWeapon != null )
            {
              bBroadsidesOnly = ciWeapon.GetProperty( Weapon_Broadsides_Only_PropertyID, false );
            }

            if( bBroadsidesOnly )
            {
              local ciPos = ciTransport.GetPosition();
              local ciTargetPos = ciTarget.GetPosition();
              local ciVector = rumVector( ciTargetPos.x - ciPos.x, ciTargetPos.y - ciPos.y );
              ciVector.Normalize();

              local eDir = GetDirectionFromVector( ciVector );
              local eHeadingFlipped = ( ciTransport.m_eHeading + 4 ) % 8;

              if( ( eDir == ciTransport.m_eHeading ) || ( eDir == eHeadingFlipped ) )
              {
                // Can't fire directly fore or aft
                i_ciPlayer.ActionFailed( msg_broadsides_client_StringID );
                bAttackFailed = true;
              }
            }

            if( !bAttackFailed )
            {
              i_ciPlayer.Attack( ciTarget, ciTransport );
            }
          }
        }
      }
      else
      {
        // Unknown attack type
        i_ciPlayer.IncrementHackAttempts();
      }

      if( !bAttackFailed )
      {
        if( !bFoundTarget )
        {
          i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
        }
        else
        {
          eDelayType = ActionDelay.Long;
        }
      }
    }

    local fDelay = i_ciPlayer.GetActionDelay( eDelayType );
    ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
  }
}
