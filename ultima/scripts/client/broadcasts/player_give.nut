// Sent from client
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
      SetRecipient( vargv[1] );
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


  function GetPlayer()
  {
    // Get the player who initiated the give
    return ::rumFetchPawn( var2 );
  }


  function GetTradeType()
  {
    return var1;
  }


  function GiveSuccessful( i_eStringID )
  {
    local ciTarget = GetPlayer();
    if( ( null == ciTarget ) || !( ciTarget instanceof Player ) )
    {
      return;
    }

    local strDesc = "";

    local eItemType = GetItemType();
    switch( eItemType )
    {
      case SharedTradeType.Property:
        local ePropertyID = GetItemSubtype();
        local iAmount = GetAmount();

        // The traded property's description
        local ciProperty = ::rumGetPropertyAsset( ePropertyID );
        if( iAmount > 1 )
        {
          strDesc = format( "%d %s", iAmount,
                            ::rumGetStringByName( ciProperty.GetName() + "_Property_client_StringID" ) );
        }
        else
        {
          strDesc = format( "%s", ::rumGetStringByName( ciProperty.GetName() + "_Property_client_StringID" ) );
        }
        break;

      case SharedTradeType.Inventory:
        local ciPlayer = ::rumGetMainPlayer();
        local eAssetID = GetItemSubtype();
        local ciAsset = ::rumGetInventoryAsset( eAssetID );
        local iAmount = GetAmount();

        // Get the traded item's description
        if( iAmount > 1 )
        {
          strDesc = format( "%d %s", iAmount, ::rumGetStringByName( ciAsset.GetName() + "_client_StringID" ) );
        }
        else
        {
          strDesc = format( "%s", ::rumGetStringByName( ciAsset.GetName() + "_client_StringID" ) );
        }
        break;
    }

    // The final trade description
    local strResult = ::rumGetString( i_eStringID );
    strDesc = format( strResult + " %s", strDesc, ciTarget.GetPlayerName() );
    ShowString( strDesc, g_strColorTagArray.Yellow );
  }


  function OnRecv()
  {
    local ciPlayer = ::rumGetMainPlayer();

    local eTradeType = GetTradeType();
    switch( eTradeType )
    {
      case TradeBroadcastType.GiveNotify:
        local ciSrcPlayer = GetPlayer();
        if( ciSrcPlayer != null && ( ciSrcPlayer instanceof Player ) && ciPlayer != ciSrcPlayer )
        {
          local strDesc = format( "%s %s",
                                  ciSrcPlayer.GetPlayerName(),
                                  ::rumGetString( msg_give_request_client_StringID ) );
          ShowString( strDesc, g_strColorTagArray.Yellow );
        }
        break;

      case TradeBroadcastType.GiveSent:
        GiveSuccessful( msg_give_sent_client_StringID );
        break;

      case TradeBroadcastType.GiveReceived:
        GiveSuccessful( msg_give_received_client_StringID );
        break;
    }

    Ultima_Stat_Update();
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


  function SetRecipient( i_uiTargetID )
  {
    var2 = i_uiTargetID;
  }


  function SetTradeType( i_eTradeType )
  {
    var1 = i_eTradeType;
  }
}
