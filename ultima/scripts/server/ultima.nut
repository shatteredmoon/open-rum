/*----------------------------------------------------------------------------

ooooo  oooo o888    o8    o88
 888    88   888  o888oo  oooo  oo ooo oooo     ooooooo
 888    88   888   888     888   888 888 888    ooooo888
 888    88   888   888     888   888 888 888  888    888
  888oo88   o888o   888o  o888o o888o888o888o  88ooo88 8o

----------------------------------------------------------------------------*/

g_ciGameTest <- null;


function CreateTransport( i_eWidgetID, i_uiTransportCode, i_ciMap, i_ciPos )
{
  local ciTransport = ::rumCreate( i_eWidgetID );
  if( ciTransport != null )
  {
    if( i_ciMap != null && i_ciMap.AddPawn( ciTransport, i_ciPos ) )
    {
      ciTransport.ScheduleCleanup();

      if( i_uiTransportCode != 0 )
      {
        // Use the override code
        ciTransport.m_uiTransportCode = i_uiTransportCode;
      }
    }
    else
    {
      ciTransport.Destroy();
    }
  }

  return ciTransport;
}


function FindTransport( i_ciMap, i_uiTransportCode )
{
  if( null == i_ciMap )
  {
    return null;
  }

  local ciPawnArray = i_ciMap.GetAllPawns();
  foreach( ciPawn in ciPawnArray )
  {
    if( ciPawn.IsVisible() && ( ciPawn instanceof Passenger_Transport_Widget ) )
    {
      if( ciPawn.m_uiTransportCode == i_uiTransportCode )
      {
        return ciPawn;
      }
    }
  }
}


function GetOrCreateMap( i_ciPlayer, i_eAssetID )
{
  local ciAsset = ::rumGetMapAsset( i_eAssetID );
  if( null == ciAsset )
  {
    return null;
  }

  local ciMap = null;

  // Determine the instance behavior of the map
  local bSolo = ciAsset.GetProperty( Map_Solo_Instance_PropertyID, false );
  if( bSolo )
  {
    // Does the player have an instance of the map?
    ciMap = GetSoloMap( i_ciPlayer, i_eAssetID );
    if( null == ciMap )
    {
      // Create the map
      ciMap = ::rumCreate( i_eAssetID );
      if( ciMap != null )
      {
        local uiPlayerID = i_ciPlayer.GetPlayerID();
        if( !( uiPlayerID in g_ciServer.m_ciSoloMapTable ) )
        {
          // Create the player's solo map table
          g_ciServer.m_ciSoloMapTable[uiPlayerID] <- {};
        }

        local ciMapTable = g_ciServer.m_ciSoloMapTable[uiPlayerID];

        // Hold a reference to the map object
        ciMapTable[i_eAssetID] <- ciMap;
      }
    }
  }
  else
  {
    ciMap = GetPublicMap( i_eAssetID );
    if( null == ciMap )
    {
      // Create the map
      ciMap = ::rumCreate( i_eAssetID );
      if( ciMap != null )
      {
        // Hold a reference to the map object
        g_ciServer.m_ciPublicMapTable[i_eAssetID] <- ciMap;
      }
    }

    // TODO - make all dungeons optional private instances
    /*
    local bPrivate = ciMap.GetProperty( Map_Private_Instance_PropertyID, false );
    if( bPrivate )
    {
      local ciParty = i_ciPlayer.GetParty();
      if( ciParty != null )
      {
        // Create a new party and add the initial member
        ciParty = Party();
        ciParty.Add( ciPlayer, true );
      }

      // Does the party have an instance of the map?
      local uiMapID = GetPrivateMapID( ciParty.m_iID, i_eAssetID );
      if( uiMapID != rumInvalidGameID )
      {
        return ::rumGetMap( uiMapID );
      }
      else
      {
        // Create the map
        ciMap = ::rumCreate( i_eAssetID );
        if( ciMap != null )
        {
          // Hold a reference to the map object
          g_ciServer.m_ciPrivateMapTable[ciParty.m_iID] <- ciMap;
        }
      }
    }*/
  }

  return ciMap;
}


function GetPublicMap( i_eAssetID )
{
  if( i_eAssetID in g_ciServer.m_ciPublicMapTable )
  {
    return g_ciServer.m_ciPublicMapTable[i_eAssetID];
  }

  return null;
}


function GetSoloMap( i_ciPlayer, i_eAssetID )
{
  if( null == i_ciPlayer )
  {
    return null;
  }

  local uiPlayerID = i_ciPlayer.GetPlayerID();
  if( uiPlayerID in g_ciServer.m_ciSoloMapTable )
  {
    local ciMapTable = g_ciServer.m_ciSoloMapTable[uiPlayerID];
    foreach( ciMap in ciMapTable )
    {
      if( ciMap.GetAssetID() == i_eAssetID )
      {
        return ciMap;
      }
    }
  }

  return null;
}


function OnPlayerLogout( i_iSocket )
{
  local ciPlayer = ::rumGetPlayerBySocket( i_iSocket );
  if( ciPlayer != null )
  {
    ciPlayer.OnLogout();
  }
}


function ReleaseAllMaps()
{
  g_ciServer.m_ciPublicMapTable.clear();
  g_ciServer.m_ciPublicMapTable = null;

  g_ciServer.m_ciSoloMapTable.clear();
  g_ciServer.m_ciSoloMapTable = null;

  //g_ciServer.m_ciPrivateMapTable.clear();
  //g_ciServer.m_ciPrivateMapTable = null;
}


function ReleaseMap( i_ciMap )
{
  if( null == i_ciMap )
  {
    return;
  }

  local eAssetID = i_ciMap.GetAssetID();

  // Determine the instance behavior of the map
  if( i_ciMap.GetProperty( Map_Solo_Instance_PropertyID, false ) )
  {
    // Release all players from this map (should only be one, but have to account for admins)
    local ciPlayerArray = rumGetAllPlayers();
    foreach( ciPlayer in ciPlayerArray )
    {
      local uiPlayerID = ciPlayer.GetPlayerID();
      if( g_ciServer.m_ciSoloMapTable.rawin( uiPlayerID ) )
      {
        local ciMapTable = g_ciServer.m_ciSoloMapTable[uiPlayerID];
        if( ciMapTable != null && ciMapTable.rawin( i_ciMap.GetAssetID() ) )
        {
          local ciMap = ciMapTable[eAssetID];
          if( ciMap.GetID() == i_ciMap.GetID() )
          {
            // Delete the entry for the map
            delete ciMapTable[eAssetID];

            if( ciMapTable.len() == 0 )
            {
              // Also delete the player from the solo map table
              delete g_ciServer.m_ciSoloMapTable[uiPlayerID];
            }

            return;
          }
        }
      }
    }
  }
  else if( i_ciMap.GetProperty( Map_Private_Instance_PropertyID, false ) )
  {
    // TODO - Release from private maps
  }
  else
  {
    // Release the reference to the map object
    g_ciServer.m_ciPublicMapTable[eAssetID] <- null;
  }
}
