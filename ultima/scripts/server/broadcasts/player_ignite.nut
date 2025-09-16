// Received from client when player ignites a torch
class Player_Ignite_Broadcast extends rumBroadcast
{
  var = 0; // Ignite or extinguish

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    // This will be true for ignite, or false for extinguish
    local bIgnite = var;

    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local eDelay = ActionDelay.Short;

    // Make sure player is on a map that requires lighting
    local ciMap = i_ciPlayer.GetMap();
    if( MapRequiresLight( ciMap ) )
    {
      if( bIgnite )
      {
        local ePropertyID = U4_Torches_PropertyID;
        local eVersion = i_ciPlayer.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
        switch( eVersion )
        {
          case GameType.Ultima1: ePropertyID = U1_Torches_PropertyID; break;
          case GameType.Ultima2: ePropertyID = U2_Torches_PropertyID; break;
          case GameType.Ultima3: ePropertyID = U3_Torches_PropertyID; break;
        }

        // Make sure player owns a torch
        local iTorches = i_ciPlayer.GetProperty( ePropertyID, 0 );
        if( iTorches > 0 )
        {
          // Make sure the player has a free hand (no 2-handed weapon equipped)
          local ciWeapon = i_ciPlayer.GetEquippedWeapon();
          local iNumHands = ciWeapon != null ? ciWeapon.GetProperty( Inventory_Weapon_Num_Hands_PropertyID, 1 ) : 1;
          if( iNumHands <= 1 )
          {
            i_ciPlayer.SetProperty( ePropertyID, iTorches - 1 );

            // Extinguish any previous lights
            i_ciPlayer.ExtinguishLight();

            // Create the torch effect
            local ciEffect = Light_Effect( false );
            ciEffect.m_uiTargetID = i_ciPlayer.GetID();

            // Put the effect in the target's effect table
            i_ciPlayer.m_ciEffectsTable[ciEffect] <- ciEffect;

            // The effect expires after the specified duration
            ::rumSchedule( ciEffect, ciEffect.Expire, ciEffect.s_fDuration );

            i_ciPlayer.SetLightRange( ciEffect.s_iRange );

            eDelay = ActionDelay.Long;
          }
          else
          {
            i_ciPlayer.ActionFailed( msg_hands_full_client_StringID );
          }
        }
        else
        {
          i_ciPlayer.ActionFailed( msg_none_owned_client_StringID );
        }
      }
      else
      {
        i_ciPlayer.ExtinguishLight();
        eDelay = ActionDelay.Medium;
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_dungeons_only_client_StringID );
    }

    local fDelay = i_ciPlayer.GetActionDelay( eDelay );
    ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
  }
}
