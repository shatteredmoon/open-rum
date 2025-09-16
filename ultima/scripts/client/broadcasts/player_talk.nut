// Received from client when player attempts to talk to an NPC
// Sent to client with initial conversation info
class Player_Talk_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;
  var3 = 0;
  var4 = 0;


  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Direction
    }
  }


  function OnRecv()
  {
    if( GameType.Ultima4 == g_ciCUO.m_eVersion )
    {
      g_ciCUO.m_uiCurrentTalkID = var1;
      local ePronounType = var2;
      local strName = var3;
      local strNPCDesc = var4;

      local iPronounToken = U4_Pronoun_He_client_StringID;
      switch( ePronounType )
      {
        case PronounType.She:   iPronounToken = U4_Pronoun_She_client_StringID;       break;
        case PronounType.It:    iPronounToken = U4_Pronoun_It_client_StringID;        break;
        case PronounType.Child: iPronounToken = U4_Pronoun_The_Child_client_StringID; break;
      }

      // The key "pronoun" always reflects the last pronoun received
      g_ciCUO.m_strDialogueTable["pronoun"] <- ::rumGetString( iPronounToken );

      // Fixup the npc name
      local strDesc = ::rumGetString( talk_i_am_client_StringID );
      strName = g_ciCUO.m_strDialogueTable["pronoun"] + " " + strDesc + " " + strName;

      // Cache the npc's name
      local strKey = "name_" + g_ciCUO.m_uiCurrentTalkID;
      g_ciCUO.m_strDialogueTable[strKey] <- strName;

      // Cache the npc's description
      strKey = "look_" + g_ciCUO.m_uiCurrentTalkID;
      g_ciCUO.m_strDialogueTable[strKey] <- strNPCDesc;

      // The player is now in npc-conversation mode
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.NPC_Chat );

      // Hide msg prompts during conversation
      g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );

      if( 0 == g_ciCUO.m_uiCurrentTalkID )
      {
        // Talking to Lord British
      }
      else
      {
        // Show npc description
        strDesc = ::rumGetString( talk_you_meet_client_StringID );
        ShowString( format( "<b>%s %s", strDesc, strNPCDesc ), g_strColorTagArray.Cyan );

        // 25% chance of the npc saying their name
        if( rand() % 4 == 0 )
        {
          ShowString( format( "<b>%s", strName ), g_strColorTagArray.Cyan );
        }

        strDesc = format( "<b>%s:", ::rumGetString( talk_your_interest_client_StringID ) );
        ShowString( strDesc, g_strColorTagArray.Cyan );
      }
    }
    else
    {
      local strResponse = var1;
      if( strResponse != null && strResponse.len() > 0 )
      {
        ShowString( format( "<b>%s<b>", strResponse ), g_strColorTagArray.Cyan );
      }
      else
      {
        ShowString( ::rumGetString( msg_no_response_client_StringID ), g_strColorTagArray.Red );
      }
    }
  }
}


// Sent from client when player answers an NPCs yes or no question
class Player_Talk_Answer_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() == 2 )
    {
      var1 = vargv[0]; // Keyword
      var2 = vargv[1]; // Answer
    }
  }
}


// Sent from client when player ends npc dialogue
// Received from server when npc dialogue is ended
class Player_Talk_Bye_Broadcast extends rumBroadcast
{
  var = 0; // Dialogue termination type


  function OnRecv()
  {
    local eTerminationType = var;
    if( DialogueTerminationType.TurnsAway == eTerminationType )
    {
      // The NPC turned away!
      ShowString( format( "%s %s<b>",
                          g_ciCUO.m_strDialogueTable["pronoun"],
                          ::rumGetString( talk_turns_away_client_StringID ) ),
                          g_strColorTagArray.Cyan );
    }

    Ultima_ListSelectionEnd();

    g_ciUI.m_ciGameInputTextBox.ResetInputMode( true );
    g_ciUI.m_ciGameInputTextBox.Clear();

    if( eTerminationType != DialogueTerminationType.Disconnected )
    {
      g_ciUI.m_ciGameInputTextBox.ShowPrompt( true );
    }
  }
}


