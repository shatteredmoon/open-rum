// Received from client
class Player_Attack_Broadcast extends rumBroadcast
{
  var1 = 0; // Attack type
  var2 = 0; // Direction or Target ID


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local bFoundTarget = false;
    local eAttackType = var1;

    if( AttackType.Directional == eAttackType )
    {
      local eDir = var2;
      local ciPos = i_ciPlayer.GetPosition() + GetDirectionVector( eDir );
      local ciMap = i_ciPlayer.GetMap();
      local eWeaponType = i_ciPlayer.GetWeaponType();

      bFoundTarget = Attack( i_ciPlayer, ciMap, ciPos, eWeaponType );
      if( !bFoundTarget && ( eWeaponType == U4_Halberd_Weapon_InventoryID ) )
      {
        // Halberds get one additional attempt in the same direction
        ciPos = ciPos + GetDirectionVector( eDir );
        bFoundTarget = Attack( i_ciPlayer, ciMap, ciPos, eWeaponType );
      }
    }
    else if( AttackType.Targeted == eAttackType )
    {
      local ciTarget = ::rumFetchPawn( var2 );
      if( ciTarget != null && ciTarget != i_ciPlayer && ciTarget.IsVisible() )
      {
        bFoundTarget = true;
        i_ciPlayer.Attack( ciTarget, i_ciPlayer.GetWeaponType() );
      }
    }
    else
    {
      // Unknown attack type
      i_ciPlayer.IncrementHackAttempts();
    }

    if( !bFoundTarget )
    {
      i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
    }

    local fDelay = i_ciPlayer.GetActionDelay( ActionDelay.Short );
    ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
  }


  function Attack( i_ciPlayer, i_ciMap, i_ciPos, i_eWeaponType )
  {
    local bFoundTarget = false;
    local ciPosData = i_ciMap.GetPositionData( i_ciPos );

    local ciParty = GetParty();

    // Find a creature target
    local ciCreature;
    while( ciCreature = ciPosData.GetNext( rumCreaturePawnType ) )
    {
      if( ciCreature.IsVisible() && i_ciPlayer != ciCreature && !ciCreature.IsDead() )
      {
        // If the player is in a party, don't allow attacks on party members
        if( ciParty != null && ( ciCreature instanceof Player ) && ciParty.HasMember( ciCreature.GetID() ) )
        {
          continue;
        }

        bFoundTarget = true;
        i_ciPlayer.Attack( ciCreature, i_eWeaponType );
        ciPosData.Stop();
      }
    }

    if( !bFoundTarget )
    {
      // Is the intended target a widget?
      ciPosData.Reset();

      local ciWidget;
      while( ciWidget = ciPosData.GetNext( rumWidgetPawnType ) )
      {
        if( ciWidget.IsVisible() && ciWidget.GetProperty( Widget_Destructible_PropertyID, false ) )
        {
          bFoundTarget = true;
          i_ciPlayer.Attack( ciWidget, i_eWeaponType );
          ciPosData.Stop();
        }
      }
    }

    return bFoundTarget;
  }
}
