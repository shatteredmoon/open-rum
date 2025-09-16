g_strAdminCommandsArray <-
[
  // TODO: "/ban"
  "/create"
  "/goto"
  "/jackpot"
  "/jail"
  "/loc", "/locate"
  "/property"
  // TODO: "/silence"
  "/summon"
  "/teleport"
  "/test"
  "/ultima"
];

g_strPlayerCommandsArray <-
[
  "/g", "/global",
  "/help",
  // TODO: "/ignore",
  "/m", "/map",
  "/p", "/party",
  "/password",
  "/s", "/say",
  "/stuck",
  "/t", "/tell",
  "/who"
];


// Sent from client when player enters a chat message or text command
// Received from server with chat message or command result string
class Player_Chat_Broadcast extends rumBroadcast
{
  var = 0;

  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var = vargv[0]; // Chat message or command result string
    }
  }

  function OnRecv()
  {
    g_ciUI.m_ciChatTextView.PushText( "<G#Prompt:vcenter>" + var );
  }


  function Validate()
  {
    local bValid = false;
    local strTokenArray = split( var, " " );
    local strToken = strTokenArray[0];

    // If this is a player or admin command, check to make sure the command exists and is executable by the player
    if( "/" == strToken.slice( 0, 1 ) )
    {
      // TODO - this can easily be exploited by hackers setting this property on themselves, however, they should
      // only gain access to the /list and /locate commands which will not gain them a particular edge over other
      // players at this time.
      local ciPlayer = ::rumGetMainPlayer();
      local bAdmin = ciPlayer.IsAdmin();

      // General commands
      if( ValueInContainer( strToken, g_strPlayerCommandsArray ) )
      {
        bValid = true;
      }
      else if( bAdmin && ValueInContainer( strToken, g_strAdminCommandsArray ) )
      {
        bValid = true;
      }
      else
      {
        ShowString( ::rumGetString( msg_unknown_command_client_StringID ) + ": " + strToken, g_strColorTagArray.Red );
      }
    }
    else
    {
      bValid = true;
    }

    return bValid;
  }
}
