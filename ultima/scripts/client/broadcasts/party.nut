g_ciPartyKeepAlive <- null;

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
      SetType( vargv[0] );

      if( iLen > 1 )
      {
        SetPartyID( vargv[1] );
      }
    }
  }


  function GetType()
  {
    return var1;
  }


  function GetMessage()
  {
    return var2;
  }


  function GetRoster()
  {
    return var2;
  }


  function GetPlayerString()
  {
    return var3;
  }


  function GetLeaderID()
  {
    return var3;
  }


  function GetPlayerID()
  {
    return var3;
  }


  function GetPartyID()
  {
    return var5;
  }


  function SetType( i_eType )
  {
    var1 = i_eType;
  }


  function SetPartyID( i_iPartyID )
  {
    var5 = i_iPartyID;
  }


  function OnRecv()
  {
    local eType = GetType();

    switch( eType )
    {
      case PartyBroadcastType.Invitation:
        local ciPlayer = ::rumGetMainPlayer();
        local ciMap = ciPlayer.GetMap();
        if( g_ciPartyKeepAlive != null )
        {
            // The player already has an invitation
            local ciBroadcast = ::rumCreate( Party_BroadcastID, PartyBroadcastType.InvitationInvalid,
                                             GetPartyID() );
            ::rumSendBroadcast( ciBroadcast );
        }
        else
        {
          local bSoloInstance = ciMap.GetProperty( Map_Solo_Instance_PropertyID, false );
          if( !bSoloInstance )
          {
            local ciInviter = ::rumFetchPawn( GetPlayerID() );
            if( ciInviter instanceof Player )
            {
              PartyMessage( ciInviter.GetPlayerName() );
              PartyInvitation( ciInviter );
            }
          }
          else
          {
            // The player cannot join a party from within a solo instance
            local ciBroadcast = ::rumCreate( Party_BroadcastID, PartyBroadcastType.InvitationInvalid,
                                             GetPartyID() );
            ::rumSendBroadcast( ciBroadcast );
          }
        }
        break;

      case PartyBroadcastType.InvitationExpired:
        ShowString( ::rumGetString( msg_party_invitation_expired_client_StringID ), g_strColorTagArray.Yellow );
        Ultima_ListSelectionEnd();
        g_ciPartyKeepAlive = null;
        break;

      case PartyBroadcastType.PlayerDismissed:
      case PartyBroadcastType.PlayerLeft:
        PlayerDismissed( eType );
        break;

      case PartyBroadcastType.PlayerJoined:
        PlayerJoined();
        break;

      case PartyBroadcastType.RosterUpdate:
        PartyRosterUpdate();
        break;

      case PartyBroadcastType.NewLeader:
        NewLeader();
        break;

      case PartyBroadcastType.IDMessage:
        local ciPlayer = ::rumFetchPawn( GetPlayerID() );
        if( ciPlayer instanceof Player )
        {
          PartyMessage( ciPlayer.GetPlayerName() );
        }
        break;

      case PartyBroadcastType.StringMessage:
        PartyMessage( GetPlayerString() );
        break;
    }
  }


  function NewLeader()
  {
    local uiPlayerID = GetPlayerID();
    local ciLeader = ::rumFetchPawn( uiPlayerID );
    if( ciLeader instanceof Player )
    {
      g_ciCUO.m_uiPartyLeaderID = uiPlayerID;

      local ciPlayer = ::rumGetMainPlayer();
      if( ciPlayer instanceof Player )
      {
        if( ciPlayer == ciLeader )
        {
          ShowString( ::rumGetString( msg_party_leader_client_StringID ), g_strColorTagArray.Yellow );
        }
        else
        {
          local strMessage = format( ::rumGetString( msg_party_new_leader_client_StringID ),
                                     ciLeader.GetPlayerName() );
          ShowString( strMessage, g_strColorTagArray.Yellow );
        }
      }
    }

    UpdatePartyStat();
  }


  function PartyInvitation( i_ciPlayer )
  {
    g_ciUI.m_ciYesNoListView.Clear();
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                       rumKeypress.KeyY() );
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                       rumKeypress.KeyN() );

    g_ciUI.m_ciYesNoListView.m_funcAccept = PartyInvitationCallback.bindenv( this );
    g_ciUI.m_ciYesNoListView.m_funcCancel = PartyInvitationCallbackCanceled.bindenv( this );
    g_ciUI.m_ciYesNoListView.SetActive( true );
    g_ciUI.m_ciYesNoListView.Focus();

    g_ciPartyKeepAlive = this;
  }


  function PartyInvitationCallback()
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();
    if( YesNoResponse.Yes == eResponse )
    {
      local ciBroadcast = ::rumCreate( Party_BroadcastID, PartyBroadcastType.InvitationAccepted, GetPartyID() );
      ::rumSendBroadcast( ciBroadcast );
    }
    else if( YesNoResponse.No == eResponse )
    {
      local ciBroadcast = ::rumCreate( Party_BroadcastID, PartyBroadcastType.InvitationDeclined, GetPartyID() );
      ::rumSendBroadcast( ciBroadcast );
    }

    Ultima_ListSelectionEnd();

    g_ciPartyKeepAlive = null;
  }


  function PartyInvitationCallbackCanceled()
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    local ciBroadcast = ::rumCreate( Party_BroadcastID, PartyBroadcastType.InvitationDeclined, GetPartyID() );
    ::rumSendBroadcast( ciBroadcast );

    Ultima_ListSelectionEnd();

    g_ciPartyKeepAlive = null;
  }


  function PartyMessage( i_strPlayer )
  {
    local strMessage = format( ::rumGetString( GetMessage() ), i_strPlayer );
    ShowString( strMessage, g_strColorTagArray.Yellow );
  }


  function PartyRosterUpdate()
  {
    local uiLeaderID = GetLeaderID();
    local uiRosterTable = GetRoster();

    g_ciCUO.m_uiPartyIDTable = uiRosterTable;
    g_ciCUO.m_uiPartyLeaderID = uiLeaderID;
  }


  function PlayerDismissed( i_eType )
  {
    local ciMember = ::rumFetchPawn( GetPlayerID() );
    if( ciMember instanceof Player )
    {
      local uiMemberID = ciMember.GetID;

      local ciPlayer = ::rumGetMainPlayer();
      if( ciPlayer != ciMember )
      {
        // Remove the dismissed player from the party
        if( uiMemberID in g_ciCUO.m_uiPartyIDTable )
        {
          delete g_ciCUO.m_uiPartyIDTable[uiMemberID];
        }

        switch( i_eType )
        {
          case PartyBroadcastType.PlayerDismissed:
            local strMesssage = format( ::rumGetString( msg_party_player_dismissed_client_StringID ),
                                        ciMember.GetPlayerName() );
            ShowString( strMesssage, g_strColorTagArray.Yellow );
            break;

          case PartyBroadcastType.PlayerLeft:
            local strMesssage = format( ::rumGetString( msg_party_player_left_client_StringID ),
                                        ciMember.GetPlayerName() );
            ShowString( strMesssage, g_strColorTagArray.Yellow );
            break;
        }

        UpdatePartyStat();
      }
      else
      {
        // Delete the party info
        g_ciCUO.m_uiPartyIDTable = null;
        g_ciCUO.m_uiPartyLeaderID = rumInvalidGameID;

        switch( i_eType )
        {
          case PartyBroadcastType.PlayerDismissed:
            ShowString( ::rumGetString( msg_party_dismissed_client_StringID ), g_strColorTagArray.Yellow );
            break;

          case PartyBroadcastType.PlayerLeft:
            ShowString( ::rumGetString( msg_party_left_client_StringID ), g_strColorTagArray.Yellow );
            break;
        }

        switch( g_ciCUO.m_eVersion )
        {
          case GameType.Ultima1: g_ciUI.m_eCurrentStatPage = U1_StatPage.Main; break;
          case GameType.Ultima2: g_ciUI.m_eCurrentStatPage = U2_StatPage.Main; break;
          case GameType.Ultima3: g_ciUI.m_eCurrentStatPage = U3_StatPage.Main; break;
          case GameType.Ultima4: g_ciUI.m_eCurrentStatPage = U4_StatPage.Main; break;
        }

        Ultima_Stat_Update();
      }
    }
  }


  function PlayerJoined()
  {
    local ciMember = ::rumFetchPawn( GetPlayerID() );
    if( ciMember instanceof Player )
    {
      local ciPlayer = ::rumGetMainPlayer();
      if( ciPlayer != ciMember )
      {
        local strMesssage = format( ::rumGetString( msg_party_joined_client_StringID ), ciMember.GetPlayerName() );
        ShowString( strMesssage, g_strColorTagArray.Yellow );

        local uiMemberID = ciMember.GetID();
        g_ciCUO.m_uiPartyIDTable[uiMemberID] <- uiMemberID;

        UpdatePartyStat();
      }
    }
  }


  function UpdatePartyStat()
  {
    switch( g_ciCUO.m_eVersion )
    {
      case GameType.Ultima1: if( g_ciUI.m_eCurrentStatPage == U1_StatPage.Party ) U1_Stat_Party(); break;
      case GameType.Ultima2: if( g_ciUI.m_eCurrentStatPage == U2_StatPage.Party ) U2_Stat_Party(); break;
      case GameType.Ultima3: if( g_ciUI.m_eCurrentStatPage == U3_StatPage.Party ) U3_Stat_Party(); break;
      case GameType.Ultima4: if( g_ciUI.m_eCurrentStatPage == U4_StatPage.Party ) U4_Stat_Party(); break;
    }
  }
}
