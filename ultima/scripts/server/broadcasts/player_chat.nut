// TODO - Add mechanism for changing class (set the class property and update the graphic)
// TODO - Add mechanism for changing graphic (besides manual property change)

// Received from client when player enters a chat message or text command
// Send from server with chat message or command result string
class Player_Chat_Broadcast extends rumBroadcast
{
  var = ""; // Chat message or text command


  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var = vargv[0]; // Chat message or command result string

      // Permanently log the chat message
      local strChat = StripTags( var, "<C#", ">" );
      ::rumLogChat( strChat );
    }
  }


  function CommandError( i_ciTarget, i_strError )
  {
    local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, i_strError );
    i_ciTarget.SendBroadcast( ciBroadcast );
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !i_ciPlayer instanceof Player )
    {
      return;
    }

    local strChat = var;

    // If a slash exists at the front of the line, then treat the chat as a player or admin command
    if( "/" == strChat.slice( 0, 1 ) )
    {
      // Tokenize the string into individual parameters
      local strParamArray = split( strChat, " " );

      foreach( i, strToken in strParamArray )
      {
        print( "Token[" + i + "] = " + strToken + "\n" );
      }

      switch( strParamArray[0] )
      {
        case "/stuck":
          PlayerCommandStuck( i_ciPlayer, strParamArray );
          break;

        case "/password":
          PlayerCommandPassword( i_ciPlayer, strParamArray );
          break;

        case "/s":
        case "/say":
          PlayerCommandSay( i_ciPlayer, strChat.slice( strChat.find( " " ) + 1 ) );
          break;

        case "/g":
        case "/global":
          PlayerCommandGlobalSay( i_ciPlayer, strChat.slice( strChat.find( " " ) + 1 ) );
          break;

        case "/m":
        case "/map":
          PlayerCommandMapSay( i_ciPlayer, strChat.slice( strChat.find( " " ) + 1 ) );
          break;

        case "/p":
          PlayerCommandPartySay( i_ciPlayer, strChat.slice( strChat.find( " " ) + 1 ) );
          break;

        case "/party":
          PlayerCommandParty( i_ciPlayer, strParamArray );
          break;

        case "/t":
        case "/tell":
          PlayerCommandTell( i_ciPlayer, strParamArray );
          break;
      }

      // Make sure this is an admin player
      if( i_ciPlayer.IsAdmin() )
      {
        switch( strParamArray[0] )
        {
          case "/create":
            AdminCommandCreate( i_ciPlayer, strParamArray );
            break;

          case "/goto":
          case "/teleport":
            AdminCommandGoto( i_ciPlayer, strParamArray );
            break;

          case "/jackpot":
            AdminCommandJackpot( i_ciPlayer );
            break;

          case "/jail":
            if( strParamArray.len() == 2 )
            {
              // Sets notoriety to max by default
              AdminCommandJail( i_ciPlayer, strParamArray[1], 24 );
            }
            else if( strParamArray.len() == 3 )
            {
              // Target notoriety level set by 3rd parameter
              AdminCommandJail( i_ciPlayer, strParamArray[1], strParamArray[2].tointeger() );
            }
            else
            {
              local strError = g_strColorTagArray.Red + "Invalid format for command: " + g_strColorTagArray.White +
                               strParamArray[0];
              CommandError( i_ciPlayer, strError );
            }
            break;

          case "/loc":
          case "/locate":
            AdminCommandLocate( i_ciPlayer, strParamArray.len() > 1 ? strParamArray[1] : null );
            break;

          case "/property":
            AdminCommandProperty( i_ciPlayer, strParamArray );
            break;

          case "/summon":
            if( strParamArray.len() > 1 )
            {
              AdminCommandSummon( i_ciPlayer, strParamArray[1] );
            }
            else
            {
              local strError = g_strColorTagArray.Red + "Invalid format for command: " + g_strColorTagArray.White +
                               strParamArray[0];
              CommandError( i_ciPlayer, strError );
            }
            break;

          case "/test":
            AdminCommandTest( i_ciPlayer, strParamArray[1] );
            break;

          case "/ultima":
            if( strParamArray.len() > 1 )
            {
              AdminCommandUltima( i_ciPlayer, strParamArray[1] );
            }
            else
            {
              local strError = g_strColorTagArray.Red + "Invalid format for command: " + g_strColorTagArray.White +
                               strParamArray[0];
              CommandError( i_ciPlayer, strError );
            }
            break;
        }
      }
    }
    else
    {
      PlayerCommandSay( i_ciPlayer, strChat );
    }

    i_ciPlayer.PopPacket();
  }


  function AdminCommandCreate( i_ciPlayer, i_strParamArray )
  {
    local bShowUsage = false;

    if( i_strParamArray.len() >= 1 )
    {
      local ciObject = null;

      local strItemName = i_strParamArray[1];
      local eObjectType = ::rumGetAssetID( strItemName );
      if( rumInvalidAssetID == eObjectType )
      {
        eObjectType = ::rumGetAssetID( strItemName + "_WidgetID" );
        if( rumInvalidAssetID == eObjectType )
        {
          eObjectType = ::rumGetAssetID( strItemName + "_CreatureID" );
          if( rumInvalidAssetID == eObjectType )
          {
            eObjectType = ::rumGetAssetID( strItemName + "_PortalID" );
          }
        }
      }

      if( eObjectType != rumInvalidAssetID )
      {
        local ciMap = i_ciPlayer.GetMap();
        local ciObject = ::rumCreate( eObjectType );
        if( ( ciObject instanceof Creature ) || ( ciObject instanceof Widget ) || ( ciObject instanceof Portal ) )
        {
          if( !ciMap.AddPawn( ciObject, i_ciPlayer.GetPosition() ) )
          {
            local strError = g_strColorTagArray.Red + "Failed to add object " + strItemName;
            CommandError( i_ciPlayer, strError );
          }
        }
        else if( ciObject instanceof Inventory )
        {
          if( !i_ciPlayer.AddItem( ciObject ) )
          {
            local strError = g_strColorTagArray.Red + "Failed to add object " + strItemName;
            CommandError( i_ciPlayer, strError );
          }
        }
      }
      else
      {
        local strError = g_strColorTagArray.Red + "Specified object not could not be found: " + strItemName;
        CommandError( i_ciPlayer, strError );
      }
    }
    else
    {
      bShowUsage = true;
      local strError = g_strColorTagArray.Red + "Not enough parameters.";
      CommandError( i_ciPlayer, strError );
    }

    if( bShowUsage )
    {
      local strError = g_strColorTagArray.Red + "Usage: /create <object name>.";
      CommandError( i_ciPlayer, strError );
    }
  }


  function AdminCommandGoto( i_ciPlayer, i_strParamArray )
  {
    if( i_strParamArray[1] == "map" )
    {
      if( i_strParamArray.len() == 5 )
      {
        local ciMap = i_ciPlayer.GetMap();

        local strMap = i_strParamArray[2];
        local eMapID = ::rumGetAssetID( strMap + "_MapID" );
        local ciDestMap = GetOrCreateMap( i_ciPlayer, eMapID );
        if( ciDestMap != null )
        {
          local eSrcVersion = ciMap.GetProperty( Ultima_Version_PropertyID, 0 );
          local eDestVersion = ciDestMap.GetProperty( Ultima_Version_PropertyID, 0 );
          if( eSrcVersion == eDestVersion )
          {
            local ciPos = rumPos( i_strParamArray[3].tointeger(), i_strParamArray[4].tointeger() );

            SendClientEffect( i_ciPlayer, ClientEffectType.Cast );

            local ciTransport = i_ciPlayer.GetTransport();
            if( ciTransport != null )
            {
              ciTransport.Portal( eMapID, ciPos );

              local ciCommander = ciTransport.GetCommander();
              if( ciCommander != null )
              {
                SendClientEffect( i_ciPlayer, ClientEffectType.Cast );
              }
            }
            else
            {
              ciMap.TransferPawn( i_ciPlayer, ciDestMap, ciPos );
              SendClientEffect( i_ciPlayer, ClientEffectType.Cast );
            }
          }
          else
          {
            local strError = g_strColorTagArray.Red +
                             "Can't teleport across Ultima boundaries. Execute '/ultima #' first.";
            CommandError( i_ciPlayer, strError );
          }
        }
        else
        {
          local strError = g_strColorTagArray.Red + "Map not found: " + g_strColorTagArray.White + strMap;
          CommandError( i_ciPlayer, strError );
        }
      }
      else
      {
        local strError = g_strColorTagArray.Red + "Invalid format for command: " + g_strColorTagArray.White +
                         i_strParamArray[0] + " " + i_strParamArray[1];
        CommandError( i_ciPlayer, strError );
      }
    }
    else if( i_strParamArray[1] == "player" )
    {
      if( i_strParamArray.len() == 3 )
      {
        local strName = i_strParamArray[2];
        local ciTargetPlayer = ::rumGetPlayerByName( strName );
        if( ciTargetPlayer != null )
        {
          local ciTargetMap = ciTargetPlayer.GetMap();
          if( ciTargetMap != null )
          {
            local ciMap = i_ciPlayer.GetMap();
            if( ciMap != null )
            {
              local eSrcVersion = ciMap.GetProperty( Ultima_Version_PropertyID, 0 );
              local eDestVersion = ciTargetMap.GetProperty( Ultima_Version_PropertyID, 0 );
              if( eSrcVersion == eDestVersion )
              {
                SendClientEffect( i_ciPlayer, ClientEffectType.Cast );

                if( ciMap.TransferPawn( i_ciPlayer, ciTargetMap, ciTargetPlayer.GetPosition() ) )
                {
                  SendClientEffect( i_ciPlayer, ClientEffectType.Cast );
                }
              }
              else
              {
                local strError = g_strColorTagArray.Red +
                                 "Can't teleport across Ultima boundaries. Execute /ultima <#> first.";
                CommandError( i_ciPlayer, strError );
              }
            }
          }
        }
        else
        {
          local strError = g_strColorTagArray.Red + "Player not found: " + g_strColorTagArray.White + strName;
          CommandError( i_ciPlayer, strError );
        }
      }
      else
      {
        local strError = g_strColorTagArray.Red + "Invalid format for command: " + g_strColorTagArray.White +
                         i_strParamArray[0] + " " + i_strParamArray[1];
        CommandError( i_ciPlayer, strError );
      }
    }
    else
    {
      local strError = g_strColorTagArray.Red + "Invalid format for command: " + g_strColorTagArray.White +
                       i_strParamArray[0] + " " + i_strParamArray[1];
      CommandError( i_ciPlayer, strError );
    }
  }


  function AdminCommandJackpot( i_ciPlayer )
  {
    SendClientEffect( i_ciPlayer, ClientEffectType.Cast );

    i_ciPlayer.Resurrect( ResurrectionType.Other );

    local eVersion = i_ciPlayer.GetProperty( Ultima_Version_PropertyID, GameType.Ultima4 );
    switch( eVersion )
    {
      case GameType.Ultima4:
        foreach( ePropertyID in g_eU4EquipmentPropertyArray )
        {
          i_ciPlayer.SetProperty( ePropertyID, 50 );
        }

        i_ciPlayer.SetProperty( U4_Horse_PropertyID, true );
        i_ciPlayer.SetProperty( U4_Last_Orb_Type_PropertyID, 0 );

        // Reagents
        foreach( ePropertyID in g_eU4ReagentPropertyArray )
        {
          local ciProperty = ::rumGetPropertyAsset( ePropertyID );
          if( ciProperty != null )
          {
            i_ciPlayer.SetProperty( ePropertyID, ciProperty.GetMaxValue() );
          }
        }

        // Add U4 spell mixtures
        foreach( eSpellType in g_eU4SpellArray )
        {
          local ciAsset = ::rumGetCustomAsset( eSpellType );
          if( ciAsset != null )
          {
            local ePropertyType = ciAsset.GetProperty( Spell_ID_PropertyID, rumInvalidAssetID );
            if( ePropertyType != rumInvalidAssetID )
            {
              i_ciPlayer.CapProperty( ePropertyType );
            }
          }
        }

        break;

      case GameType.Ultima3:
        i_ciPlayer.SetProperty( U3_Horse_PropertyID, true );

        foreach( ePropertyID in g_eU3EquipmentPropertyArray )
        {
          i_ciPlayer.SetProperty( ePropertyID, 50 );
        }
        break;

      case GameType.Ultima2:
        i_ciPlayer.SetProperty( U2_Horse_PropertyID, true );

        foreach( ePropertyID in g_eU2EquipmentPropertyArray )
        {
          i_ciPlayer.SetProperty( ePropertyID, 50 );
        }

        // Spells
        foreach( eSpellType in g_eU2SpellArray )
        {
          local ciAsset = ::rumGetCustomAsset( eSpellType );
          if( ciAsset != null )
          {
            local ePropertyType = ciAsset.GetProperty( Spell_ID_PropertyID, rumInvalidAssetID );
            if( ePropertyType != rumInvalidAssetID )
            {
              i_ciPlayer.CapProperty( ePropertyType );
            }
          }
        }

        break;

      case GameType.Ultima1:
        i_ciPlayer.SetProperty( U1_Horse_PropertyID, true );

        foreach( ePropertyID in g_eU1EquipmentPropertyArray )
        {
          i_ciPlayer.SetProperty( ePropertyID, 50 );
        }

        i_ciPlayer.SetProperty( U1_Shuttle_Pass_PropertyID, true );

        // Spells
        foreach( eSpellType in g_eU1SpellArray )
        {
          local ciAsset = ::rumGetCustomAsset( eSpellType );
          if( ciAsset != null )
          {
            local ePropertyType = ciAsset.GetProperty( Spell_ID_PropertyID, rumInvalidAssetID );
            if( ePropertyType != rumInvalidAssetID )
            {
              i_ciPlayer.CapProperty( ePropertyType );
            }
          }
        }

        break;
    }

    i_ciPlayer.SetVersionedProperty( g_eHitpointsPropertyVersionArray, i_ciPlayer.GetMaxHitpoints() );
    i_ciPlayer.SetVersionedProperty( g_eManaPropertyVersionArray, i_ciPlayer.GetMaxMana() );
    i_ciPlayer.SetVersionedProperty( g_eGoldPropertyVersionArray,
                                     ::rumGetMaxPropertyValue( g_eGoldPropertyVersionArray[eVersion] ) );
    i_ciPlayer.SetVersionedProperty( g_eFoodPropertyVersionArray,
                                     ::rumGetMaxPropertyValue( g_eFoodPropertyVersionArray[eVersion] ) );
    i_ciPlayer.SetVersionedProperty( g_eSpiritBoundPropertyVersionArray, true );
  }


  function AdminCommandJail( i_ciPlayer, i_strName, i_iNotoriety )
  {
    local ciTargetPlayer = ::rumGetPlayerByName( i_strName );
    if( ciTargetPlayer != null )
    {
      // Set the player's notoriety to the highest of either their current notoriety, or the passed in amount
      local iCurrentNotoriety = ciTargetPlayer.GetVersionedProperty( g_eNotorietyPropertyVersionArray );
      if( i_iNotoriety > iCurrentNotoriety );
      {
        ciTargetPlayer.SetVersionedProperty( g_eNotorietyPropertyVersionArray, i_iNotoriety );
      }

      ciTargetPlayer.Jail();
    }
  }


  function AdminCommandLocate( i_ciPlayer, i_strName )
  {
    local ciTarget = i_ciPlayer;

    if( i_strName != null )
    {
      local ciTarget = ::rumGetPlayerByName( i_strName );
      if( null == ciTarget )
      {
        local strError = g_strColorTagArray.Red + "Could not find player: " + g_strColorTagArray.White + i_strName;
        CommandError( i_ciPlayer, strError );
        return;
      }
    }
    else
    {
      i_strName = i_ciPlayer.GetPlayerName();
    }

    local ciMap = ciTarget.GetMap();
    if( null == ciMap )
    {
      local strError = g_strColorTagArray.Red + "Could not find player map";
      CommandError( i_ciPlayer, strError );
      return;
    }

    local ciPos = ciTarget.GetPosition();
    local strResult = format( "Location of %s%s%s: map %s %d %d",
                              g_strColorTagArray.Blue, i_strName, g_strColorTagArray.White, ciMap.GetName(),
                              ciPos.x, ciPos.y );

    local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strResult );
    i_ciPlayer.SendBroadcast( ciBroadcast );
  }


  function AdminCommandProperty( i_ciPlayer, i_strParamArray )
  {
    local bInvalidFormat = false;
    local bInvalidTarget = false;
    local bInvalidProperty = false;

    if( i_strParamArray.len() >= 5 )
    {
      if( i_strParamArray[1].tolower() == "get" )
      {
        if( i_strParamArray[2].tolower() == "player" )
        {
          // TODO - This doesn't have to go to the server if it's shared or client only

          // Does the specified player exist?
          local strName = i_strParamArray[3];
          local ciTargetPlayer = ::rumGetPlayerByName( strName );
          if( ciTargetPlayer )
          {
            local strProperty = i_strParamArray[4];
            local ePropertyID = ::rumGetAssetID( strProperty + "_PropertyID" );
            local ciProperty = ::rumGetPropertyAsset( ePropertyID );
            if( ciProperty != null )
            {
              local value = ciTargetPlayer.GetProperty( ePropertyID, null );
              local strResult = g_strColorTagArray.Blue + strName + " " + g_strColorTagArray.Green + strProperty +
                                g_strColorTagArray.White + ": ";

              if( value != null )
              {
                strResult += value.tostring();
              }
              else
              {
                strResult += "null";
              }

              local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strResult );
              i_ciPlayer.SendBroadcast( ciBroadcast );
            }
            else
            {
              bInvalidProperty = true;
            }
          }
          else
          {
            bInvalidTarget = true;
          }
        }
        else
        {
          // Map and Inventory properties
          print( "Not implemented\n" );
        }
      }
      else
      {
        if( i_strParamArray.len() >= 6 )
        {
          // Set property
          if( i_strParamArray[2].tolower() == "player" )
          {
            // Does the specified player exist?
            local strName = i_strParamArray[3];
            local ciTargetPlayer = ::rumGetPlayerByName( strName );
            if( ciTargetPlayer )
            {
              local strProperty = i_strParamArray[4];
              local ePropertyID = ::rumGetAssetID( strProperty + "_PropertyID" );
              local ciProperty = ::rumGetPropertyAsset( ePropertyID );
              if( ciProperty != null )
              {
                local value = i_strParamArray[5];
                local eType = ciProperty.GetValueType();
                switch( eType )
                {
                  case rumIntegerPropertyValueType:
                    value = value.tointeger();
                    break;

                  case rumBoolPropertyValueType:
                    value = value.tointeger() ? true : false;
                    break;

                  case rumFloatPropertyValueType:
                    value = value.tofloat();
                    break;
                }

                ciTargetPlayer.SetProperty( ePropertyID, value );
              }
              else
              {
                bInvalidProperty = true;
              }
            }
            else
            {
              bInvalidTarget = true;
            }
          }
          else
          {
            // Map and Inventory properties
            print( "Not implemented\n" );
          }
        }
        else
        {
          bInvalidFormat = true;
        }
      }
    }
    else
    {
      bInvalidFormat = true;
    }

    if( bInvalidProperty )
    {
      local strError = g_strColorTagArray.Red + "Unknown property: " + g_strColorTagArray.White + i_strParamArray[4];
      CommandError( i_ciPlayer, strError );
    }
    else if( bInvalidTarget )
    {
      local strError = g_strColorTagArray.Red + "Invalid target: " + g_strColorTagArray.White + i_strParamArray[3];
      CommandError( i_ciPlayer, strError );
    }
    else if( bInvalidFormat )
    {
      local strError = g_strColorTagArray.Red + "Invalid format for command: " + g_strColorTagArray.White +
                       i_strParamArray[0];
      CommandError( i_ciPlayer, strError );
    }
  }


  function AdminCommandSummon( i_ciPlayer, i_strName )
  {
    local ciTargetPlayer = ::rumGetPlayerByName( i_strName );
    if( ciTargetPlayer != null )
    {
      local ciCasterMap = i_ciPlayer.GetMap();
      if( ciCasterMap != null )
      {
        SendClientEffect( i_ciPlayer, ClientEffectType.Cast );

        local ciMap = ciTargetPlayer.GetMap();
        if( ciMap != null )
        {
          SendClientEffect( ciTargetPlayer, ClientEffectType.Cast );

          // TODO - Verify with player that it's okay to be summoned?
          ciMap.TransferPawn( ciTargetPlayer, ciCasterMap, i_ciPlayer.GetPosition() );

          SendClientEffect( ciTargetPlayer, ClientEffectType.Cast );
        }
      }
    }
  }


  function AdminCommandTest( i_ciPlayer, i_strVersion )
  {
    local eVersion = i_strVersion.tointeger();
    eVersion = clamp( eVersion, GameType.Ultima1, GameType.Ultima4 );

    switch( eVersion )
    {
      case GameType.Ultima4: g_ciGameTest = ::U4_GameTest( i_ciPlayer ); break;
      case GameType.Ultima3: g_ciGameTest = ::U3_GameTest( i_ciPlayer ); break;
      case GameType.Ultima2: g_ciGameTest = ::U2_GameTest( i_ciPlayer ); break;
      case GameType.Ultima1: g_ciGameTest = ::U1_GameTest( i_ciPlayer ); break;
    }
  }


  function AdminCommandUltima( i_ciPlayer, i_strVersion )
  {
    local eVersion = i_strVersion.tointeger();
    eVersion = clamp( eVersion, GameType.Ultima1, GameType.Ultima4 );

    i_ciPlayer.ChangeWorld( eVersion );
  }


  function PlayerCommandGlobalSay( i_ciPlayer, i_strSay )
  {
    local strResult = g_strColorTagArray.Blue + i_ciPlayer.GetPlayerName() + ":" + g_strColorTagArray.White + i_strSay;

    local ciPlayerArray = ::rumGetAllPlayers();
    foreach( i, ciTarget in ciPlayerArray )
    {
      local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strResult );
      ciTarget.SendBroadcast( ciBroadcast );
    }

    // TODO: Charge gold for global chat?
  }


  function PlayerCommandMapSay( i_ciPlayer, i_strSay )
  {
    local ciMap = i_ciPlayer.GetMap();
    if( ciMap )
    {
      local strResult = g_strColorTagArray.Blue + i_ciPlayer.GetPlayerName() + ":" + g_strColorTagArray.White +
                        i_strSay;

      local ciPlayerArray = ciMap.GetAllPlayers();
      foreach( ciTarget in ciPlayerArray )
      {
        local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strResult );
        ciTarget.SendBroadcast( ciBroadcast );
      }
    }
  }


  function PlayerCommandParty( i_ciPlayer, i_strParamArray )
  {
    if( i_strParamArray.len() < 2 )
    {
      local strError = g_strColorTagArray.Red + "Invalid format for command: " + g_strColorTagArray.White +
                       i_strParamArray[0];
      CommandError( i_ciPlayer, strError );
      return;
    }

    local ciParty = i_ciPlayer.GetParty();
    local uiPlayerID = i_ciPlayer.GetID();

    switch( i_strParamArray[1] )
    {
      case "invite":
        if( i_ciPlayer.GetPlayerName().tolower() == i_strParamArray[2].tolower() )
        {
          // Ignore any attempts for players to invite themselves to a party
          return;
        }

        local ciMap = i_ciPlayer.GetMap();
        local bSoloInstance = ciMap.GetProperty( Map_Solo_Instance_PropertyID, false );
        if( bSoloInstance )
        {
          i_ciPlayer.ActionFailed( msg_party_unavailable_solo_client_StringID );
          return;
        }

        if( ciParty == null )
        {
          // Create a new party and add the initial member
          ciParty = Party();
          ciParty.Add( i_ciPlayer, true /* force since this wasn't an invite */ );
        }

        if( ciParty != null && i_strParamArray.len() > 2 )
        {
          if( uiPlayerID == ciParty.m_uiLeaderID )
          {
            ciParty.Invite( i_strParamArray[2] );
          }
          else
          {
            i_ciPlayer.ActionFailed( msg_party_not_leader_client_StringID );
          }
        }
        break;

      case "boot":
      case "dismiss":
        if( ciParty != null && i_strParamArray.len() > 2 )
        {
          if( uiPlayerID == ciParty.m_uiLeaderID )
          {
            local ciMember = ::rumGetPlayerByName( i_strParamArray[2] );
            ciParty.Dismiss( ciMember, PartyBroadcastType.PlayerDismissed );
          }
          else
          {
            i_ciPlayer.ActionFailed( msg_party_not_leader_client_StringID );
          }
        }
        break;

      case "disband":
        if( ciParty != null )
        {
          if( uiPlayerID == ciParty.m_uiLeaderID )
          {
            ciParty.Disband();
          }
          else
          {
            i_ciPlayer.ActionFailed( msg_party_not_leader_client_StringID );
          }
        }
        break;

      case "leave":
        if( ciParty != null )
        {
          ciParty.Dismiss( i_ciPlayer, PartyBroadcastType.PlayerLeft );
        }
        break;

      case "promote":
        if( ciParty != null && i_strParamArray.len() > 2 )
        {
          if( uiPlayerID == ciParty.m_uiLeaderID )
          {
            ciParty.Promote( i_strParamArray[2] );
          }
          else
          {
            i_ciPlayer.ActionFailed( msg_party_not_leader_client_StringID );
          }
        }
        break;
    }
  }


  function PlayerCommandPartySay( i_ciPlayer, i_strSay )
  {
    local ciParty = i_ciPlayer.GetParty();
    if( ciParty != null )
    {
      local strMessage = g_strColorTagArray.Blue + i_ciPlayer.GetPlayerName() + ":" + g_strColorTagArray.White +
                         i_strSay;

      // Inform each player
      foreach( uiMemberID in ciParty.m_uiRosterTable )
      {
        local ciMember = ::rumFetchPawn( uiMemberID );
        if( ciMember != null && ( ciMember instanceof Player ) )
        {
          local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strMessage );
          ciMember.SendBroadcast( ciBroadcast );
        }
      }
    }
  }


  function PlayerCommandSay( i_ciPlayer, i_strSay )
  {
    local ciMap = i_ciPlayer.GetMap();
    if( ciMap )
    {
      if( ( ciMap.GetAssetID() == U3_Sosaria_MapID ) && ( i_strSay.tolower() == "evocare" ) )
      {
        // Directly under the serpent?
        local ciPos = i_ciPlayer.GetPosition() + GetDirectionVector( Direction.North );

        local ciPosData = ciMap.GetPositionData( ciPos );
        local ciSerpent = ciPosData.GetNext( rumWidgetPawnType, U3_Serpent_Head_WidgetID );
        if( ciSerpent != null )
        {
          // Transfer the transport two positions north
          local ciTransport = i_ciPlayer.GetTransport();
          if( ciTransport != null )
          {
            local iMarkCount = 0;

            // The transport can only move if all passengers bear the Mark of the Snake
            local uiPassengerTable = ciTransport.GetPassengers();
            if( uiPassengerTable != null )
            {
              local strMsg = g_strColorTagArray.Blue + i_ciPlayer.GetPlayerName() + ":" +
                             g_strColorTagArray.White + "EVOCARE!!!";

              foreach( uiPlayerID in uiPassengerTable )
              {
                local ciPlayer = ::rumFetchPawn( uiPlayerID );
                if( ciPlayer != null )
                {
                  local iFlags = ciPlayer.GetProperty( U3_Marks_PropertyID, 0 );
                  if( ::rumBitOn( iFlags, U3_MarkType.Snake ) )
                  {
                    ++iMarkCount;
                  }

                  local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strMsg );
                  ciPlayer.SendBroadcast( ciBroadcast );
                }
              }

              if( ciTransport.GetNumPassengers() == iMarkCount )
              {
                SendClientEffect( ciTransport, ClientEffectType.ScreenShake );

                ciPos.y -= 2;
                ciTransport.Portal( U3_Sosaria_MapID, ciPos );
              }
              else
              {
                foreach( uiPlayerID in uiPassengerTable )
                {
                  local ciPlayer = ::rumFetchPawn( uiPlayerID );
                  if( ciPlayer != null )
                  {
                    ciPlayer.ActionFailed( msg_strange_force_client_StringID );
                  }
                }
              }
            }
          }

          // Dialogue already sent privately to all players in the group
          return;
        }
      }

      local strResult = g_strColorTagArray.Blue + i_ciPlayer.GetPlayerName() + ":" + g_strColorTagArray.White +
                        i_strSay;

      // Get all players within earshot of the player (basically, are they on-screen?)
      local iLOSRadius = 5;
      local ciPlayerArray = ciMap.GetPlayers( i_ciPlayer.GetPosition(), iLOSRadius, false );
      foreach( ciTarget in ciPlayerArray )
      {
        local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strResult );
        ciTarget.SendBroadcast( ciBroadcast );
      }
    }
  }


  function PlayerCommandTell( i_ciPlayer, i_strParamArray )
  {
    local strPlayer = i_strParamArray[1];
    local ciTargetPlayer = ::rumGetPlayerByName( strPlayer );
    if( ciTargetPlayer instanceof Player )
    {
      // Send tell to indicated player
      local strMsg = var.slice( i_strParamArray[0].len() + i_strParamArray[1].len() + 2 );
      local strResult = format( "%s%s %s: %s%s", g_strColorTagArray.Blue, i_ciPlayer.GetPlayerName(),
                                ::rumGetString( _chat_player_tells_you_server_StringID, ciTargetPlayer.m_iLanguageID ),
                                g_strColorTagArray.White, strMsg );

      local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strResult );
      ciTargetPlayer.SendBroadcast( ciBroadcast );

      // Send tell to sender
      strResult = format( "%s%s %s: %s%s", g_strColorTagArray.Blue,
                          ::rumGetString( _chat_player_you_tell_server_StringID, i_ciPlayer.m_iLanguageID ),
                          ciTargetPlayer.GetPlayerName(),
                          g_strColorTagArray.White, strMsg );

      local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strResult );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
    else
    {
      // Couldn't find recipient
      local strResult = format( "%s%s: %s", g_strColorTagArray.Red,
                                ::rumGetString( _chat_recipient_not_found_server_StringID, i_ciPlayer.m_iLanguageID ),
                                strPlayer );

      local ciBroadcast = ::rumCreate( Player_Chat_BroadcastID, strResult );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
  }


  function PlayerCommandStuck( i_ciPlayer, strParamArray )
  {
    // For now just kill and resurrect the player from the void
    i_ciPlayer.Kill( null );
    i_ciPlayer.Resurrect( ResurrectionType.Void );
  }
}
