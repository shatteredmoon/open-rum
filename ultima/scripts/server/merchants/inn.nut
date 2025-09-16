function TransactInnStart( i_ciPlayer )
{
  if( InnHasVacancy( i_ciPlayer.GetMap() ) )
  {
    local ciBroadcast = ::rumCreate( Merchant_Inn_BroadcastID, MerchantInnTransaction.Greet );
    ::rumSendPrivate( i_ciPlayer.GetSocket(), ciBroadcast );
  }
  else
  {
    local ciBroadcast = ::rumCreate( Merchant_Inn_BroadcastID, MerchantInnTransaction.NoVacancy );
    ::rumSendPrivate( i_ciPlayer.GetSocket(), ciBroadcast );
  }
}


function HandleInnCheckout( i_ciPlayer, i_iRoomIndex )
{
  HandleInnWakeup( i_ciPlayer, i_iRoomIndex );

  // Heal the player
  i_ciPlayer.AffectHitpoints( rand() % 300 + 500 );

  // If you stay in Skara Brae, you get a visit from Isaac the ghost
  local ciMap = i_ciPlayer.GetMap();
  if( ciMap.GetAssetID() == U4_Towne_Skara_Brae_MapID )
  {
    local ciPawnArray = ciMap.GetAllPawns();
    foreach( ciPawn in ciPawnArray )
    {
      if( ciPawn.GetAssetID() == U4_Ghost_CreatureID )
      {
        // Temporarily show Isaac
        ciPawn.SetVisibility( true );
        ::rumSchedule( ciPawn, ciPawn.Hide, 120.0 );
        break;
      }
    }

    i_ciPlayer.ActionSuccess( msg_ghostly_client_StringID );
  }
  else
  {
    i_ciPlayer.ActionSuccess( msg_morning_client_StringID );
  }
}


function HandleInnInvasion( i_ciPlayer, i_iRoomIndex )
{
  HandleInnWakeup( i_ciPlayer, i_iRoomIndex );

  i_ciPlayer.ActionWarning( u4_merchant_inn_invasion_client_StringID );

  local ciPos = i_ciPlayer.GetPosition();
  local ciMap = i_ciPlayer.GetMap();

  // Create up to four randomly placed rogues
  local iNumRogues = rand() % 4 + 1;
  for( local i = 0; i < iNumRogues; ++i )
  {
    local ciRogue = ::rumCreate( U4_Rogue_CreatureID );
    local bPlaced = false;

    // Try a 3x3 grid around the bed
    for( local x = -3; x <= 3 && !bPlaced; ++x )
    {
      for( local y = -3; y <= 3 && !bPlaced; ++y )
      {
        local ciOffsetPos = ::rumVector( x, y );
        local ciRoguePos = ciPos + ciOffsetPos;
        if( ciMap.TestLOS( ciRoguePos, ciPos, 3 ) )
        {
          // Test placement
          if( ciMap.MovePawn( ciRogue, ciRoguePos,
                              rumIgnoreDistanceMoveFlag | rumTestMoveFlag ) == rumSuccessMoveResultType )
          {
            if( ciMap.AddPawn( ciRogue, ciRoguePos ) )
            {
              bPlaced = true;

              ciRogue.m_bRespawns = false;
              ciRogue.m_eDefaultPosture = PostureType.Attack;
              ciRogue.m_eDefaultNpcType = NPCType.Standard;

              ::rumSchedule( ciRogue, ciRogue.AIDetermine, frandVariance( ciRogue.m_fTimeDecideVariance ) );

              // Store the NPC's original position for leashing, etc.
              ciRogue.m_ciOriginPos = ciRoguePos;
            }
          }
        }
      }
    }
  }
}


function HandleInnWakeup( i_ciPlayer, i_iRoomIndex )
{
  local ciMap = i_ciPlayer.GetMap();
  if( ciMap )
  {
    // Modify the bed state, so that clients are updated
    local ciBed;
    local ciPosData = ciMap.GetPositionData( GetPosition() );
    while( ciBed = ciPosData.GetNext( rumWidgetPawnType, U4_Bed_Head_WidgetID ) )
    {
      ciBed.SetState( 0 );
      ciPosData.Stop();
    }

    // Wake up and show the player again
    i_ciPlayer.SetVisibility( true );
    i_ciPlayer.RemoveProperty( Unconscious_PropertyID );
  }
}


function InnHasVacancy( i_ciMap )
{
  local eRoomArray = null;

  // Determine the data table row
  local eMapArray = ::rumGetDataTableColumn( merchant_inn_DataTableID, 0 );
  local iRow = eMapArray.find( i_ciMap.GetAssetID() );
  if( iRow != null )
  {
    // Fetch the row and slice off the map column
    eRoomArray = ::rumGetDataTableRow( merchant_inn_DataTableID, iRow ).slice( 1 );
  }

  // Inns can have up to 4 rooms
  local iNumBedChoices = ( eRoomArray[0] > 0 ? 1 : 0 ) +
                         ( eRoomArray[3] > 0 ? 1 : 0 ) +
                         ( eRoomArray[6] > 0 ? 1 : 0 ) +
                         ( eRoomArray[9] > 0 ? 1 : 0 );

  return( ( eRoomArray[0] > 0 && IsInnRoomVacant( i_ciMap, 0 ) ) ||
          ( eRoomArray[3] > 0 && IsInnRoomVacant( i_ciMap, 1 ) ) ||
          ( eRoomArray[6] > 0 && IsInnRoomVacant( i_ciMap, 2 ) ) ||
          ( eRoomArray[9] > 0 && IsInnRoomVacant( i_ciMap, 3 ) ) );
}


