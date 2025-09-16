// Received from client
class Player_Give_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;
  var3 = 0;
  var4 = 0;
  var5 = 0;


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() == 2 )
    {
      SetTradeType( vargv[0] );
      SetPlayer( vargv[1] );
    }
  }


  function GetAmount()
  {
    return var5;
  }


  function GetItemSubtype()
  {
    return var4;
  }


  function GetItemType()
  {
    return var3;
  }


  function GetTargetPlayer()
  {
    return ::rumFetchPawn( var2 );
  }


  function GetTradeType()
  {
    return var1;
  }


  function Give( i_ciPlayer, i_ciTarget, i_ciMap )
  {
    // Verify that the src player and target player are adjacent
    local iDistance = i_ciMap.GetTileDistance( i_ciPlayer.GetPosition(), i_ciTarget.GetPosition() );
    if( iDistance <= 1 )
    {
      local eItemType = GetItemType();
      switch( eItemType )
      {
        case SharedTradeType.Property:
          local ePropertyID = GetItemSubtype();
          GivePropertyAmount( ePropertyID, i_ciPlayer, i_ciTarget );
          break;

        case SharedTradeType.Inventory:
          local ciObject = i_ciPlayer.GetInventory( GetItemSubtype() );
          GiveInventory( ciObject, i_ciPlayer, i_ciTarget );
          break;

        default:
          // Atempt to give an unknown type
          i_ciPlayer.IncrementHackAttempts();
          break;
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
    }
  }


  function GiveInventory( i_ciObject, i_ciPlayer, i_ciTarget )
  {
    if( null == i_ciObject || !( i_ciObject instanceof rumInventory ) )
    {
      return;
    }

    local uiObjectID = i_ciObject.GetID();
    local uiEquippedArmourID = i_ciPlayer.GetEquippedArmourID();
    local uiEquippedWeaponID = i_ciPlayer.GetEquippedWeaponID();

    if( ( uiObjectID == uiEquippedArmourID ) || ( uiObjectID == uiEquippedWeaponID ) )
    {
      // Players cannot trade equipped weapons or armour
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
    }
    else
    {
      // Players can't trade bound items
      local bBound = i_ciObject.GetProperty( Inventory_Bound_PropertyID, false );
      if( !bBound )
      {
        local iAmount = GetAmount();

        if( iAmount > 1 && i_ciObject.GetProperty( Inventory_Stacks_PropertyID, false ) )
        {
          // Determine how many the owner can transfer (up to the amount passed to the server)
          local iOwnerNumOwned = i_ciObject.GetProperty( Inventory_Quantity_PropertyID, 1 );
          iAmount = min( iAmount, iOwnerNumOwned );

          local iMaxQuantity = ::rumGetMaxPropertyValue( Inventory_Quantity_PropertyID );

          // Determine how many the recipient can carry
          local iNumRecipientOwned = 0;
          local iNumRecipientCanReceive = iMaxQuantity;
          local eItemAssetType = i_ciObject.GetAssetID();
          local ciRecipientObject = i_ciTarget.GetInventoryByType( eItemAssetType );
          if( ciRecipientObject != null )
          {
            iNumRecipientOwned = ciRecipientObject.GetProperty( Inventory_Quantity_PropertyID, 1 );
            iNumRecipientCanReceive = iMaxQuantity - iNumRecipientOwned;
          }

          // Adjust down to however many the recipient can accept
          iAmount = min( iAmount, iNumRecipientCanReceive );

          if( iAmount > 1 )
          {
            // Stack transfer
            if( iAmount < iOwnerNumOwned )
            {
              // Reduce the owner's quantity
              i_ciObject.SetProperty( Inventory_Quantity_PropertyID, iOwnerNumOwned - iAmount );

              if( null == ciRecipientObject )
              {
                i_ciTarget.AddOrCreateItem( eItemAssetType, iAmount );
              }
              else
              {
                // Increase the recipient's quantity
                ciRecipientObject.SetProperty( Inventory_Quantity_PropertyID, iNumRecipientOwned + iAmount );
              }
            }
            else if( null == ciRecipientObject )
            {
              i_ciPlayer.TransferInventory( i_ciObject, i_ciTarget );
            }
            else
            {
              // Remove the owner's item
              i_ciPlayer.DeleteInventory( i_ciObject );

              // Increase the recipient's quantity
              ciRecipientObject.SetProperty( Inventory_Quantity_PropertyID, iNumRecipientOwned + iAmount );
            }
          }
          else
          {
            i_ciPlayer.TransferInventory( i_ciObject, i_ciTarget );
          }
        }

        // Notify the sender
        local ciBroadcast = ::rumCreate( Player_Give_BroadcastID );
        ciBroadcast.SetAmount( iAmount );
        ciBroadcast.SetItemSubtype( i_ciObject.GetAssetID() );
        ciBroadcast.SetItemType( SharedTradeType.Inventory );
        ciBroadcast.SetPlayer( i_ciTarget );
        ciBroadcast.SetTradeType( TradeBroadcastType.GiveSent );
        i_ciPlayer.SendBroadcast( ciBroadcast );

        // Notify the receiver
        ciBroadcast.SetPlayer( i_ciPlayer );
        ciBroadcast.SetTradeType( TradeBroadcastType.GiveReceived );
        i_ciTarget.SendBroadcast( ciBroadcast );
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      }
    }
  }


  function GivePropertyAmount( i_ePropertyID, i_ciPlayer, i_ciTarget )
  {
    // Players can only transfer properties that are marked as transferable
    local ciProperty = ::rumGetPropertyAsset( i_ePropertyID );
    if( null == ciProperty )
    {
      return;
    }

    local iFlags = ciProperty.GetUserFlags();
    if( ( iFlags & PropertyUserFlags.Transferable ) != 0 )
    {
      local iAmount = GetAmount();
      local iSourceAmount = i_ciPlayer.GetProperty( i_ePropertyID, 0 );
      local iDestAmount = i_ciTarget.GetProperty( i_ePropertyID, 0 );
      local iMaxAmount = ciProperty.GetMaxValue();
      local iDestSpaceAvailable = iMaxAmount - iDestAmount;

      // Does the source player have enough?
      iAmount = min( iSourceAmount, iAmount );

      if( ( U4_Food_PropertyID == i_ePropertyID ) || ( U3_Food_PropertyID == i_ePropertyID ) ||
          ( U2_Food_PropertyID == i_ePropertyID ) || ( U1_Food_PropertyID == i_ePropertyID ) )
      {
        // Players cannot trade away their last 25 units of food (to prevent a Lord British resurrection exploit)
        if( iSourceAmount - iAmount <= 25 )
        {
          iAmount = max( 0, iSourceAmount - 25 );
        }
      }

      // Can the destination player hold the given amount?
      iAmount = min( iDestSpaceAvailable, iAmount );

      if( iAmount > 0 )
      {
        // Remove the amount from the source player
        i_ciPlayer.SetProperty( i_ePropertyID, iSourceAmount - iAmount );

        // Give the amount to the destination player
        i_ciTarget.SetProperty( i_ePropertyID, iDestAmount + iAmount );

        // Notify the sender
        local ciBroadcast = ::rumCreate( Player_Give_BroadcastID );
        ciBroadcast.SetAmount( iAmount );
        ciBroadcast.SetItemSubtype( i_ePropertyID );
        ciBroadcast.SetItemType( SharedTradeType.Property );
        ciBroadcast.SetPlayer( i_ciTarget );
        ciBroadcast.SetTradeType( TradeBroadcastType.GiveSent );
        i_ciPlayer.SendBroadcast( ciBroadcast );

        // Notify the receiver
        ciBroadcast.SetPlayer( i_ciPlayer );
        ciBroadcast.SetTradeType( TradeBroadcastType.GiveReceived );
        i_ciTarget.SendBroadcast( ciBroadcast );
      }
      else
      {
        // Target player doesn't have room
        i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      }
    }
    else
    {
      // Atempt to transfer a property that isn't transferable
      i_ciPlayer.IncrementHackAttempts();
    }
  }


  function NotifyRecipient( i_ciPlayer, i_ciTarget, i_ciMap )
  {
    // Verify that the src player and target player are reasonably close
    local iDistance = i_ciMap.GetTileDistance( i_ciPlayer.GetPosition(), i_ciTarget.GetPosition() );
    if( iDistance <= 5 )
    {
      local ciBroadcast = ::rumCreate( Player_Give_BroadcastID, TradeBroadcastType.GiveNotify, i_ciPlayer );
      i_ciTarget.SendBroadcast( ciBroadcast );
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( i_ciPlayer != null && ( i_ciPlayer instanceof Player ) )
    {
      local eDelay = ActionDelay.Short;

      local ciTarget = GetTargetPlayer();
      if( ciTarget != null && ( ciTarget instanceof Player ) && i_ciPlayer != ciTarget )
      {
        local ciSrcMap = i_ciPlayer.GetMap();
        local ciDestMap = ciTarget.GetMap();

        if( ciSrcMap == ciDestMap )
        {
          local eTradeType = GetTradeType();
          switch( eTradeType )
          {
            case TradeBroadcastType.GiveNotify:
              NotifyRecipient( i_ciPlayer, ciTarget, ciSrcMap );
              break;

            case TradeBroadcastType.Give:
              Give( i_ciPlayer, ciTarget, ciSrcMap );
              break;
          }

          eDelay = ActionDelay.Long;
        }
        else
        {
          i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
        }
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
      }

      local fDelay = i_ciPlayer.GetActionDelay( eDelay );
      ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
    }
  }


  function SetAmount( i_iAmount )
  {
    var5 = i_iAmount;
  }


  function SetItemSubtype( i_eSubtype )
  {
    var4 = i_eSubtype;
  }


  function SetItemType( i_eType )
  {
    var3 = i_eType;
  }


  function SetPlayer( i_ciPlayer )
  {
    if( null != i_ciPlayer && ( i_ciPlayer instanceof Player ) )
    {
      var2 = i_ciPlayer.GetID();
    }
  }


  function SetTradeType( i_eTradeType )
  {
    var1 = i_eTradeType;
  }
}