// Sent from client when player wants NPC to carve a rune
class Player_Talk_Carve_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;
  var3 = 0;


  constructor( ... )
  {
    base.constructor();

    if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Carve phase
      var2 = vargv[1]; // Carve virtue
    }
  }


  function OnRecv()
  {
    local ePhase = var1;
    if( CarveState.SymbolPrompt == ePhase )
    {
      local eVirtue = var2;

      // The npc on the server wants to know what they symbol of a certain virtue is
      local strQuestion = ::rumGetString( talk_virtue_symbol_client_StringID );
      strQuestion += " " + ::rumGetString( g_eU4VirtueStringArray[eVirtue] );
      strQuestion += "?";

      ShowString( format( "%s", strQuestion ), g_strColorTagArray.Cyan );

      g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Text_Response, U4_CarveSymbolResponse );
    }
    else if( CarveState.SymbolWrong == ePhase )
    {
      // The player answered the symbol question incorrectly
      ShowString( format( "<b>%s", ::rumGetString( talk_incorrect_symbol_client_StringID ) ),
                  g_strColorTagArray.Cyan );
      U4_NPCPromptInterest();
    }
    else if( CarveState.MaterialNeeded == ePhase )
    {
      local eVirtue = var2;
      local strQuest = var3;

      // The player answered the symbol question correctly - display instructions on gathering a rune material
      local strDesc = ::rumGetString( talk_carve_act_client_StringID );
      strDesc += " " + ::rumGetString( g_eU4VirtueStringArray[eVirtue] ) + ".";
      ShowString( format( "<b>%s<b>%s", strDesc, strQuest ), g_strColorTagArray.Cyan );
      U4_NPCPromptInterest();
    }
    else if( CarveState.Success == ePhase )
    {
      // The NPC carved a rune for the player
      local strSuccess = var2;
      ShowString( format( "<b>%s", strSuccess ), g_strColorTagArray.Cyan );
      U4_NPCPromptInterest();
    }
    else if( CarveState.RuneOwned == ePhase )
    {
      // Player already has a rune
      ShowString( ::rumGetString( talk_already_carved_client_StringID ), g_strColorTagArray.Cyan );
      U4_NPCPromptInterest();
    }
  }
}


// Sent from client when player wants to give money to a beggar
class Player_Talk_Give_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;


  constructor( ... )
  {
    base.constructor();

    if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Give phase
      var2 = vargv[1]; // Give amount or NPC response
    }
  }


  function OnRecv()
  {
    local ePhase = var1;
    if( GiveState.AmountPrompt == ePhase )
    {
      // The npc on the server wants to know how much gold the player is willing to give
      ShowString( ::rumGetString( talk_how_much_client_StringID ), g_strColorTagArray.Cyan );

      g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, U4_GiveGold );
    }
    else if( GiveState.Reaction == ePhase )
    {
      // Beggar response
      ShowString( format( "<b>%s", ::rumGetStringByName( var2 + "_client_StringID" ) ), g_strColorTagArray.Cyan );
      U4_NPCPromptInterest();
    }
  }
}


class Player_Talk_Interrupted_Broadcast extends rumBroadcast
{
  function OnRecv()
  {
    if( g_ciCUO.m_ciKingRef != null || g_ciCUO.m_ciMerchantRef != null )
    {
      g_ciCUO.m_ciKingRef = null;
      g_ciCUO.m_ciMerchantRef = null;

      Ultima_ListSelectionEnd();
    }
    else if( g_ciUI.m_ciGameInputTextBox_m_eInputMode == InputMode.NPC_Chat )
    {
      g_ciUI.m_ciGameInputTextBox.ResetInputMode( false );
    }
  }
}


// Received from client when player gives an NPC a keyword
// Sent from server with NPC response
class Player_Talk_Keyword_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;


  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Keyword
    }
  }


  function OnRecv()
  {
    local strKeyword = var1;
    local strResponse = var2;

    if( strResponse.len() > 0 )
    {
      // Cache the npc's response
      local strKey = strKeyword + "_" + g_ciCUO.m_uiCurrentTalkID;
      g_ciCUO.m_strDialogueTable[strKey] <- strResponse;

      local iQuestionIndex = strResponse.find( "<q>" );

      // Does this response contain a question?
      if( iQuestionIndex != null )
      {
        U4_NPCQuestion( strResponse, iQuestionIndex );
      }
      else
      {
        ShowString( strResponse, g_strColorTagArray.Cyan );
      }
    }
    else
    {
      // The npc has nothing to say
      ShowString( ::rumGetString( talk_cannot_help_client_StringID ), g_strColorTagArray.Cyan );
    }

    U4_NPCPromptInterest();
  }
}


