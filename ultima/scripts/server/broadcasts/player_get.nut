// Received from client when player gets a chest
// Sent from server to inform client how much gold was found
class Player_Get_Broadcast extends rumBroadcast
{
  var = 0;


  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var = vargv[0]; // Amount of gold found
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local eDelay = ActionDelay.Short;
    local eVersion = i_ciPlayer.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );

    local bFound = false;
    local bCanOpen = true;

    local ciWidget;
    local ciMap = i_ciPlayer.GetMap();
    local ciPosData = ciMap.GetPositionData( i_ciPlayer.GetPosition() );
    while( ciWidget = ciPosData.GetNext( rumWidgetPawnType ) )
    {
      bCanOpen = true;

      if( ciWidget.IsVisible() )
      {
        local eAssetID = ciWidget.GetAssetID();

        if( ( ( U4_Chest_WidgetID == eAssetID ) && ( GameType.Ultima4 == eVersion ) ) ||
            ( ( U3_Chest_WidgetID == eAssetID ) && ( GameType.Ultima3 == eVersion ) ) ||
            ( ( U2_Chest_WidgetID == eAssetID ) && ( GameType.Ultima2 == eVersion ) ) ||
            ( ( U1_Chest_WidgetID == eAssetID ) && ( GameType.Ultima1 == eVersion ) ) )
        {
          bFound = true;

          // Don't let the player get a chest if they're at max gold
          local iMaxGold = ::rumGetMaxPropertyValue( g_eGoldPropertyVersionArray[eVersion] );
          local iGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
          if( iGold >= iMaxGold )
          {
            // Player is capped on gold
            bCanOpen = false;
            i_ciPlayer.ActionFailed( msg_cant_carry_more_client_StringID );
          }
        }
        else if( ( ( U2_Shield_WidgetID == eAssetID ) || ( U2_Sword_WidgetID == eAssetID ) ) &&
                 ( GameType.Ultima2 == eVersion ) )
        {
          bFound = true;
          bCanOpen = ciWidget.CanOpen( i_ciPlayer );
        }

        else if( ( U1_Gem_Immortality_WidgetID == eAssetID ) && ( GameType.Ultima1 == eVersion ) )
        {
          bFound = true;
          bCanOpen = true;
        }

        if( bFound && bCanOpen )
        {
          // Open the container without using magic
          ciWidget.Open( i_ciPlayer, false );
          ciPosData.Stop();
          eDelay = ActionDelay.Long;
        }
      }
    }

    if( !bFound )
    {
      i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
    }

    if( !bCanOpen )
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
    }

    local fDelay = i_ciPlayer.GetActionDelay( eDelay );
    ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
  }
}
