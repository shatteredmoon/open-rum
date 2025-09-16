function TransactTransportRecv( i_ciPlayer, i_eTransactionType, i_eTransportID )
{
  if( MerchantTransportTransaction.Purchase != i_eTransactionType )
  {
    // Why was the message even sent?
    i_ciPlayer.IncrementHackAttempts();
    return;
  }

  local ciMap = i_ciPlayer.GetMap();
  local eTransportArray = null;

  // Determine the data table row
  local eMapArray = ::rumGetDataTableColumn( merchant_transports_DataTableID, 0 );
  local iRow = eMapArray.find( ciMap.GetAssetID() );
  if( iRow != null )
  {
    // Fetch the row and slice off the map column
    eTransportArray = ::rumGetDataTableRow( merchant_transports_DataTableID, iRow ).slice( 1, 7 );
  }

  // Does the merchant sell this transport?
  if( ValueInContainer( i_eTransportID, eTransportArray ) )
  {
    local fDiscount = i_ciPlayer.GetDiscountPercent();

    // What's the going rate for the transport?
    local ciTransport = ::rumGetWidgetAsset( i_eTransportID );
    if( null == ciTransport )
    {
      local ciBroadcast = ::rumCreate( Merchant_Transport_BroadcastID, MerchantTransportTransaction.ServerTerminated );
      i_ciPlayer.SendBroadcast( ciBroadcast );

      return;
    }

    local iPrice = ciTransport.GetProperty( Merchant_Price_PropertyID, 0 );
    iPrice = iPrice - ( iPrice * fDiscount ).tointeger();
    iPrice = max( iPrice, 1 );
    if( iPrice <= 0 )
    {
      // Transport price not found
      i_ciPlayer.IncrementHackAttempts();

      local ciBroadcast = ::rumCreate( Merchant_Transport_BroadcastID, MerchantTransportTransaction.ServerTerminated );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
    else
    {
      // Verify gold
      local iPlayerGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );

      if( ( U1_Shuttle_WidgetID == i_eTransportID ) && i_ciPlayer.GetProperty( U1_Shuttle_Pass_PropertyID, false ) )
      {
        // The player already has a shuttle pass, so there's no charge
        iPrice = 0;
      }

      // Deduct gold from the player
      if( iPrice > 0 )
      {
        i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iPrice );
      }

      if( iPlayerGold < iPrice )
      {
        local ciBroadcast = ::rumCreate( Merchant_Transport_BroadcastID, MerchantTransportTransaction.Purchase,
                                         merchant_transport_cant_pay_client_StringID );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else
      {
        if( U1_Shuttle_WidgetID == i_eTransportID )
        {
          // Give the player a shuttle pass - it can be redeemed for a flight, should the user exit
          // the shuttle or logout prior to taking off
          i_ciPlayer.SetProperty( U1_Shuttle_Pass_PropertyID, true );

          local bCreate = true;

          // Create the shuttle only if the player is not in a group, or the group doesn't already
          // have a shuttle present on this map.
          local ciParty = i_ciPlayer.GetParty();
          if( ciParty != null )
          {
            foreach( uiMemberID in ciParty.m_uiRosterTable )
            {
              local ciMember = ::rumFetchPawn( uiMemberID );
              local ciMap = ciMember.GetMap();
              if( ciMap == this )
              {
                local ciTransport = ciMember.GetTransport();
                if( ciTransport != null )
                {
                  local eTransportType = ciTransport.GetType();
                  if( TransportType.SpaceShip == eTransportType )
                  {
                    // Attempt to board the existing shuttle
                    local ciBroadcast = ::rumCreate( Merchant_Transport_BroadcastID,
                                                     MerchantTransportTransaction.ServerTerminated );
                    i_ciPlayer.SendBroadcast( ciBroadcast );

                    ciTransport.Board( i_ciPlayer );

                    bCreate = false;
                    break;
                  }
                }
              }
            }
          }

          if( bCreate )
          {
            local iShuttlePosX = ::rumGetDataTableRow( merchant_transports_DataTableID, iRow )[7];
            local iShuttlePosY = ::rumGetDataTableRow( merchant_transports_DataTableID, iRow )[8];
            local ciShuttlePos = rumPos( iShuttlePosX, iShuttlePosY );
            local ciTransport = CreateTransport( U1_Shuttle_WidgetID, 0, i_ciPlayer.GetMap(), ciShuttlePos );
            if( ciTransport != null )
            {
              ciTransport.Board( i_ciPlayer );
            }

            local ciBroadcast = ::rumCreate( Merchant_Transport_BroadcastID,
                                             MerchantTransportTransaction.ServerTerminated );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
        }
        else if( U1_Frigate_WidgetID == i_eTransportID )
        {
          local iFrigatePosX = ::rumGetDataTableRow( merchant_transports_DataTableID, iRow )[9];
          local iFrigatePosY = ::rumGetDataTableRow( merchant_transports_DataTableID, iRow )[10];
          local ciFrigatePos = rumPos( iFrigatePosX, iFrigatePosY );

          local ciBroadcast = ::rumCreate( Merchant_Transport_BroadcastID,
                                           MerchantTransportTransaction.ServerTerminated );
          i_ciPlayer.SendBroadcast( ciBroadcast );

          local ciNewMap = GetOrCreateMap( i_ciPlayer, U1_Sosaria_MapID );
          if( ciNewMap != null )
          {
            local ciTransport = CreateTransport( U1_Frigate_WidgetID, 0, ciNewMap, ciFrigatePos );
            if( ciTransport != null )
            {
              // The commander is always transferred last
              if( ciMap.TransferPawn( i_ciPlayer, ciNewMap, ciFrigatePos ) )
              {
                ciTransport.Board( i_ciPlayer );
              }
            }
          }
        }
        else
        {
          switch( i_eTransportID )
          {
            case U1_Aircar_WidgetID: i_ciPlayer.SetProperty( U1_Aircar_PropertyID, true ); break;
            case U1_Cart_WidgetID:   i_ciPlayer.SetProperty( U1_Cart_PropertyID, true );   break;
            case U1_Horse_WidgetID:  i_ciPlayer.SetProperty( U1_Horse_PropertyID, true );  break;
            case U1_Raft_WidgetID:   i_ciPlayer.SetProperty( U1_Raft_PropertyID, true );   break;
          }

          // Notify client
          local ciBroadcast = ::rumCreate( Merchant_Transport_BroadcastID,
                                           MerchantTransportTransaction.Purchase,
                                           merchant_transport_buy_yes_client_StringID );
          i_ciPlayer.SendBroadcast( ciBroadcast );

          local ciTransport = CreateTransport( i_eTransportID, 0, ciMap, i_ciPlayer.GetPosition() );
          if( ciTransport != null && i_eTransportID != U1_Raft_WidgetID )
          {
            ciTransport.Board( i_ciPlayer );
          }
        }
      }
    }
  }
  else
  {
    // Hack attempt - transport not sold by this merchant
    i_ciPlayer.IncrementHackAttempts();

    local ciBroadcast = ::rumCreate( Merchant_Transport_BroadcastID, MerchantTransportTransaction.ServerTerminated );
    i_ciPlayer.SendBroadcast( ciBroadcast );
  }
}


function TransactTransportStart( i_ciPlayer )
{
  local ciBroadcast = ::rumCreate( Merchant_Transport_BroadcastID, MerchantTransportTransaction.Greet );
  ::rumSendPrivate( i_ciPlayer.GetSocket(), ciBroadcast );
}