// Sent from server when player talks to Lord British
class Player_Talk_Lord_British_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;


  function OnKeyPressed()
  {
    ShowString( format( "<b>%s", ::rumGetString( u4_lord_british_greet2_client_StringID ) ),
                g_strColorTagArray.Cyan );
    ShowString( format( "<b>%s", ::rumGetString( u4_lord_british_prompt1_client_StringID ) ),
                g_strColorTagArray.Cyan );

    g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.NPC_Chat );
  }


  function OnRecv()
  {
    // Lord British gets special music
    PlayMusic( U3_Fanfare_SoundID );

    local eTalkType = var1;
    local ciPlayer = ::rumGetMainPlayer();
    local strName = ciPlayer.GetPlayerName();

    if( U4_LBTalkType.Greet == eTalkType )
    {
      // The player is meeting Lord British for the first time
      local strDesc = format( ::rumGetString( u4_lord_british_greet1_client_StringID ), strName );
      ShowString( format( "<b>%s", strDesc ), g_strColorTagArray.Cyan );

      // Get a keypress from the player
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.PressAnyKey, OnKeyPressed.bindenv( this ) );
    }
    else if( U4_LBTalkType.LevelUp == eTalkType )
    {
      local uiStatPointsArray = var2;

      local strDesc = format( ::rumGetString( u4_lord_british_level_client_StringID ),
                              strName, uiStatPointsArray[0] );
      ShowString( format( "<b>%s", strDesc ), g_strColorTagArray.Cyan );

      if( uiStatPointsArray[1] > 0 )
      {
        strDesc = format( "%s + %d",
                          ::rumGetString( U4_Strength_Property_client_StringID ), uiStatPointsArray[1] );
        ShowString( strDesc, g_strColorTagArray.Green );
      }

      if( uiStatPointsArray[2] > 0 )
      {
        strDesc = format( "%s + %d",
                          ::rumGetString( U4_Dexterity_Property_client_StringID ), uiStatPointsArray[2] );
        ShowString( strDesc, g_strColorTagArray.Green );
      }

      if( uiStatPointsArray[3] > 0 )
      {
        strDesc = format( "%s + %d",
                          ::rumGetString( U4_Intelligence_Property_client_StringID ), uiStatPointsArray[3] );
        ShowString( strDesc, g_strColorTagArray.Green );
      }
    }
    else if( U4_LBTalkType.Standard == eTalkType )
    {
      // The player has already met Lord British
      local strDesc = format( ::rumGetString( u4_lord_british_greet3_client_StringID ), strName );
      ShowString( format( "<b>%s", strDesc ), g_strColorTagArray.Cyan );

      strDesc = format( "<b>%s", ::rumGetString( u4_lord_british_prompt2_client_StringID ) );
      ShowString( strDesc, g_strColorTagArray.Cyan );
    }
  }
}


VirtueLoc <- [];

// Sent from server when player talks to Seer Hawkwind
class Player_Talk_Seer_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;


  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Phase
    }
    else if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Phase
      var2 = vargv[1]; // Virtue
    }
  }


  function OnRecv()
  {
    local ePhase = var1;

    if( U4_SeerTalkType.Greet == ePhase )
    {
      // The player has started talking to Seer Hawkwind

      // Rebuild the localized virtue table
      VirtueLoc.resize( VirtueType.NumVirtues );

      for( local eVirtue = 0; eVirtue < VirtueType.NumVirtues; ++eVirtue )
      {
        VirtueLoc[eVirtue] = ::rumGetString( g_eU4VirtueStringArray[eVirtue] ).tolower();
      }

      // The player is now in Seer conversation mode
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Seer_Chat );

      // Hide msg prompts during conversation
      g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );

      local ciPlayer = ::rumGetMainPlayer();
      local strDesc = format( ::rumGetString( u4_seer_greet_client_StringID ), ciPlayer.GetPlayerName() );
      ShowString( format( "<b>%s", strDesc ), g_strColorTagArray.Cyan );
      ShowString( format( "<b>%s", ::rumGetString( u4_seer_prompt_client_StringID ) ), g_strColorTagArray.Cyan );
    }
    else if( U4_SeerTalkType.Response == ePhase )
    {
      // The player received a response from Seer Hawkwind
      ShowString( format( "<b>%s", var2 ), g_strColorTagArray.Cyan );
      ShowString( format( "<b>%s", ::rumGetString( u4_seer_prompt_client_StringID ) ), g_strColorTagArray.Cyan );
    }
  }
}


