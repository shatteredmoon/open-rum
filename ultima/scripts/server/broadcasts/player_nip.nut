// Received from client
class Player_Nip_Broadcast extends rumBroadcast
{
  var1 = 0; // Self or other creature
  var2 = 0; // PropertyID
  var3 = 0; // Direction

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local eDelay = ActionDelay.Short;

    if( !( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() ) )
    {
      local iPotions = i_ciPlayer.GetVersionedProperty( g_ePotionsPropertyVersionArray, rumInvalidAssetID );
      if( iPotions > 0 )
      {
        local ciMap = i_ciPlayer.GetMap();
        local ciTarget = null;

        if( var1 )
        {
          // The player is consuming their own potion
          ciTarget = i_ciPlayer;
        }
        else
        {
          // Not self-targeted, so use direction to identify target
          local ciCreature = null;
          local ciPos = i_ciPlayer.GetPosition() + GetDirectionVector( var3 );
          local ciPosData = ciMap.GetPositionData( ciPos );
          while( ciCreature = ciPosData.GetNext( rumCreaturePawnType ) )
          {
            if( ciCreature.IsVisible() )
            {
              ciTarget = ciCreature;
              ciPosData.Stop();
            }
          }

          if( ciTarget != null )
          {
            if( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() )
            {
              i_ciPlayer.ActionFailed( msg_cant_client_StringID );
              ciTarget = null;
            }
          }
        }

        if( ciTarget != null )
        {
          local ePropertyID = var2;
          local eSpellID = rumInvalidAssetID;

          if( U4_Potions_PropertyID == var2 )
          {
            if( ciTarget.IsPoisoned() )
            {
              eSpellID = U4_Cure_Spell_CustomID;
            }
            else
            {
              eSpellID = U4_Heal_Spell_CustomID;
            }
          }
          else if( U3_Potions_PropertyID == var2 )
          {
            if( ciTarget.IsPoisoned() )
            {
              eSpellID = U3_Alcort_Spell_CustomID;
            }
            else
            {
              eSpellID = U3_Sanctu_Spell_CustomID;
            }
          }
          else if( U2_Potions_PropertyID == var2 )
          {
            eSpellID = U2_Heal_Spell_CustomID;
          }
          else if( U1_Potions_PropertyID == var2 )
          {
            eSpellID = U1_Heal_Spell_CustomID;
          }

          if( eSpellID != rumInvalidAssetID )
          {
            local ciSpell = ::rumGetAsset( eSpellID );
            Cast( ciSpell, i_ciPlayer, ciTarget, null );

            // Consume the potion
            i_ciPlayer.SetProperty( ePropertyID, iPotions - 1 );

            eDelay = ActionDelay.Medium;
          }
          else
          {
            i_ciPlayer.ActionFailed( msg_cant_client_StringID );
          }
        }
        else
        {
          i_ciPlayer.ActionFailed( msg_nothing_here_client_StringID );
        }
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_none_owned_client_StringID );
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
    }

    local fDelay = i_ciPlayer.GetActionDelay( eDelay );
    ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
  }
}
