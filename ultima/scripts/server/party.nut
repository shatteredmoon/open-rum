class Party
{
  m_uiLeaderID = rumInvalidGameID;
  m_uiPartyID = 0;
  m_uiRosterTable = null;
  m_uiInvitationTable = null;

  static s_fInvitationExpireTime = 15.0;
  static s_iMaxMemberCount = 8;


  constructor()
  {
    m_uiRosterTable = {};
    m_uiInvitationTable = {};

    m_uiPartyID = ++g_ciServer.m_uiPartyIndex;
    g_ciServer.m_uiPartyTable[m_uiPartyID] <- this;
  }


  function Add( i_ciPlayer, i_bForce = false )
  {
    if( i_ciPlayer != null && ( i_ciPlayer instanceof Player ) )
    {
      if( NumMembers() >= s_iMaxMemberCount && !i_bForce )
      {
        m_uiLeaderID.ActionFailed( msg_party_full_client_StringID );
        i_ciPlayer.ActionFailed( msg_party_full_client_StringID );
        return;
      }

      if( !IsMember( i_ciPlayer ) )
      {
        local uiPlayerID = i_ciPlayer.GetID();
        local bInvited = ( uiPlayerID in m_uiInvitationTable );
        if( bInvited || i_bForce )
        {
          // Remove the new member from the invitation table
          if( bInvited )
          {
            delete m_uiInvitationTable[uiPlayerID];
          }

          // Add the new member to the group
          m_uiRosterTable[uiPlayerID] <- uiPlayerID;
          i_ciPlayer.m_uiPartyID = m_uiPartyID;

          if( rumInvalidGameID == m_uiLeaderID )
          {
            SetLeader( i_ciPlayer );
          }

          // Send a notification to all current party members
          foreach( uiMemberID in m_uiRosterTable )
          {
            local ciMember = ::rumFetchPawn( uiMemberID );
            if( ciMember != null && ( ciMember instanceof Player ) )
            {
              local ciBroadcast = ::rumCreate( Party_BroadcastID, PartyBroadcastType.PlayerJoined, uiPlayerID );
              ciMember.SendBroadcast( ciBroadcast );
            }
          }

          // Send all current member info to the new member
          local ciBroadcast = ::rumCreate( Party_BroadcastID, PartyBroadcastType.RosterUpdate, m_uiRosterTable,
                                           m_uiLeaderID );
          i_ciPlayer.SendBroadcast( ciBroadcast );
        }
      }
    }
  }


  function Disband()
  {
    // Inform each player
    foreach( uiMemberID in m_uiRosterTable )
    {
      local ciMember = ::rumFetchPawn( uiMemberID );
      if( ciMember != null && ( ciMember instanceof Player ) )
      {
        ciMember.ActionInfo( msg_party_disbanded_client_StringID );

        local ciBroadcast = ::rumCreate( Party_BroadcastID, PartyBroadcastType.RosterUpdate, null, rumInvalidGameID );
        ciMember.SendBroadcast( ciBroadcast );

        ciMember.m_uiPartyID = 0;
      }
    }

    m_uiLeaderID = rumInvalidGameID;
    m_uiRosterTable.clear();
    m_uiRosterTable = null;

    g_ciServer.m_uiPartyTable[m_uiPartyID] <- this;
    m_uiPartyID = 0;
  }


  function Dismiss( i_ciPlayer, i_eDismissalType )
  {
    if( ( i_ciPlayer == null ) || !( i_ciPlayer instanceof Player ) && !( i_ciPlayer.GetID() in m_uiRosterTable ) )
    {
      return;
    }

    local uiPlayerID = i_ciPlayer.GetID();
    local ciBroadcast = ::rumCreate( Party_BroadcastID, i_eDismissalType, uiPlayerID );
    i_ciPlayer.SendBroadcast( ciBroadcast );

    i_ciPlayer.m_uiPartyID = 0;
    delete m_uiRosterTable[uiPlayerID];

    // Find a new leader if necessary
    if( m_uiLeaderID == uiPlayerID )
    {
      m_uiLeaderID = rumInvalidGameID;

      if( NumMembers() > 0 )
      {
        foreach( uiMemberID in m_uiRosterTable )
        {
          local ciMember = ::rumFetchPawn( uiMemberID );
          if( ciMember != null && ( ciMember instanceof Player ) )
          {
            SetLeader( ciMember );
            break;
          }
        }
      }
    }

    if( 0 == m_uiLeaderID )
    {
      Disband();
    }
  }


  function GetPartyID()
  {
    return m_uiPartyID;
  }


  function HasMember( uiPlayerID )
  {
    return ( uiPlayerID in m_uiRosterTable );
  }


  function Invite( i_strPlayer )
  {
    local ciLeader = ::rumFetchPawn( m_uiLeaderID );
    if( ( ciLeader == null ) || !( ciLeader instanceof Player ) )
    {
      return;
    }

    if( NumMembers() >= s_iMaxMemberCount )
    {
      ciLeader.ActionFailed( msg_party_full_client_StringID );
      return;
    }

    local ciPlayer = ::rumGetPlayerByName( i_strPlayer );
    if( ciPlayer instanceof Player )
    {
      if( ciPlayer.GetPartyID() == 0 )
      {
        local uiPlayerID = ciPlayer.GetID();
        if( uiPlayerID in m_uiInvitationTable )
        {
          // This player already has an invitation
          local ciBroadcast = ::rumCreate( Party_BroadcastID,
                                           PartyBroadcastType.IDMessage,
                                           msg_party_invitation_sent_client_StringID,
                                           uiPlayerID );
          ciLeader.SendBroadcast( ciBroadcast );
        }
        else
        {
          // Send an invitation to the player
          local ciBroadcast = ::rumCreate( Party_BroadcastID,
                                           PartyBroadcastType.Invitation,
                                           msg_party_invitation_client_StringID,
                                           m_uiLeaderID,
                                           m_uiPartyID );
          ciPlayer.SendBroadcast( ciBroadcast );

          // Let the leader know the invitation was sent
          ciBroadcast = ::rumCreate( Party_BroadcastID,
                                     PartyBroadcastType.IDMessage,
                                     msg_party_invitation_sent_client_StringID,
                                     uiPlayerID );
          ciLeader.SendBroadcast( ciBroadcast );

          // Add the player to the invitation table
          m_uiInvitationTable[uiPlayerID] <- uiPlayerID;

          ::rumSchedule( this, InvitationExpiration, s_fInvitationExpireTime, uiPlayerID );
        }
      }
      else
      {
        // The target player is already in a party!
        local ciBroadcast = ::rumCreate( Party_BroadcastID,
                                         PartyBroadcastType.IDMessage,
                                         msg_party_already_grouped_client_StringID,
                                         ciPlayer.GetID() );
        ciLeader.SendBroadcast( ciBroadcast );
      }
    }
    else
    {
      // The target player could not be found - they either don't exist or are not online
      local ciBroadcast = ::rumCreate( Party_BroadcastID,
                                       PartyBroadcastType.StringMessage,
                                       msg_party_player_not_found_client_StringID,
                                       i_strPlayer );
      ciLeader.SendBroadcast( ciBroadcast );
    }
  }


  function InvitationAccepted( i_ciPlayer )
  {
    if( i_ciPlayer != null && ( i_ciPlayer instanceof Player ) )
    {
      // This call also deletes the existing invitation
      Add( i_ciPlayer );
    }
  }


  function InvitationDeclined( i_ciPlayer )
  {
    if( ( null == i_ciPlayer ) || !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local ciLeader = ::rumFetchPawn( m_uiLeaderID );
    if( ciLeader != null && ( ciLeader instanceof Player ) )
    {
      local uiPlayerID = i_ciPlayer.GetID();
      local ciBroadcast = ::rumCreate( Party_BroadcastID,
                                       PartyBroadcastType.IDMessage,
                                       msg_party_invitation_declined_client_StringID,
                                       uiPlayerID );
      ciLeader.SendBroadcast( ciBroadcast );

      // Forget the invitation
      delete m_uiInvitationTable[uiPlayerID];
    }
  }


  function InvitationInvalid( i_ciPlayer )
  {
    if( ( null == i_ciPlayer ) || !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local ciLeader = ::rumFetchPawn( m_uiLeaderID );
    if( ciLeader != null && ( ciLeader instanceof Player ) )
    {
      local uiPlayerID = i_ciPlayer.GetID();
      local ciBroadcast = ::rumCreate( Party_BroadcastID,
                                       PartyBroadcastType.IDMessage,
                                       msg_party_invitation_invalid_client_StringID,
                                       uiPlayerID );
      ciLeader.SendBroadcast( ciBroadcast );

      // Forget the invitation
      delete m_uiInvitationTable[uiPlayerID];
    }
  }


  function InvitationExpiration( i_uiPlayerID )
  {
    if( i_uiPlayerID in m_uiInvitationTable )
    {
      // Forget the invitation
      delete m_uiInvitationTable[i_uiPlayerID];

      local ciLeader = ::rumFetchPawn( m_uiLeaderID );
      if( ciLeader != null && ( ciLeader instanceof Player ) )
      {
        // Inform the leader that the invited player did not respond
        local ciBroadcast = ::rumCreate( Party_BroadcastID,
                                         PartyBroadcastType.IDMessage,
                                         msg_party_invitation_expired_leader_client_StringID,
                                         i_uiPlayerID );
        ciLeader.SendBroadcast( ciBroadcast );
      }

      local ciPlayer = ::rumFetchPawn( i_uiPlayerID );
      if( ciPlayer != null && ( ciPlayer instanceof Player ) )
      {
        // Removed the player's invitation prompt
        local ciBroadcast = ::rumCreate( Party_BroadcastID, PartyBroadcastType.InvitationExpired );
        ciPlayer.SendBroadcast( ciBroadcast );
      }
    }
  }


  function IsMember( i_ciPlayer )
  {
    if( i_ciPlayer != null && ( i_ciPlayer instanceof Player ) )
    {
      return ( i_ciPlayer.GetID() in m_uiRosterTable );
    }

    return false;
  }


  function NumMembers()
  {
    return m_uiRosterTable.len();
  }


  function Promote( i_strPlayer )
  {
    local ciPlayer = ::rumGetPlayerByName( i_strPlayer );
    if( ciPlayer != null && ( ciPlayer instanceof Player ) )
    {
      SetLeader( ciPlayer );
    }
  }


  function SetLeader( i_ciPlayer )
  {
    if( ( null == i_ciPlayer ) || !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local uiPlayerID = i_ciPlayer.GetID();
    if( m_uiLeaderID != uiPlayerID && IsMember( i_ciPlayer ) )
    {
      // Set leadership on the specified player
      m_uiLeaderID = uiPlayerID;

      // Send a notification to all current party members
      foreach( uiMemberID in m_uiRosterTable )
      {
        if( uiMemberID != m_uiLeaderID )
        {
          local ciMember = ::rumFetchPawn( uiMemberID );
          if( ciMember != null && ( ciMember instanceof Player ) )
          {
            local ciBroadcast = ::rumCreate( Party_BroadcastID, PartyBroadcastType.NewLeader, m_uiLeaderID );
            ciMember.SendBroadcast( ciBroadcast );
          }
        }
      }

      // Inform the leader
      i_ciPlayer.ActionInfo( msg_party_leader_client_StringID );
    }
  }
}