// Sent from server when player talks to an Ultima 1 King
class Player_Talk_U1_King_Broadcast extends rumBroadcast
{
  var1 = 0; // Type
  var2 = 0;
  var3 = 0;
  var4 = 0;


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      var1 = vargv[0]; // Type
      if( vargv.len() > 1 )
      {
        var2 = vargv[1]; // Amount
      }
    }
  }


  function EndTransaction()
  {
    g_ciUI.m_ciGameInputTextBox.Clear();
    g_ciCUO.m_ciKingRef = null;

    local ciBroadcast = ::rumCreate( Player_Talk_U1_King_BroadcastID, U1_KingTalkType.Bye );
    ::rumSendBroadcast( ciBroadcast );
  }


  function OfferCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciGameListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local strDesc;
    local eType = g_ciUI.m_ciGameListView.GetSelectedKey();
    if( U1_KingTalkType.Pence == eType )
    {
      strDesc = ::rumGetString( talk_how_much_client_StringID );
      ShowString( strDesc );

      g_ciUI.m_ciGameInputTextBox.Clear();
      g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );

      // Note that we are sending a callback to a member function, so we have to bind to "this"
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Amount, OfferAmountCallback.bindenv( this ) );
    }
    else if( U1_KingTalkType.Service == eType )
    {
      local ciBroadcast = ::rumCreate( Player_Talk_U1_King_BroadcastID, U1_KingTalkType.Service );
      ::rumSendBroadcast( ciBroadcast );
    }
    else if( U1_KingTalkType.Return == eType )
    {
      local ciBroadcast = ::rumCreate( Player_Talk_U1_King_BroadcastID, U1_KingTalkType.Return );
      ::rumSendBroadcast( ciBroadcast );

      g_ciUI.m_ciGameInputTextBox.Clear();
    }
    else
    {
      EndTransaction();
    }

    Ultima_ListSelectionEnd();
  }


  function OfferAmountCallback( i_iAmount )
  {
    ShowString( i_iAmount.tostring() + "<b>" );
    g_ciUI.m_ciGameInputTextBox.Clear();

    local bOfferValid = false;

    if( i_iAmount > 0 )
    {
      local ciPlayer = ::rumGetMainPlayer();
      local ciCoin = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );

      // Does the player have the amount specified
      bOfferValid = ( ciCoin >= i_iAmount );

      if( ciCoin >= i_iAmount )
      {
        bOfferValid = true;
      }
      else
      {
        ShowString( ::rumGetString( talk_not_enough_client_StringID ) );
      }
    }

    if( bOfferValid )
    {
      local ciBroadcast = ::rumCreate( Player_Talk_U1_King_BroadcastID, U1_KingTalkType.Pence, i_iAmount );
      ::rumSendBroadcast( ciBroadcast );
    }
    else
    {
      EndTransaction();
    }
  }


  function OnRecv()
  {
    local ePhase = var1;

    // Save a reference to this instance, otherwise it'll be garbage collected before the callback is made
    g_ciCUO.m_ciKingRef = this;

    if( U1_KingTalkType.Greet == ePhase )
    {
      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();

      ShowString( format( "<b>%s", ::rumGetString( u1_king_prompt_client_StringID ) ), g_strColorTagArray.Cyan );

      g_ciUI.m_ciGameListView.Clear();
      g_ciUI.m_ciGameListView.SetEntry( U1_KingTalkType.Pence,
                                        ::rumGetString( u1_king_offer_pence_client_StringID ) );
      g_ciUI.m_ciGameListView.SetEntry( U1_KingTalkType.Service,
                                        ::rumGetString( u1_king_offer_service_client_StringID ) );

      if( ciMap instanceof U1_Castle_Lord_British_Map )
      {
        g_ciUI.m_ciGameListView.SetEntry( U1_KingTalkType.Return,
                                          ::rumGetString( u1_king_return_britannia_client_StringID ) );
      }

      CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

      g_ciUI.m_ciGameListView.m_funcAccept = OfferCallback.bindenv( this );
      g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
      g_ciUI.m_ciGameListView.SetActive( true );
      g_ciUI.m_ciGameListView.Focus();

      return;
    }
    else if( U1_KingTalkType.HPGranted == ePhase )
    {
      local iHitpoints = var2;
      ShowString( format( ::rumGetString( u1_king_hitpoints_granted_client_StringID ), var2 ),
                  g_strColorTagArray.Cyan );
    }
    else if( U1_KingTalkType.QuestBestowed == ePhase )
    {
      local strDesc = "";
      local eQuestState = var2;
      local eQuestType = var3;

      if( U1_KingQuestState.Bestowed1 == eQuestState )
      {
        strDesc = ::rumGetString( g_tblU1QuestArray[eQuestType].eTokenID1 );
      }
      else if( U1_KingQuestState.Bestowed2 == eQuestState )
      {
        strDesc = ::rumGetString( g_tblU1QuestArray[eQuestType].eTokenID2 );
      }

      if( ( U1_KingQuestType.LostKing == eQuestType ) || ( U1_KingQuestType.Rondorin == eQuestType ) ||
          ( U1_KingQuestType.BlackDragon == eQuestType ) || ( U1_KingQuestType.Shamino == eQuestType ) )
      {
        strDesc = format( ::rumGetString( u1_king_quest_bestowed_kill_client_StringID ), strDesc );
      }
      else
      {
        strDesc = format( ::rumGetString( u1_king_quest_bestowed_find_client_StringID ), strDesc );
      }

      ShowString( format( "%s %s<b>", strDesc, ::rumGetString( u1_king_quest_bestowed_client_StringID ) ),
                  g_strColorTagArray.Cyan );
    }
    else if( U1_KingTalkType.QuestAlreadyBestowed == ePhase )
    {
      ShowString( ::rumGetString( u1_king_quest_already_bestowed_client_StringID ), g_strColorTagArray.Cyan );
    }
    else if( U1_KingTalkType.QuestReward == ePhase )
    {
      local ciPlayer = ::rumGetMainPlayer();
      ShowString( format( ::rumGetString( u1_king_quest_reward_client_StringID ), ciPlayer.GetPlayerName() ),
                  g_strColorTagArray.Cyan );

      local eRewardType = var2;
      if( U1_KingQuestRewardType.Stat == eRewardType )
      {
        local ePropertyID = var3;
        local iAmount = var4;

        local ciProperty = ::rumGetPropertyAsset( ePropertyID );
        local strPropertyDesc = ::rumGetStringByName( format( "%s_Property_client_StringID", ciProperty.GetName() ) );
        local strDesc = format( ::rumGetString( u1_king_quest_reward_stat_client_StringID ), iAmount, strPropertyDesc );
        ShowString( format( "<b>%s", strDesc ), g_strColorTagArray.Cyan );
        ShowString( format( "<b>%s + %d", strPropertyDesc, iAmount ), g_strColorTagArray.Green );
      }
      else if( U1_KingQuestRewardType.Hint == eRewardType )
      {
        local strHint = var3;
        ShowString( "<b>" + format( ::rumGetString( u1_king_quest_reward_hint_client_StringID ), strHint ),
                    g_strColorTagArray.Cyan );
      }
      else if( U1_KingQuestRewardType.Gem == eRewardType )
      {
        local eQuestType = var3;
        local strGem = format( "u1_property_gem_%d_client_StringID", eQuestType );
        local strDesc = "<b>" + format( ::rumGetString( u1_king_quest_reward_gem_client_StringID ),
                                        ::rumGetStringByName( strGem ) );
        ShowString( strDesc, g_strColorTagArray.Green );
      }
    }

    EndTransaction();
  }
}


