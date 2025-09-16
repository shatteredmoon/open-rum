// Received from client when player uses an item
class Player_Use_Broadcast extends rumBroadcast
{
  var1 = 0; // Property ID
  var2 = 0; // Direction


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 1 )
    {
      var2 = vargv[1];
    }

    if( vargv.len() > 0 )
    {
      var1 = vargv[0];
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local iPropertyID = var1;

    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local ciPos = i_ciPlayer.GetPosition();
    local ciMap = i_ciPlayer.GetMap();

    local bSuccess = false;
    local eMsg = msg_no_effect_client_StringID;

    ///////////////////////
    // Transports/Mounts //
    ///////////////////////
    if( ( U4_Horse_PropertyID == iPropertyID )  ||
        ( U3_Horse_PropertyID == iPropertyID )  ||
        ( U2_Horse_PropertyID == iPropertyID )  ||
        ( U1_Horse_PropertyID == iPropertyID )  ||
        ( U1_Aircar_PropertyID == iPropertyID ) ||
        ( U1_Cart_PropertyID == iPropertyID )   ||
        ( U1_Raft_PropertyID == iPropertyID ) )
    {
      eMsg = msg_cant_client_StringID;

      if( i_ciPlayer.GetTransportID() == rumInvalidGameID )
      {
        local iLastUsageTime = 0;
        local eWidgetID = rumInvalidAssetID;

        switch( iPropertyID )
        {
          case U4_Horse_PropertyID:
            eWidgetID = U4_Horse_WidgetID;
          case U3_Horse_PropertyID:
            eWidgetID = U3_Horse_WidgetID;
          case U2_Horse_PropertyID:
            eWidgetID = U2_Horse_WidgetID;
          case U1_Horse_PropertyID:
            eWidgetID = U1_Horse_WidgetID;
            iLastUsageTime = i_ciPlayer.GetVersionedProperty( g_eHorseDismountTimePropertyVersionArray, 0 );
            break;

          case U1_Aircar_PropertyID:
            eWidgetID = U1_Aircar_WidgetID;
            iLastUsageTime = i_ciPlayer.GetProperty( U1_Aircar_Deboard_Time_PropertyID, 0 );
            break;

          case U1_Cart_PropertyID:
            eWidgetID = U1_Cart_WidgetID;
            iLastUsageTime = i_ciPlayer.GetProperty( U1_Cart_Deboard_Time_PropertyID, 0 );
            break;

          case U1_Raft_PropertyID:
            eWidgetID = U1_Raft_WidgetID;
            iLastUsageTime = i_ciPlayer.GetProperty( U1_Raft_Deboard_Time_PropertyID, 0 );
            break;
        }

        if( ::rumGetSecondsSinceEpoch() - iLastUsageTime > Transport_Widget.s_fIdleInterval )
        {
          local eMapAssetID = ciMap.GetAssetID();

          // Is the map compatible with usable transports?
          local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
          if( ( MapType.Dungeon == eMapType )      ||
              ( MapType.Abyss == eMapType )        ||
              ( MapType.Altar == eMapType )        ||
              ( MapType.Cave == eMapType )         ||
              ( MapType.Codex == eMapType )        ||
              ( MapType.Shrine == eMapType )       ||
              ( MapType.Space == eMapType )        ||
              ( MapType.SpaceStation == eMapType ) ||
              ( U4_Castle_British_2_MapID == eMapAssetID )                  ||
              ( U3_Castle_Fire_MapID == eMapAssetID )                       ||
              ( U3_Castle_Fire_2_MapID == eMapAssetID )                     ||
              ( U2_Earth_Legends_Castle_Shadow_Guard_MapID == eMapAssetID ) ||
              ( U1_Time_Machine_MapID == eMapAssetID )                      ||
              ( U1_Lair_Mondain_MapID == eMapAssetID ) )
          {
            eMsg = msg_not_here_client_StringID;
          }
          else if( i_ciPlayer.IsJailed() )
          {
            eMsg = msg_not_here_client_StringID;
          }
          else if( rumInvalidAssetID != eWidgetID )
          {
            local ciTransport = CreateTransport( eWidgetID, 0, ciMap, i_ciPlayer.GetPosition() );
            if( ciTransport != null )
            {
              bSuccess = ciTransport.Board( i_ciPlayer );
            }
          }
        }
      }
    }

    /////////////////////
    // Bell of Courage //
    /////////////////////
    else if( U4_Item_Bell_PropertyID == iPropertyID )
    {
      if( ciMap.GetAssetID() == U4_Britannia_MapID )
      {
        local eState = i_ciPlayer.GetProperty( U4_Item_Bell_PropertyID, U4_QuestItemState.NotFound );
        if( U4_QuestItemState.Found == eState )
        {
          // Cape of lost hope
          if( ciPos.x == 96 && ciPos.y == 215 )
          {
            i_ciPlayer.SetProperty( U4_Item_Bell_PropertyID, U4_QuestItemState.Imbued );
            i_ciPlayer.ActionInfo( msg_bell_bestowed_client_StringID );
            bSuccess = true;
          }
        }
        else if( U4_QuestItemState.Imbued == eState )
        {
          // Abyss entrance
          if( ciPos.x == 233 && ciPos.y == 233 )
          {
            i_ciPlayer.SetProperty( U4_Item_Bell_PropertyID, U4_QuestItemState.Abyss_Used );
            i_ciPlayer.ActionInfo( msg_bell_abyss_client_StringID );
            bSuccess = true;
          }
        }
      }
    }

    ///////////////////
    // Book of Truth //
    ///////////////////
    else if( U4_Item_Book_PropertyID == iPropertyID )
    {
      if( ciMap.GetAssetID() == U4_Britannia_MapID )
      {
        local eState = i_ciPlayer.GetProperty( U4_Item_Book_PropertyID, U4_QuestItemState.NotFound );
        if( U4_QuestItemState.Found == eState )
        {
          // Well of Knowledge
          if( ciPos.x == 176 && ciPos.y == 208 )
          {
            i_ciPlayer.SetProperty( U4_Item_Book_PropertyID, U4_QuestItemState.Imbued );
            i_ciPlayer.ActionInfo( msg_book_bestowed_client_StringID );
            bSuccess = true;
          }
        }
        else if( U4_QuestItemState.Imbued == eState )
        {
          // Abyss entrance
          if( ciPos.x == 233 && ciPos.y == 233 )
          {
            // Player has to have used the imbued bell at the mouth of the abyss
            if( i_ciPlayer.GetProperty( U4_Item_Bell_PropertyID, U4_QuestItemState.NotFound ) ==
                U4_QuestItemState.Abyss_Used )
            {
              i_ciPlayer.SetProperty( U4_Item_Book_PropertyID, U4_QuestItemState.Abyss_Used );
              i_ciPlayer.ActionInfo( msg_book_abyss_client_StringID );
              bSuccess = true;
            }
          }
        }
      }
    }

    ////////////////////
    // Candle of Love //
    ////////////////////
    else if( U4_Item_Candle_PropertyID == iPropertyID )
    {
      local eState = i_ciPlayer.GetProperty( U4_Item_Candle_PropertyID, U4_QuestItemState.NotFound );
      if( ciMap.GetAssetID() == U4_Keep_Empath_Abbey_MapID )
      {
        if( U4_QuestItemState.Found == eState )
        {
          // The Oak Grove
          if( ciPos.x == 22 && ciPos.y == 4 )
          {
            i_ciPlayer.SetProperty( U4_Item_Candle_PropertyID, U4_QuestItemState.Imbued );
            i_ciPlayer.ActionInfo( msg_candle_bestowed_client_StringID );
            bSuccess = true;
          }
        }
      }
      else if( ciMap.GetAssetID() == U4_Britannia_MapID )
      {
        if( U4_QuestItemState.Imbued == eState )
        {
          // Abyss entrance
          if( ciPos.x == 233 && ciPos.y == 233 )
          {
            // Player has to have used the imbued bell and imbued book at the mouth of the abyss
            if( i_ciPlayer.GetProperty( U4_Item_Bell_PropertyID, U4_QuestItemState.NotFound ) ==
                U4_QuestItemState.Abyss_Used &&
                i_ciPlayer.GetProperty( U4_Item_Book_PropertyID, U4_QuestItemState.NotFound ) ==
                U4_QuestItemState.Abyss_Used )
            {
              i_ciPlayer.SetProperty( U4_Item_Candle_PropertyID, U4_QuestItemState.Abyss_Used );
              i_ciPlayer.ActionInfo( msg_candle_abyss_client_StringID );
              SendClientEffect( i_ciPlayer, ClientEffectType.ScreenShake );
              bSuccess = true;
            }
          }
        }
      }
    }

    /////////////////
    // Silver Horn //
    /////////////////
    else if( U4_Item_Silver_Horn_PropertyID == iPropertyID )
    {
      local eState = i_ciPlayer.GetProperty( U4_Item_Silver_Horn_PropertyID, U4_QuestItemState.NotFound );
      if( U4_QuestItemState.Found == eState )
      {
        if( ciMap.GetAssetID() == U4_Towne_Magincia_MapID )
        {
          // Make sure the player is near the towne center
          if( ciPos.x > 11 && ciPos.x < 21 && ciPos.y > 11 && ciPos.y < 21 )
          {
            i_ciPlayer.SetProperty( U4_Item_Silver_Horn_PropertyID, U4_QuestItemState.Imbued );
            i_ciPlayer.ActionInfo( msg_horn_bestowed_client_StringID );
            SendClientEffect( i_ciPlayer, ClientEffectType.ScreenShake );
            bSuccess = true;
          }
        }
      }
      else if( U4_QuestItemState.Imbued == eState )
      {
        if( ( ciMap.GetAssetID() == U4_Shrine_Humility_MapID ) ||
            ( ( ciMap.GetAssetID() == U4_Britannia_MapID ) && ( ( ciPos.x == 231 ) && ( ciPos.y == 216 ) ) ) )
        {
          local bImmune = i_ciPlayer.GetProperty( U4_Shrine_Daemon_Immunity_PropertyID, false );
          if( !bImmune )
          {
            SendClientEffect( i_ciPlayer, ClientEffectType.ScreenShake );

            i_ciPlayer.SetProperty( U4_Shrine_Daemon_Immunity_PropertyID, true );

            // Create the immunity effect
            local ciEffect = Daemon_Immunity_Effect();
            ciEffect.m_uiTargetID = i_ciPlayer.GetID();

            // Put the effect in the target's effect table
            i_ciPlayer.m_ciEffectsTable[ciEffect] <- ciEffect;

            // Schedule effect expiration
            ::rumSchedule( ciEffect, ciEffect.Expire, ciEffect.s_fDuration );

            bSuccess = true;
          }
        }
        else if( ( ciMap.GetAssetID() == U4_Britannia_MapID ) && i_ciPlayer.IsFlying() )
        {
          local eDir = var2;
          local ciSpell = ::rumGetCustomAsset( U4_Wind_Change_Spell_CustomID );
          CastWindChange( i_ciPlayer, ciSpell, eDir );

          bSuccess = true;
        }
      }
    }

    ////////////////////
    // Skull Fragment //
    ////////////////////
    else if( U4_Item_Skull_Fragment_PropertyID == iPropertyID )
    {
      local eState = i_ciPlayer.GetProperty( U4_Item_Skull_Fragment_PropertyID, U4_QuestItemState.NotFound );
      if( U4_QuestItemState.Found == eState )
      {
        if( ciMap.GetAssetID() == U4_Britannia_MapID )
        {
          // Abyss entrance
          if( ciPos.x == 233 && ciPos.y == 233 )
          {
            SendClientEffect( i_ciPlayer, ClientEffectType.ScreenShake );

            // Increase all virtues for destroying the skull fragment
            for( local eVirtue = 0; eVirtue < VirtueType.NumVirtues; ++eVirtue )
            {
              i_ciPlayer.AffectVirtue( eVirtue, 10, true, true );
            }

            // Forget the last incremented virtue
            i_ciPlayer.SetProperty( U4_Last_Incremented_Virtue_PropertyID, -1 );

            i_ciPlayer.ActionInfo( msg_skull_abyss_client_StringID );
            i_ciPlayer.SetProperty( U4_Item_Skull_Fragment_PropertyID, U4_QuestItemState.Abyss_Used );

            // Player is using the skull fragment
            local ciSpell = ::rumGetCustomAsset( U4_Tremor_Spell_CustomID );
            CastTremor( i_ciPlayer, ciSpell );

            bSuccess = true;
          }
        }

        if( !bSuccess )
        {
          // Player is using the skull fragment
          local ciSpell = ::rumGetCustomAsset( U4_Tremor_Spell_CustomID );
          CastTremor( i_ciPlayer, ciSpell );

          // Penalties
          for( local eVirtue = 0; eVirtue < VirtueType.NumVirtues; ++eVirtue )
          {
            i_ciPlayer.AffectVirtue( eVirtue, -2, true, true );
          }

          bSuccess = true;
        }
      }
    }

    ////////////////////////
    // Key of Three Parts //
    ////////////////////////
    else if( U4_Item_Three_Part_Key_PropertyID == iPropertyID )
    {
      if( i_ciPlayer.HasThreePartsKey() )
      {
        if( ciMap.GetAssetID() == U4_Dungeon_Codex_Chamber_MapID )
        {
          // Player must be adjacent to the chamber door
          local ciPawnArray = ciMap.GetPawns( ciPos, 1, false );
          foreach( ciPawn in ciPawnArray )
          {
            if( ciPawn instanceof U4_Door_Codex_Widget )
            {
              local ciPawnPos = ciPawn.GetPosition();

              // The player must be below the chamber door for it to have any effect
              if( ciPos.y > ciPawnPos.y )
              {
                local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID,
                                                 U4_AbyssCodexPhaseType.ChamberDoor );
                i_ciPlayer.SendBroadcast( ciBroadcast );

                bSuccess = true;
                break;
              }
            }
          }
        }
      }
    }
    else
    {
      local eWidgetID = i_ciPlayer.GetVersionedProperty( g_eTransportWidgetPropertyVersionArray, rumInvalidAssetID );
      if( eWidgetID != rumInvalidAssetID )
      {
        local eMapID = i_ciPlayer.GetVersionedProperty( g_eTransportMapPropertyVersionArray, rumInvalidAssetID );
        if( eMapID != rumInvalidAssetID && ( ciMap.GetAssetID() == eMapID ) )
        {
          // Create the transport if necessary
          bSuccess = i_ciPlayer.SummonLastTransport() != null;
        }
      }
    }

    local eDelay = ActionDelay.Long;

    if( !bSuccess )
    {
      i_ciPlayer.ActionFailed( eMsg );
      eDelay = ActionDelay.Short;
    }

    local fDelay = i_ciPlayer.GetActionDelay( eDelay );
    ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
  }
}
