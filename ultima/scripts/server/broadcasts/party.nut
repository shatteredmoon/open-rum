class Party_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;
  var3 = 0;
  var4 = 0;
  var5 = 0;

  constructor( ... )
  {
    base.constructor();

    local iLen = vargv.len();
    if( iLen > 0 )
    {
      local eType = vargv[0];
      SetType( eType );

      if( iLen > 1 )
      {
        if( eType == PartyBroadcastType.RosterUpdate )
        {
          SetRoster( vargv[1], vargv[2] );
        }
        else if( ( eType == PartyBroadcastType.PlayerJoined ) || ( eType == PartyBroadcastType.NewLeader ) ||
                 ( eType == PartyBroadcastType.PlayerDismissed ) || ( eType == PartyBroadcastType.PlayerLeft ) )
        {
          SetPlayerID( vargv[1] );
        }
        else
        {
          SetMessage( vargv[1] );

          if( iLen > 2 )
          {
            switch( eType )
            {
              case PartyBroadcastType.Invitation:
                SetPlayerID( vargv[2] );
                SetPartyID( vargv[3] );
                break;

              case PartyBroadcastType.IDMessage:
                SetPlayerID( vargv[2] );
                break;

              case PartyBroadcastType.StringMessage:
                SetPlayerString( vargv[2] );
                break;
            }
          }
        }
      }
    }
  }


  function GetType()
  {
    return var1;
  }


  function GetPartyID()
  {
    return var5;
  }


  function SetType( i_eType )
  {
    var1 = i_eType;
  }


  function SetMessage( i_strMessage )
  {
    var2 = i_strMessage;
  }


  function SetRoster( i_uiRosterTable, i_uiLeaderID )
  {
    var2 = i_uiRosterTable;
    var3 = i_uiLeaderID;
  }


  function SetPlayerString( i_strPlayer )
  {
    var3 = i_strPlayer;
  }


  function SetPlayerID( i_uiPlayerID )
  {
    var3 = i_uiPlayerID;
  }


  function SetPartyID( i_uiPartyID )
  {
    var5 = i_uiPartyID;
  }


  function InvitationResponse( i_eType, i_ciPlayer )
  {
    local uiPartyID = GetPartyID();
    if( uiPartyID in g_ciServer.m_uiPartyTable )
    {
      local ciParty = g_ciServer.m_uiPartyTable[uiPartyID];
      if( ciParty instanceof Party )
      {
        if( PartyBroadcastType.InvitationAccepted == i_eType )
        {
          ciParty.InvitationAccepted( i_ciPlayer );
        }
        else if( PartyBroadcastType.InvitationDeclined == i_eType )
        {
          ciParty.InvitationDeclined( i_ciPlayer );
        }
        else if( PartyBroadcastType.InvitationInvalid == i_eType )
        {
          ciParty.InvitationInvalid( i_ciPlayer );
        }
      }
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( i_ciPlayer instanceof Player )
    {
      local eType = GetType();
      switch( eType )
      {
        case PartyBroadcastType.InvitationAccepted:
        case PartyBroadcastType.InvitationDeclined:
        case PartyBroadcastType.InvitationInvalid:
          InvitationResponse( eType, i_ciPlayer );
          break;

        default:
          // Received message of an unimplemented type
          i_ciPlayer.IncrementHackAttempts();
          break;
      }

      i_ciPlayer.PopPacket();
    }
  }
}