// Sent from server when player talks to an Ultima 1 princess
class Player_Talk_U1_Princess_Broadcast extends rumBroadcast
{
  var1 = 0; // Type
  var2 = 0; // String
  var3 = 0; // String


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() == 2 )
    {
      var1 = vargv[0]; // Type
      var2 = vargv[1]; // String
    }
  }


  function OnRecv()
  {
    local eType = var1;

    if( U1_PrincessTalkType.Greet == eType )
    {
      local eNameStringID = var2;
      local strMessage = var3;

      local strDesc = format( "<b>%s %s!<b>", strMessage, ::rumGetString( eNameStringID ) );
      ShowString( strDesc, g_strColorTagArray.Green );
      ShowString( ::rumGetString( msg_princess_reward_client_StringID ), g_strColorTagArray.Green );
    }
    else if( U1_PrincessTalkType.TimeMachine == eType )
    {
      local strDesc = format( "<b>%s", var2 );
      ShowString( strDesc, g_strColorTagArray.Cyan );
    }
  }
}


// Sent from server when player talks to Ultima 2 Hotel California clerk
class Player_Talk_U2_HotelClerk_Broadcast extends rumBroadcast
{
  var1 = 0; // Type
  var2 = 0; // Amount
  var3 = 0; // Stat index


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      var1 = vargv[0]; // Type
    }
  }


  function EndTransaction()
  {
    g_ciUI.m_ciGameInputTextBox.Clear();
    g_ciCUO.m_ciKingRef = null;

    local ciBroadcast = ::rumCreate( Player_Talk_U2_HotelClerk_BroadcastID, U2_ClerkTalkType.Bye );
    ::rumSendBroadcast( ciBroadcast );
  }


  function OnRecv()
  {
    local ePhase = var1;

    // Save a reference to this instance, otherwise it'll be garbage collected before the callback is made
    g_ciCUO.m_ciKingRef = this;

    if( U2_ClerkTalkType.Greet == ePhase )
    {
      ShowString( format( "<b>%s", ::rumGetString( u2_clerk_greet_client_StringID ) ), g_strColorTagArray.Cyan );

      g_ciUI.m_ciGameListView.Clear();
      g_ciUI.m_ciGameListView.SetEntry( U2_ClerkTalkType.RequestRoom,
                                        ::rumGetString( u2_clerk_request_room_client_StringID ) );
      g_ciUI.m_ciGameListView.SetEntry( U2_ClerkTalkType.RequestRoomTip,
                                        ::rumGetString( u2_clerk_request_room_with_tip_client_StringID ) );
      g_ciUI.m_ciGameListView.SetEntry( U2_ClerkTalkType.Bye,
                                        ::rumGetString( talk_bye_client_StringID ) );

      CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

      g_ciUI.m_ciGameListView.m_funcAccept = TalkCallback.bindenv( this );
      g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
      g_ciUI.m_ciGameListView.SetActive( true );
      g_ciUI.m_ciGameListView.Focus();
    }
    else if( U2_ClerkTalkType.RequestRoomTip == ePhase )
    {
      local iAmount = var2;
      local iStatIndex = var3;

      ShowString( ::rumGetString( token_alakazam_client_StringID ), g_strColorTagArray.Cyan );

      local eDesc = g_eU2StatDescriptionArray[iStatIndex];
      local strDesc = format( ::rumGetString( eDesc ), iAmount );
      ShowString( strDesc, g_strColorTagArray.Green );
    }
  }


  function TalkCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciGameListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local strDesc;
    local eType = g_ciUI.m_ciGameListView.GetSelectedKey();
    if( U2_ClerkTalkType.RequestRoom == eType )
    {
      local ciPlayer = ::rumGetMainPlayer();
      local iPlayerGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
      if( iPlayerGold >= U2_NPC.s_iRoomCost )
      {
        local ciBroadcast = ::rumCreate( Player_Talk_U2_HotelClerk_BroadcastID, U2_ClerkTalkType.RequestRoom );
        ::rumSendBroadcast( ciBroadcast );
      }
      else
      {
        ShowString( ::rumGetString( u2_lord_british_tribute_failed_client_StringID ), g_strColorTagArray.Cyan );
        EndTransaction();
      }
    }
    else if( U2_ClerkTalkType.RequestRoomTip == eType )
    {
      local ciPlayer = ::rumGetMainPlayer();
      local iPlayerGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
      if( iPlayerGold >= U2_NPC.s_iRoomCostWithTip )
      {
        local ciBroadcast = ::rumCreate( Player_Talk_U2_HotelClerk_BroadcastID, U2_ClerkTalkType.RequestRoomTip );
        ::rumSendBroadcast( ciBroadcast );
      }
      else
      {
        ShowString( ::rumGetString( u2_lord_british_tribute_failed_client_StringID ), g_strColorTagArray.Cyan );
        EndTransaction();
      }
    }
    else
    {
      EndTransaction();
    }

    Ultima_ListSelectionEnd();
  }
}


