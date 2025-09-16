// Received from client when player inserts a card into Exodus
// Sent from server with results
class Player_Insert_Broadcast extends rumBroadcast
{
  var1 = 0; // Card type
  var2 = 0; // Direction


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() >= 1 )
    {
      var1 = vargv[0];

      if( vargv.len() >= 2 )
      {
        var2 = vargv[1];
      }
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local eCardType = var1;

    // Verify player has the specified card
    local iFlags = i_ciPlayer.GetProperty( U3_Cards_PropertyID, 0 );
    if( ::rumBitOn( iFlags, eCardType ) )
    {
      // Verify player is on the correct map
      local ciMap = i_ciPlayer.GetMap();
      if( ciMap.GetAssetID() == U3_Castle_Fire_2_MapID )
      {
        // Verify player is next to an Exodus component
        local eDir = var2;
        local ciPos = i_ciPlayer.GetPosition() + GetDirectionVector( eDir );

        // Is there an Exodus component at this location?
        local ciPosData = ciMap.GetPositionData( ciPos );
        local ciExodus = ciPosData.GetNext( rumWidgetPawnType, U3_Exodus_WidgetID );
        if( ciExodus != null )
        {
          // Is the component functional (not already disabled with a card)?
          local eState = ciExodus.GetProperty( State_PropertyID, 0 );
          if( U3_ExodusComponentState.Functional == eState )
          {
            local eCardOrderArray = null;
            local iComponentID = ciExodus.GetProperty( Widget_Exodus_Component_ID_PropertyID, 0 );
            switch( iComponentID )
            {
              case 0: eCardOrderArray = g_uiU3ExodusOrderIndex0Array; break;
              case 1: eCardOrderArray = g_uiU3ExodusOrderIndex1Array; break;
              case 2: eCardOrderArray = g_uiU3ExodusOrderIndex2Array; break;
              case 3: eCardOrderArray = g_uiU3ExodusOrderIndex3Array; break;
            }

            if( eCardOrderArray != null )
            {
              local iCardIndex = i_ciPlayer.GetProperty( U3_Exodus_Card_Index_PropertyID, 9 );

              // Has the player provided the proper card?
              if( eCardOrderArray[iCardIndex] == eCardType )
              {
                // Disable this component
                SendClientEffect( ciExodus, ClientEffectType.Damage );
                SendClientEffect( ciExodus, ClientEffectType.ScreenShake );

                ciExodus.SetProperty( State_PropertyID, U3_ExodusComponentState.Destroyed );

                local iNumDestroyed = 1;

                // Exodus is fully destroyed if all components are destroyed
                local ciPawnArray = ciMap.GetAllPawns();
                foreach( ciPawn in ciPawnArray )
                {
                  if( ciPawn != ciExodus && ciPawn instanceof U3_Exodus_Widget )
                  {
                    local eDestroyed = ciPawn.GetProperty( State_PropertyID, 0 );
                    if( U3_ExodusComponentState.Destroyed == eDestroyed )
                    {
                      ++iNumDestroyed;
                    }
                  }
                }

                if( 4 == iNumDestroyed )
                {
                  // Signal that Exodus has been destroyed
                  local ciBroadcast = ::rumCreate( Player_Insert_BroadcastID );
                  i_ciPlayer.SendBroadcast( ciBroadcast );

                  // Remember that the player has destroyed Exodus
                  local iFlags = i_ciPlayer.GetProperty( Ultima_Completions_PropertyID, 0 );
                  iFlags = ::rumBitSet( iFlags, UltimaCompletions.DestroyedExodus );
                  i_ciPlayer.SetProperty( Ultima_Completions_PropertyID, iFlags );

                  i_ciPlayer.ActionSuccess( msg_exodus_destroyed_client_StringID );

                  // The castle crumbles...
                  ciMap.DestroyStart();
                }
              }
              else
              {
                // Kill the player outright
                i_ciPlayer.Kill( ciExodus, null, true );
              }
            }
          }
          else
          {
            i_ciPlayer.ActionFailed( msg_no_effect_client_StringID );
          }
        }
        else
        {
          // Player isn't adjacent to an Exodus component
          i_ciPlayer.IncrementHackAttempts();
        }
      }
      else
      {
        // Player isn't even on the Exodus map
        i_ciPlayer.IncrementHackAttempts();
      }
    }
    else
    {
      // Player doesn't have the specified card
      i_ciPlayer.IncrementHackAttempts();
    }

    local fDelay = i_ciPlayer.GetActionDelay( ActionDelay.Long );
    ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
  }
}
