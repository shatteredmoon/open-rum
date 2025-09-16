// Received from client
class Player_Look_Broadcast extends rumBroadcast
{
  var1 = 0; // Object ID
  var2 = 0; // Var
  var3 = 0; // Var
  var4 = 0; // Var


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      var1 = vargv[0];

      if( vargv.len() > 1 )
      {
        var2 = vargv[1];

        if( vargv.len() > 2 )
        {
          var3 = vargv[2];

          if( vargv.len() > 3 )
          {
            var4 = vargv[3];
          }
        }
      }
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    // Determine which object the player is interacting with
    local ciObject = ::rumFetchPawn( var1 );
    if( ( null == ciObject ) || !( ciObject instanceof Widget ) || !ciObject.IsVisible() )
    {
      // Trying to interact with an invalid or invisible object
      i_ciPlayer.IncrementHackAttempts();
      return;
    }

    // The player must be adjacent to the object
    local ciPlayerPos = i_ciPlayer.GetPosition();
    local ciObjectPos = ciObject.GetPosition();
    local ciMap = ciObject.GetMap();
    if( !ciMap.IsPositionWithinTileDistance( ciPlayerPos, ciObjectPos, 1 ) )
    {
      // Trying to interact with an object that is too far away
      i_ciPlayer.IncrementHackAttempts();
      return;
    }

    local eAssetID = ciObject.GetAssetID();
    local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
    if( ( U3_Fountain_WidgetID == eAssetID ) || ( U4_Fountain_WidgetID == eAssetID ) ||
        ( U4_Orb_WidgetID == eAssetID ) )
    {
      if( ( MapType.Dungeon == eMapType ) || ( MapType.Abyss == eMapType ) )
      {
        ciObject.Use( i_ciPlayer );
      }
    }
    else if( U4_Altar_WidgetID == eAssetID )
    {
      if( MapType.Altar == eMapType )
      {
        ciObject.Use( i_ciPlayer, var2 );
      }
      else if( MapType.Abyss == eMapType )
      {
        ciObject.Use( i_ciPlayer, null );
      }
    }
    else if( U1_Pond_WidgetID == eAssetID )
    {
      ciObject.Use( i_ciPlayer, var2 );
    }
    else if( ( U1_Signpost_WidgetID == eAssetID ) ||
             ( U2_Signpost_WidgetID == eAssetID ) ||
             ( U3_Signpost_WidgetID == eAssetID ) )
    {
      ciObject.Use( i_ciPlayer );
    }
    else if( U3_Mark_Rod_WidgetID == eAssetID )
    {
      ciObject.Use( i_ciPlayer );
    }
    else if( ( U4_Telescope_WidgetID == eAssetID ) &&
             ( U4_Keep_Lycaeum_MapID == ciMap.GetAssetID() ) )
    {
      ciObject.Use( i_ciPlayer );
    }
    else if( ( U4_Codex_WidgetID == eAssetID ) &&
             ( U4_Dungeon_Codex_Chamber_MapID == ciMap.GetAssetID() ) )
    {
      ciObject.Use( i_ciPlayer );
    }
    else
    {
      // Trying to interact with an unsupported object
      i_ciPlayer.IncrementHackAttempts();
    }

    local fDelay = i_ciPlayer.GetActionDelay( ActionDelay.Short );
    ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
  }
}