// Sent from server when player talks to Ultima 2 Lord British
class Player_Talk_U2_LordBritish_Broadcast extends rumBroadcast
{
  var1 = 0; // Type
  var2 = 0; // Amount
  var3 = 0; // Stat index


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      var1 = vargv[0]; // Type
      if( vargv.len() > 1 )
      {
        var2 = vargv[1];
      }
    }
  }


  function EndTransaction()
  {
    g_ciUI.m_ciGameInputTextBox.Clear();
    g_ciCUO.m_ciKingRef = null;

    local ciBroadcast = ::rumCreate( Player_Talk_U2_LordBritish_BroadcastID, U2_LBTalkType.Bye );
    ::rumSendBroadcast( ciBroadcast );
  }


  function OnRecv()
  {
    local ePhase = var1;

    // Save a reference to this instance, otherwise it'll be garbage collected before the callback is made
    g_ciCUO.m_ciKingRef = this;

    if( U2_LBTalkType.Greet == ePhase )
    {
      ShowString( format( "<b>%s", ::rumGetString( u2_lord_british_greet_client_StringID ) ),
                  g_strColorTagArray.Cyan );

      g_ciUI.m_ciGameListView.Clear();
      g_ciUI.m_ciGameListView.SetEntry( U2_LBTalkType.HitpointTribute,
                                        ::rumGetString( u2_lord_british_offer_hitpoint_tribute_client_StringID ) );
      g_ciUI.m_ciGameListView.SetEntry( U2_LBTalkType.StatTribute,
                                        ::rumGetString( u2_lord_british_offer_stat_tribute_client_StringID ) );
      g_ciUI.m_ciGameListView.SetEntry( U2_LBTalkType.Return,
                                        ::rumGetString( u2_lord_british_return_britannia_client_StringID ) );
      g_ciUI.m_ciGameListView.SetEntry( U2_LBTalkType.Bye,
                                        ::rumGetString( talk_bye_client_StringID ) );

      CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

      g_ciUI.m_ciGameListView.m_funcAccept = TalkCallback.bindenv( this );
      g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
      g_ciUI.m_ciGameListView.SetActive( true );
      g_ciUI.m_ciGameListView.Focus();
    }
    else if( U2_LBTalkType.HitpointTribute == ePhase )
    {
      local iHitpoints = var2;

      local strDesc = format( ::rumGetString( u2_lord_british_hp_client_StringID ), var2 );
      ShowString( strDesc, g_strColorTagArray.Cyan );
    }
    else if( U2_LBTalkType.StatTribute == ePhase )
    {
      local iAmount = var2;
      local iStatIndex = var3;

      ShowString( ::rumGetString( token_alakazam_client_StringID ), g_strColorTagArray.Cyan );

      local eDesc = g_eU2StatDescriptionArray[iStatIndex];
      local strDesc = format( ::rumGetString( eDesc ), iAmount );
      ShowString( strDesc, g_strColorTagArray.Green );
    }
    else if( U2_LBTalkType.Endgame == ePhase )
    {
      local ciArray = [var2, 0];
      U2_Endgame( ciArray );
      return;
    }
    else
    {
      EndTransaction();
    }
  }


  function TalkCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciGameListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local strDesc;
    local eType = g_ciUI.m_ciGameListView.GetSelectedKey();
    if( U2_LBTalkType.HitpointTribute == eType )
    {
      ShowString( ::rumGetString( u2_lord_british_tribute_client_StringID ), g_strColorTagArray.Cyan );

      local ciPlayer = ::rumGetMainPlayer();
      local iPlayerGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
      if( iPlayerGold >= U2_NPC.s_iHitpointTributeCost )
      {
        local ciBroadcast = ::rumCreate( Player_Talk_U2_LordBritish_BroadcastID, U2_LBTalkType.HitpointTribute );
        ::rumSendBroadcast( ciBroadcast );
      }
      else
      {
        ShowString( ::rumGetString( u2_lord_british_tribute_failed_client_StringID ), g_strColorTagArray.Cyan );
        EndTransaction();
      }
    }
    else if( U2_LBTalkType.StatTribute == eType )
    {
      ShowString( ::rumGetString( u2_lord_british_tribute_client_StringID ), g_strColorTagArray.Cyan );

      local ciPlayer = ::rumGetMainPlayer();
      local iPlayerGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
      if( iPlayerGold >= U2_NPC.s_iStatTributeCost )
      {
        local ciBroadcast = ::rumCreate( Player_Talk_U2_LordBritish_BroadcastID, U2_LBTalkType.StatTribute );
        ::rumSendBroadcast( ciBroadcast );
      }
      else
      {
        ShowString( ::rumGetString( u2_lord_british_tribute_failed_client_StringID ), g_strColorTagArray.Cyan );
        EndTransaction();
      }
    }
    else if( U2_LBTalkType.Return == eType )
    {
      local ciBroadcast = ::rumCreate( Player_Talk_U2_LordBritish_BroadcastID, U2_LBTalkType.Return );
      ::rumSendBroadcast( ciBroadcast );

      g_ciUI.m_ciGameInputTextBox.Clear();
    }
    else
    {
      EndTransaction();
    }

    Ultima_ListSelectionEnd();
  }
}


