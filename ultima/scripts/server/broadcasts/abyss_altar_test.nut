// Sent to client when a player looks at an Abyss altar
class Abyss_Altar_Test_Broadcast extends rumBroadcast
{
  var1 = 0; // Test phase or answer
  var2 = 0; // Answer


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
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local ePhase = var1;

    local ciPos = i_ciPlayer.GetPosition();
    local ciMap = i_ciPlayer.GetMap();

    local ciAltar = null;

    // Player must be adjacent to an altar
    local ciPawnArray = ciMap.GetPawns( ciPos, 1, false );
    foreach( ciPawn in ciPawnArray )
    {
      if( ciPawn.GetAssetID() == U4_Altar_WidgetID )
      {
        local eAltarType = ciPawn.GetProperty( Widget_Altar_Type_PropertyID, AltarType.Normal );
        if( AltarType.Abyss == eAltarType )
        {
          ciAltar = ciPawn;
          break;
        }
      }
    }

    if( null == ciAltar )
    {
      return;
    }

    if( U4_AbyssAltarPhaseType.Virtue == ePhase )
    {
      local eVirtue = var2;

      // Check the answer
      local eLevelVirtue = ciMap.GetProperty( Map_Level_PropertyID, 0 ) - 1;
      if( eLevelVirtue == eVirtue )
      {
        // The player gave the correct virtue
        local ciBroadcast = ::rumCreate( Abyss_Altar_Test_BroadcastID, U4_AbyssAltarPhaseType.Stone );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else
      {
        // Incorrect virtue specified
        i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
      }
    }
    else if( U4_AbyssAltarPhaseType.Stone == ePhase )
    {
      local eStone = var2;

      // Check the answer
      local eLevelStone = ciMap.GetProperty( Map_Level_PropertyID, 0 ) - 1;
      if( eLevelStone == eStone )
      {
        if( eLevelStone == VirtueType.Humility )
        {
          // TODO - remove player from group

          // Douse all player lights
          i_ciPlayer.ExtinguishLight();

          // The virtue test is over
          local ciBroadcast = ::rumCreate( Abyss_Altar_Test_BroadcastID, U4_AbyssAltarPhaseType.Codex );
          i_ciPlayer.SendBroadcast( ciBroadcast );
        }

        // The player used the correct stone, transfer the player to the next level
        local eMapID = ciAltar.GetProperty( Map_ID_PropertyID, rumInvalidAssetID );
        local ciDestMap = GetOrCreateMap( i_ciPlayer, eMapID );
        if( ciDestMap != null )
        {
          local ciDestPos = rumPos();
          ciDestPos.x = ciAltar.GetProperty( Map_PosX_PropertyID, 0 );
          ciDestPos.y = ciAltar.GetProperty( Map_PosY_PropertyID, 0 );

          ciMap.TransferPawn( i_ciPlayer, ciDestMap, ciDestPos );
        }
      }
      else
      {
        // Incorrect stone used
        i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
      }
    }

    i_ciPlayer.PopPacket();
  }
}