function IsInnRoomVacant( i_ciMap, i_iRoomIndex )
{
  // Return true if the room hasn't been used during this up-time
  return i_ciMap.GetProperty( Merchant_Inn_Vacancy_0_PropertyID + i_iRoomIndex, true );
}


function SetInnRoomVacant( i_ciMap, i_iRoomIndex, i_bVacant )
{
  i_ciMap.SetProperty( Merchant_Inn_Vacancy_0_PropertyID + i_iRoomIndex, i_bVacant );
}


function TransactInnRecv( i_ciPlayer, i_eTransactionType, i_iRoomIndex )
{
  if( MerchantInnTransaction.RoomPurchase != i_eTransactionType )
  {
    // Why was the message even sent?
    i_ciPlayer.IncrementHackAttempts();
    return;
  }

  local bSuccess = false;
  local ciMap = i_ciPlayer.GetMap();

  // Make absolutely sure the room is still available
  if( IsInnRoomVacant( ciMap, i_iRoomIndex ) )
  {
    local strResult = "u4_merchant_inn_buy_cancel";

    local eRoomArray = null;

    // Determine the data table row
    local eMapArray = ::rumGetDataTableColumn( merchant_inn_DataTableID, 0 );
    local iRow = eMapArray.find( ciMap.GetAssetID() );
    if( iRow != null )
    {
      // Fetch the row and slice off the map column
      eRoomArray = ::rumGetDataTableRow( merchant_inn_DataTableID, iRow ).slice( 1 );
    }

    local iRoomOffset = ( i_iRoomIndex - 1 ) * 3;
    local iPrice = eRoomArray[iRoomOffset];
    if( iPrice > 0 )
    {
      local ciPos = rumPos( eRoomArray[iRoomOffset + 1], eRoomArray[iRoomOffset + 2] );

      // Verify gold
      local iPlayerGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
      if( iPlayerGold < iPrice )
      {
        // The Player's inability to pay should've been caught on the client
        strResult = "u4_merchant_inn_cant_pay";
        i_ciPlayer.IncrementHackAttempts();
      }
      else
      {
        // Charge the player
        i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iPrice );

        // Relocate the player
        ciMap.MovePawn( i_ciPlayer, ciPos,
                        rumIgnoreTileCollisionMoveFlag | rumIgnorePawnCollisionMoveFlag | rumIgnoreDistanceMoveFlag );

        // Temporarily hide the player
        i_ciPlayer.SetVisibility( false );

        // Prevent the player from moving
        i_ciPlayer.SetProperty( Unconscious_PropertyID, true );

        // Modify the bed state, so that clients are updated
        local ciBed;
        local ciPosData = ciMap.GetPositionData( ciPos );
        while( ciBed = ciPosData.GetNext( rumWidgetPawnType, U4_Bed_Head_WidgetID ) )
        {
          ciBed.SetState( 1 );
          ciPosData.Stop();
        }

        // Determine if the player will be attacked during the night. The inn invasion chance is a percentage,
        // based on this point system:
        // 5% base chance - all inns start with this, so at the very least there's a 5% chance of invasion
        // +5% if the inn is inn a coastal village or town (within 3 squares) frequented by pirates
        // +5% if there are no town guards
        // +5% if there is a pub/tavern
        // +5% if the inn is in a village, or the inn is very cheap
        if( rand() % 100 < ciMap.GetProperty( Merchant_Inn_Invasion_Chance_PropertyID, 5 ) )
        {
          local fInterval = rand() % ciMap.s_iInnInvasionVariance + ciMap.s_fInnInvasionMinimumTime;
          ::rumSchedule( i_ciPlayer, i_ciPlayer.OnInnInvasion, fInterval, i_iRoomIndex );
        }
        else
        {
          // Uneventful night
          ::rumSchedule( i_ciPlayer, i_ciPlayer.OnInnCheckout, ciMap.s_fInnOccupancyInterval, i_iRoomIndex );
        }

        // The room is no longer vacant
        SetInnRoomVacant( ciMap, i_iRoomIndex, false );

        // Schedule the vacancy
        ::rumSchedule( ciMap, ciMap.OnInnVacancy, ciMap.s_fInnOccupancyInterval, i_iRoomIndex );

        local ciBroadcast = ::rumCreate( Merchant_Inn_BroadcastID, MerchantInnTransaction.RoomPurchase );
        i_ciPlayer.SendBroadcast( ciBroadcast );

        bSuccess = true;
      }
    }
    else
    {
      strResult = "u4_merchant_inn_room_occupied";
    }
  }

  if( !bSuccess )
  {
    // Terminate the transaction
    local ciBroadcast = ::rumCreate( Merchant_Inn_BroadcastID, MerchantInnTransaction.ServerTerminated, strResult );
    i_ciPlayer.SendBroadcast( ciBroadcast );
  }
}