class Player_Talk_U2_OldMan_Broadcast extends rumBroadcast
{
  var1 = 0; // Dialogue
  var2 = 0; // Ring location


  function OnRecv()
  {
    if( var1.find( "%s" ) )
    {
      local eMapID = var2;
      local ciMapAsset = ::rumGetMapAsset( eMapID );
      if( ciMapAsset != null )
      {
        // Show the Ring's location
        local strMapName = ciMapAsset.GetName();
        local ex = regexp( "[0-9_]+$" );
        local res = ex.search( strMapName );
        if( res != null )
        {
          strMapName = strMapName.slice( 0, res.begin );
        }

        local strMapDesc = ::rumGetStringByName( strMapName + "_Map_client_StringID" );
        local ciStrArray = split( strMapDesc, "()" );
        local strLocation = format( var1, ciStrArray[0], ciStrArray[1] );
        ShowString( format( "<b>%s<b>", strLocation ), g_strColorTagArray.Cyan );
      }
    }
    else
    {
      ShowString( format( "<b>%s<b>", var1 ), g_strColorTagArray.Cyan );
    }
  }
}


class Player_Talk_U2_Oracle_Broadcast extends rumBroadcast
{
  var1 = 0; // Type
  var2 = 0; // Clue


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      var1 = vargv[0]; // Type
    }
  }


  function EndTransaction()
  {
    g_ciUI.m_ciGameInputTextBox.Clear();
    g_ciCUO.m_ciKingRef = null;

    local ciBroadcast = ::rumCreate( Player_Talk_U2_HotelClerk_BroadcastID, U2_ClerkTalkType.Bye );
    ::rumSendBroadcast( ciBroadcast );
  }


  function GetCost( i_ciPlayer )
  {
    local iOracleFlags = i_ciPlayer.GetProperty( U2_Oracle_Flags_PropertyID, 0 );
    local iNumOraclesPaid = PopCount( iOracleFlags );

    return 100 * iNumOraclesPaid + 100;
  }


  function OnRecv()
  {
    local ePhase = var1;

    // Save a reference to this instance, otherwise it'll be garbage collected before the callback is made
    g_ciCUO.m_ciKingRef = this;

    if( U2_ClerkTalkType.Greet == ePhase )
    {
      ShowString( format( "<b>%s", ::rumGetString( u2_oracle_greet_client_StringID ) ), g_strColorTagArray.Cyan );

      local ciPlayer = ::rumGetMainPlayer();
      local strDesc = format( ::rumGetString( u2_oracle_pay_client_StringID ), GetCost( ciPlayer ) );

      g_ciUI.m_ciGameListView.Clear();
      g_ciUI.m_ciGameListView.SetEntry( U2_OracleTalkType.Pay, strDesc );
      g_ciUI.m_ciGameListView.SetEntry( U2_OracleTalkType.Bye, ::rumGetString( talk_bye_client_StringID ) );

      CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

      g_ciUI.m_ciGameListView.m_funcAccept = TalkCallback.bindenv( this );
      g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
      g_ciUI.m_ciGameListView.SetActive( true );
      g_ciUI.m_ciGameListView.Focus();
    }
    else if( U2_OracleTalkType.Clue == ePhase )
    {
      local strClue = var2;
      ShowString( strClue, g_strColorTagArray.Cyan );
    }
  }


  function TalkCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciGameListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local strDesc;
    local eType = g_ciUI.m_ciGameListView.GetSelectedKey();
    if( U2_OracleTalkType.Pay == eType )
    {
      local ciPlayer = ::rumGetMainPlayer();

      local iPlayerGold = ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
      if( iPlayerGold >= GetCost( ciPlayer ) )
      {
        local ciBroadcast = ::rumCreate( Player_Talk_U2_Oracle_BroadcastID, U2_OracleTalkType.Pay );
        ::rumSendBroadcast( ciBroadcast );
      }
      else
      {
        ShowString( ::rumGetString( u2_lord_british_tribute_failed_client_StringID ), g_strColorTagArray.Cyan );
        EndTransaction();
      }
    }
    else if( U2_OracleTalkType.Bye == eType )
    {
      ShowString( format( "%s<b>", ::rumGetString( u2_oracle_bye_client_StringID ) ), g_strColorTagArray.Cyan );
      EndTransaction();
    }
    else
    {
      EndTransaction();
    }

    Ultima_ListSelectionEnd();
  }
}


