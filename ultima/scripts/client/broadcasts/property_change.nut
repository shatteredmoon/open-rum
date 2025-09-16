// Received from server to show results of a player action or game event
class Change_PropertyID_Broadcast extends rumBroadcast
{
  var1 = null; // Property class
  var2 = 1;    // Integer change
  var3 = null; // Color tag

  function OnRecv()
  {
    if( var2 > 0 )
    {
      if( U1_Coin_PropertyID == var1 )
      {
        OnRecvCoin();
      }
      else
      {
        local strDesc = format( "%s + %d", ::rumGetString( var1.s_strDescription ), var2 );
        ShowString( strDesc, var3 );
      }
    }
    else if( var2 < 0 )
    {
      local strDesc = format( "%s - %d", ::rumGetString( var1.s_strDescription ), abs( var2 ) );
      ShowString( strDesc, var3 );
    }
    else
    {
      local strDesc = format( "%s", ::rumGetString( var1.s_strDescription ) );
      ShowString( strDesc, var3 );
    }
  }


  function OnRecvCoin()
  {
    local iCoin = var2;
    local iCopper = iCoin % 10;
    local iSilver = ( iCoin / 10 ) % 10;
    local iGold = iCoin / 100;

    local strDesc = "";

    if( iGold != 0 )
    {
      strDesc += format( "%s + %d", ::rumGetString( token_gold_client_StringID ), iGold );
    }

    if( iSilver != 0 )
    {
      if( iGold != 0 )
      {
        strDesc += ", ";
      }

      strDesc += format( "%s + %d", ::rumGetString( token_silver_client_StringID ), iSilver );
    }

    if( iCopper != 0 )
    {
      if( !( iGold == 0 && iSilver == 0 ) )
      {
        strDesc += ", ";
      }

      strDesc += format( "%s + %d", ::rumGetString( token_copper_client_StringID ), iCopper );
    }

    if( strDesc != "" )
    {
      ShowString( strDesc, var3 );
    }
  }
}
