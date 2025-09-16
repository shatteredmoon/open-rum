// Sent from client
class Player_Get_Broadcast extends rumBroadcast
{
  var = 0;

  function OnRecv()
  {
    if( GameType.Ultima1 == g_ciCUO.m_eVersion )
    {
      local strDesc = ::rumGetString( msg_u1_chest_holds_client_StringID );
      strDesc += HandleCoin();
      ShowString( strDesc );
    }
    else
    {
      local iGold = var;
      local strDesc = format( ::rumGetString( msg_u4_chest_holds_client_StringID ), iGold );
      ShowString( strDesc );
    }
  }


  function HandleCoin()
  {
    local iCoin = var;
    local iCopper = iCoin % 10;
    local iSilver = ( iCoin / 10 ) % 10;
    local iGold = iCoin / 100;

    local strDesc = "";

    if( iGold != 0 )
    {
      strDesc += format( "%d %s", iGold, ::rumGetString( token_gold_client_StringID ) );
    }

    if( iSilver != 0 )
    {
      if( iGold != 0 )
      {
        strDesc += ", ";
      }

      strDesc += format( "%d %s", iSilver, ::rumGetString( token_silver_client_StringID ) );
    }

    if( iCopper != 0 )
    {
      if( !( iGold == 0 && iSilver == 0 ) )
      {
        strDesc += ", ";
      }

      strDesc += format( "%d %s", iCopper, ::rumGetString( token_copper_client_StringID ), iCopper );
    }

    return strDesc;
  }
}