// Sent from server when player talks to Ultima 2 Sentri
class Player_Talk_U2_Sentri_Broadcast extends rumBroadcast
{
  var1 = 0; // Dialogue


  function OnRecv()
  {
    local strDesc = var1;
    ShowString( format( "<b>%s", strDesc ), g_strColorTagArray.Cyan );
  }
}


// Sent from server when player talks to Ultima 3 Lord British
class Player_Talk_U3_LordBritish_Broadcast extends rumBroadcast
{
  var1 = 0; // Type
  var2 = 0;


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      var1 = vargv[0]; // Type
      if( vargv.len() > 1 )
      {
        var2 = vargv[1]; // Amount
      }
    }
  }


  function DialogueCallback()
  {
    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciGameListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local strDesc;
    local eType = g_ciUI.m_ciGameListView.GetSelectedKey();
    if( U3_LBTalkType.Report == eType )
    {
      local ciBroadcast = ::rumCreate( Player_Talk_U3_LordBritish_BroadcastID, U3_LBTalkType.Report );
      ::rumSendBroadcast( ciBroadcast );

      g_ciUI.m_ciGameInputTextBox.Clear();
    }
    else if( U3_LBTalkType.Return == eType )
    {
      local ciBroadcast = ::rumCreate( Player_Talk_U3_LordBritish_BroadcastID, U3_LBTalkType.Return );
      ::rumSendBroadcast( ciBroadcast );

      g_ciUI.m_ciGameInputTextBox.Clear();
    }
    else
    {
      EndTransaction();
    }

    Ultima_ListSelectionEnd();
  }


  function EndTransaction()
  {
    g_ciUI.m_ciGameInputTextBox.Clear();
    g_ciCUO.m_ciKingRef = null;

    local ciBroadcast = ::rumCreate( Player_Talk_U3_LordBritish_BroadcastID, U3_LBTalkType.Bye );
    ::rumSendBroadcast( ciBroadcast );
  }


  function OnRecv()
  {
    local ePhase = var1;

    // Save a reference to this instance, otherwise it'll be garbage collected before the callback is made
    g_ciCUO.m_ciKingRef = this;

    if( U3_LBTalkType.Greet == ePhase )
    {
      ShowString( ::rumGetString( u3_lord_british_greet_client_StringID ), g_strColorTagArray.Cyan );

      g_ciUI.m_ciGameListView.Clear();

      g_ciUI.m_ciGameListView.SetEntry( U3_LBTalkType.Report,
                                        ::rumGetString( u3_lord_british_report_client_StringID ) );
      g_ciUI.m_ciGameListView.SetEntry( U3_LBTalkType.Return,
                                        ::rumGetString( u3_lord_british_return_britannia_client_StringID ) );
      g_ciUI.m_ciGameListView.SetEntry( U3_LBTalkType.Bye,
                                        ::rumGetString( u3_lord_british_bye_client_StringID ) );

      CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

      g_ciUI.m_ciGameListView.m_funcAccept = DialogueCallback.bindenv( this );
      g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
      g_ciUI.m_ciGameListView.SetActive( true );
      g_ciUI.m_ciGameListView.Focus();

      return;
    }
    else if( U3_LBTalkType.NeedExp == ePhase )
    {
      ShowString( ::rumGetString( u3_lord_british_exp_more_client_StringID ), g_strColorTagArray.Cyan );
    }
    else if( U3_LBTalkType.LevelUp == ePhase )
    {
      ShowString( ::rumGetString( u3_lord_british_greater_client_StringID ), g_strColorTagArray.Cyan );
    }
    else if( U3_LBTalkType.LevelCap == ePhase )
    {
      ShowString( ::rumGetString( u3_lord_british_no_more_client_StringID ), g_strColorTagArray.Cyan );
    }
    else if( U3_LBTalkType.SeekMark == ePhase )
    {
      ShowString( ::rumGetString( u3_lord_british_mark_req_client_StringID ), g_strColorTagArray.Cyan );
    }
    else if( U3_LBTalkType.Endgame == ePhase )
    {
      local ciArray = [var2, 0];
      U3_Endgame( ciArray );
      return;
    }

    EndTransaction();
  }
}


// Sent from server when player talks to Ultima 3 Timelord
class Player_Talk_U3_Timelord_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;
  var3 = 0;
  var4 = 0;


  function OnRecv()
  {
    local strCard1 = ::rumGetStringByName( format( "u3_card_%d_client_StringID", var1 ) );
    local strCard2 = ::rumGetStringByName( format( "u3_card_%d_client_StringID", var2 ) );
    local strCard3 = ::rumGetStringByName( format( "u3_card_%d_client_StringID", var3 ) );
    local strCard4 = ::rumGetStringByName( format( "u3_card_%d_client_StringID", var4 ) );

    local strDesc = format( ::rumGetString( msg_timelord_client_StringID ), strCard1, strCard2, strCard3, strCard4 );
    ShowString( format( "<b>%s", strDesc ), g_strColorTagArray.Cyan );
  }
}
