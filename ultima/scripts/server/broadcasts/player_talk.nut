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
      var1 = vargv[0]; // Response
    }
    else if( 4 == vargv.len() )
    {
      var1 = vargv[0]; // Talk id
      var2 = vargv[1]; // Pronoun
      var3 = vargv[2]; // Name
      var4 = vargv[3]; // Desc
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local eDir = var1;
    local bFound = false;

    if( !( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() ) )
    {
      local eTransportType = TransportType.None;
      local ciTransport = i_ciPlayer.GetTransport();
      if( ciTransport != null )
      {
        eTransportType = ciTransport.GetType();
      }

      if( ( TransportType.None == eTransportType ) || ( TransportType.Horse == eTransportType ) )
      {
        local ciVector = GetDirectionVector( eDir );

        // Save the first target position
        local ciPos = i_ciPlayer.GetPosition() + ciVector;

        local ciMap = i_ciPlayer.GetMap();
        local ciPosData = ciMap.GetPositionData( ciPos );

        // If the player is talking to a sign, advance to the next tile
        local ciTile = ::rumGetTileAsset( ciPosData.GetTileID() );
        if( IsSignTile( ciTile ) )
        {
          bFound = TryTalk( i_ciPlayer, ciMap, ciPos + ciVector, eDir );
          if( !bFound )
          {
            // If the vector was diagonal, check other nearby spaces
            if( ciVector.x != 0 && ciVector.y != 0 )
            {
              // Try a vertical check
              local ciVectorTest = rumVector( 0, ciVector.y );
              bFound = TryTalk( i_ciPlayer, ciMap, ciPos + ciVectorTest, eDir );
              if( !bFound )
              {
                // Try a horizontal check
                ciVectorTest = rumVector( ciVector.x, 0 );
                bFound = TryTalk( i_ciPlayer, ciMap, ciPos + ciVectorTest, eDir );
              }
            }
          }
        }
        else
        {
          bFound = TryTalk( i_ciPlayer, ciMap, ciPos, eDir );
        }
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_only_on_foot_client_StringID );
        bFound = true;
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      bFound = true;
    }

    if( !bFound )
    {
      // This differs slightly from Ultima IV... say "Not here" when
      // no NPC is at the location specified. We only say "Funny, no
      // response" when a response might be expected.
      i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
    }

    i_ciPlayer.PopPacket();
  }


  function TryTalk( i_ciPlayer, i_ciMap, i_ciPos, i_eDir )
  {
    local bFound = false;
    local ciVector = GetDirectionVector( i_eDir );
    local ciPosData = i_ciMap.GetPositionData( i_ciPos );

    // Skip across widgets that allow it
    local ciWidget = ciPosData.GetNext( rumWidgetPawnType );
    if( ciWidget != null )
    {
      local bForward = ciWidget.GetProperty( Widget_Forwards_Interaction_PropertyID, false );
      if( bForward )
      {
        bFound = TryTalk( i_ciPlayer, i_ciMap, i_ciPos + ciVector, i_eDir );
        if( !bFound )
        {
          // If the vector was diagonal, check other nearby spaces
          if( ciVector.x != 0 && ciVector.y != 0 )
          {
            // Try a vertical check
            local ciVectorTest = rumVector( 0, ciVector.y );
            bFound = TryTalk( i_ciPlayer, i_ciMap, i_ciPos + ciVectorTest, i_eDir );
            if( !bFound )
            {
              // Try a horizontal check
              ciVectorTest = rumVector( ciVector.x, 0 );
              bFound = TryTalk( i_ciPlayer, i_ciMap, i_ciPos + ciVectorTest, i_eDir );
            }
          }
        }
      }
    }

    if( !bFound )
    {
      ciPosData.Reset();

      local ciTarget = null;
      while( ciTarget = ciPosData.GetNext( rumCreaturePawnType ) )
      {
        if( ciTarget.IsVisible() && ( ciTarget instanceof NPC ) )
        {
          bFound = true;
          ciTarget.Talk( i_ciPlayer );
          ciPosData.Stop();
        }
      }
    }

    return bFound;
  }
}


// Received from client when player answers an NPCs yes or no question
class Player_Talk_Answer_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    // Get the NPC the player is talking to
    local ciTarget = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
    if( ciTarget != null && ( ciTarget instanceof NPC ) )
    {
      local ciMap = i_ciPlayer.GetMap();
      if( ciMap.GetTileDistance( i_ciPlayer.GetPosition(), ciTarget.GetPosition() ) <= 1 )
      {
        ciTarget.CheckAnswer( i_ciPlayer, var1, var2 );
      }
    }

    i_ciPlayer.PopPacket();
  }
}


// Received from client when player willfully ends npc dialogue
// Sent from server on conversation end
class Player_Talk_Bye_Broadcast extends rumBroadcast
{
  var = 0;


  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var = vargv[0]; // Termination type
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( i_ciPlayer instanceof Player )
    {
      // Get the NPC the player is talking to
      local ciTarget = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
      if( ciTarget != null && ( ciTarget instanceof NPC ) )
      {
        ciTarget.TalkEnd( i_ciPlayer, DialogueTerminationType.Standard );
      }

      i_ciPlayer.PopPacket();
    }
  }
}


// Received from client when player wants NPC to carve a rune
class Player_Talk_Carve_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;
  var3 = 0;


  constructor( ... )
  {
    base.constructor();

    switch( vargv.len() )
    {
      case 3: var3 = vargv[2]; // fall through!
      case 2: var2 = vargv[1];
      case 1: var1 = vargv[0];
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local ePhase = var1;

    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local ciTarget = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
    if( ciTarget != null && ( ciTarget instanceof NPC ) )
    {
      local ciMap = i_ciPlayer.GetMap();
      if( ciMap.GetTileDistance( i_ciPlayer.GetPosition(), ciTarget.GetPosition() ) <= 1 )
      {
        if( CarveState.Init == ePhase )
        {
          // Get the virtue of the rune that this NPC carves
          local eVirtue = ciTarget.GetProperty( NPC_Carves_Rune_PropertyID, -1 );
          if( eVirtue != -1 )
          {
            local iRuneFlags = i_ciPlayer.GetProperty( U4_Item_Runes_PropertyID, 0 );
            if( ::rumBitOn( iRuneFlags, eVirtue ) )
            {
              local ciBroadcast = ::rumCreate( Player_Talk_Carve_BroadcastID, CarveState.RuneOwned );
              i_ciPlayer.SendBroadcast( ciBroadcast );
            }
            else
            {
              // Start by asking the player what the symbol of the relative virtue is
              local ciBroadcast = ::rumCreate( Player_Talk_Carve_BroadcastID, CarveState.SymbolPrompt, eVirtue );
              i_ciPlayer.SendBroadcast( ciBroadcast );
            }
          }
          else
          {
            // NPC may not be a rune-carver, but may have something to say about carving
            ciTarget.KeywordResponse( i_ciPlayer, var2 );
          }
        }
        else if( CarveState.SymbolAnswer == ePhase )
        {
          // Get the virtue of the rune that this NPC carves
          local eVirtue = ciTarget.GetProperty( NPC_Carves_Rune_PropertyID, -1 );

          // Make sure the player's answer was correct
          local strSymbol = ::rumGetString( g_eU4VirtueSymbolStringArray[eVirtue], i_ciPlayer.m_iLanguageID ).tolower();
          if( strSymbol == var2 )
          {
            local iFlags = i_ciPlayer.GetProperty( U4_Item_Rune_Materials_PropertyID, 0 );
            if( !::rumBitOn( iFlags, eVirtue ) )
            {
              // Tell the player what is needed
              local strDesc = ::rumGetString( g_eU4CarveQuestBestowStringArray[eVirtue], i_ciPlayer.m_iLanguageID );

              local ciBroadcast = ::rumCreate( Player_Talk_Carve_BroadcastID,
                                               CarveState.MaterialNeeded,
                                               eVirtue,
                                               strDesc );
              i_ciPlayer.SendBroadcast( ciBroadcast );
            }
            else
            {
              // The player has the correct material, create the rune for them
              local strDesc = ::rumGetString( g_eU4CarveQuestCompleteStringArray[eVirtue], i_ciPlayer.m_iLanguageID );

              local ciBroadcast = ::rumCreate( Player_Talk_Carve_BroadcastID, CarveState.Success, strDesc );
              i_ciPlayer.SendBroadcast( ciBroadcast );

              // Remove the material, and add the rune
              iFlags = ::rumBitClear( iFlags, eVirtue );
              i_ciPlayer.SetProperty( U4_Item_Rune_Materials_PropertyID, iFlags );

              local iRuneFlags = i_ciPlayer.GetProperty( U4_Item_Runes_PropertyID, 0 );
              iRuneFlags = ::rumBitSet( iRuneFlags, eVirtue );
              i_ciPlayer.SetProperty( U4_Item_Runes_PropertyID, iRuneFlags );

              // Virtue reward for quest
              i_ciPlayer.AffectVirtue( VirtueType.Honor, 2, true, true );
            }
          }
          else
          {
            // Tell the player that they were wrong about the symbol
            local ciBroadcast = ::rumCreate( Player_Talk_Carve_BroadcastID, CarveState.SymbolWrong,
                                             "talk_incorrect_symbol" );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
        }
      }
    }

    i_ciPlayer.PopPacket();
  }
}


// Received from client when player wants to give money to a beggar
// Sent to the client when the npc wants to know how much
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
      var2 = vargv[1]; // Give amount
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local ePhase = var1;

    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local ciTarget = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
    if( ciTarget != null && ( ciTarget instanceof NPC ) )
    {
      local ciMap = i_ciPlayer.GetMap();
      if( ciMap.GetTileDistance( i_ciPlayer.GetPosition(), ciTarget.GetPosition() ) <= 1 )
      {
        if( 0 == ePhase )
        {
          // The player is simply trying to give gold to the NPC... does this npc beg for gold?
          if( ciTarget.GetProperty( NPC_Begs_PropertyID, false ) )
          {
            // Get the amount from the player
            local ciBroadcast = ::rumCreate( Player_Talk_Give_BroadcastID, GiveState.AmountPrompt, 0 );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
          else
          {
            // NPC may not beg, but may have something to say about begging
            ciTarget.KeywordResponse( i_ciPlayer, var2 );
          }
        }
        else if( 2 == ePhase )
        {
          // The player has given the npc some gold, so find out if they have the amount given
          local iGoldGiven = var2;
          local iGoldOwned = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );

          if( iGoldOwned >= iGoldGiven )
          {
            // Thank the player
            local ciBroadcast = ::rumCreate( Player_Talk_Give_BroadcastID, GiveState.Reaction, "talk_thank_kindness" );
            i_ciPlayer.SendBroadcast( ciBroadcast );

            i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iGoldGiven );

            if( iGoldOwned == iGoldGiven )
            {
              i_ciPlayer.AffectVirtue( VirtueType.Honor, 3, false, true );
            }

            local iVirtueReward = 1;

            if( iGoldGiven > 75 )
            {
              iVirtueReward = 4;
            }
            else if( iGoldGiven > 50)
            {
              iVirtueReward = 3;
            }
            else if( iGoldGiven > 25)
            {
              iVirtueReward = 2;
            }

            i_ciPlayer.AffectVirtue( VirtueType.Compassion, 3, false, true );
          }
          else
          {
            // Tell the player that they didn't have enough gold
            local ciBroadcast = ::rumCreate( Player_Talk_Give_BroadcastID, GiveState.Reaction, "talk_not_enough_gold" );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
        }
      }
    }

    i_ciPlayer.PopPacket();
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

    if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Keyword
      var2 = vargv[1]; // Response
    }
  }

  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local strKeyword = var1;

    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local ciTarget = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
    if( ciTarget != null && ( ciTarget instanceof NPC ) )
    {
      local ciMap = i_ciPlayer.GetMap();
      if( ciMap.GetTileDistance( i_ciPlayer.GetPosition(), ciTarget.GetPosition() ) <= 1 )
      {
        local iAttackChance = ciTarget.GetProperty( NPC_Attack_Chance_PropertyID, 0 );
        if( iAttackChance > 0 && rand() % 100 < iAttackChance )
        {
          // The NPC stops talking to the player and attacks
          ciTarget.TalkEnd( i_ciPlayer, DialogueTerminationType.TurnsAway );
          ciTarget.AddAttackTarget( i_ciPlayer, iAttackChance );
        }
        else
        {
          local iTurnAwayChance = ciTarget.GetProperty( NPC_Turn_Away_Chance_PropertyID, 0 );
          if( iTurnAwayChance > 0 && ( rand() % 100 < iTurnAwayChance ) )
          {
            // The NPC stops talking to the player
            ciTarget.TalkEnd( i_ciPlayer, DialogueTerminationType.TurnsAway );
          }
          else
          {
            // Get the NPC the player is talking to
            ciTarget.KeywordResponse( i_ciPlayer, strKeyword );
          }
        }
      }
    }

    i_ciPlayer.PopPacket();
  }
}


// Sent from server when player talks to Lord British
class Player_Talk_Lord_British_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;


  constructor( ... )
  {
    base.constructor();

    if( 1 == vargv.len() )
    {
      var1 = vargv[0]; // Talk type
    }
    else if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Talk type
      var2 = vargv[1]; // LevelUp Array
    }
  }
}


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
      var2 = vargv[1]; // Virtue response
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local ciTarget = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
    if( ciTarget != null && ( ciTarget instanceof NPC ) )
    {
      local ciMap = i_ciPlayer.GetMap();
      if( ciMap.GetTileDistance( i_ciPlayer.GetPosition(), ciTarget.GetPosition() ) <= 2 )
      {
        local ePhase = var1;

        if( 1 == ePhase )
        {
          local eVirtue = var2;

          // Retrieve the property matching the virtue specified
          local iLevel = i_ciPlayer.GetProperty( g_eU4VirtuePropertyArray[eVirtue], 0 );
          if( iLevel >= g_uiVirtuePointsTable.VeryHigh )
          {
            iLevel = 1;

            // Give elevation permission for the affected virtue
            local iFlags = i_ciPlayer.GetProperty( U4_Seer_Bestowals_PropertyID, 0 );
            iFlags = ::rumBitSet( iFlags, eVirtue );
            i_ciPlayer.SetProperty( U4_Seer_Bestowals_PropertyID, iFlags );
          }
          else if( iLevel >= g_uiVirtuePointsTable.High )
          {
            iLevel = 2;
          }
          else if( iLevel > g_uiVirtuePointsTable.Low )
          {
            iLevel = 3;
          }
          else if( iLevel > g_uiVirtuePointsTable.VeryLow )
          {
            iLevel = 4;
          }
          else // g_uiVirtuePointsTable.VeryLow
          {
            iLevel = 5;
          }

          // Get localized response based on virtue iLevel
          local strToken = "u4_seer_virtue_" + eVirtue + "_" + iLevel + "_server_StringID";
          local strDesc = ::rumGetStringByName( strToken, i_ciPlayer.m_iLanguageID );

          // Send response to player
          local ciBroadcast = ::rumCreate( Player_Talk_Seer_BroadcastID, U4_SeerTalkType.Response, strDesc );
          i_ciPlayer.SendBroadcast( ciBroadcast );
        }
        else if( 2 == ePhase )
        {
          // Increase Spirituality for talking to the Seer
          i_ciPlayer.AffectVirtue( VirtueType.Spirituality, 2, false, true );
        }
      }
    }

    i_ciPlayer.PopPacket();
  }
}


// Sent from server when player talks to an Ultima 1 King
class Player_Talk_U1_King_Broadcast extends rumBroadcast
{
  var1 = 0; // Type
  var2 = 0;
  var3 = 0;
  var4 = 0;

  static s_iQuestStatReward = 5;


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      var1 = vargv[0]; // Type
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


  function EndTransaction( i_ciPlayer )
  {
    local ciTarget = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
    if( ciTarget != null && ( ciTarget instanceof NPC ) )
    {
      ciTarget.InteractionEnd( i_ciPlayer );
    }
  }


  function IncreaseRandomStat( i_ciPlayer )
  {
    // Increase a random stat
    local ePropertyID = g_eU1StatPropertyArray[rand() % g_eU1StatPropertyArray.len()];
    i_ciPlayer.SetProperty( ePropertyID, i_ciPlayer.GetProperty( ePropertyID, 0 ) + s_iQuestStatReward );

    local ciBroadcast = ::rumCreate( Player_Talk_U1_King_BroadcastID, U1_KingTalkType.QuestReward,
                                     U1_KingQuestRewardType.Stat, ePropertyID, s_iQuestStatReward );
    i_ciPlayer.SendBroadcast( ciBroadcast );
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local ciTarget = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
    if( ciTarget != null && ( ciTarget instanceof NPC ) )
    {
      local ciMap = i_ciPlayer.GetMap();
      if( ciMap.GetTileDistance( i_ciPlayer.GetPosition(), ciTarget.GetPosition() ) <= 1 )
      {
        local eTalkType = var1;

        if( U1_KingTalkType.Pence == eTalkType )
        {
          local iAmount = var2;

          // Verify funds
          if( i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray ) >= iAmount )
          {
            i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iAmount );

            // Hitpoints received are 1.5 times the offer, rounded down
            local iBonus = ( iAmount * 1.5 ).tointeger();

            local ciBroadcast = ::rumCreate( Player_Talk_U1_King_BroadcastID, U1_KingTalkType.HPGranted, iBonus );
            i_ciPlayer.SendBroadcast( ciBroadcast );

            i_ciPlayer.AffectHitpoints( iBonus );
          }
          else
          {
            i_ciPlayer.IncrementHackAttempts();
          }
        }
        else if( U1_KingTalkType.Service == eTalkType )
        {
          local ciMap = i_ciPlayer.GetMap();
          local eQuestType = ciMap.GetProperty( U1_Quest_Type_PropertyID, null );
          if( eQuestType != null )
          {
            UpdateQuest( eQuestType, i_ciPlayer );
          }
        }
        else if( U1_KingTalkType.Return == eTalkType )
        {
          i_ciPlayer.ChangeWorld( GameType.Ultima4 );
        }
      }
    }

    EndTransaction( i_ciPlayer );
    i_ciPlayer.PopPacket();
  }


  function UpdateQuest( i_eQuestType, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local ePropertyID = g_eU1KingQuestPropertyArray[i_eQuestType];

    local eQuestState = i_ciPlayer.GetProperty( ePropertyID, U1_KingQuestState.Unbestowed1 );
    if( ( U1_KingQuestState.Unbestowed1 == eQuestState ) || ( U1_KingQuestState.Unbestowed2 == eQuestState ) )
    {
      // Advance and bestow the quest
      ++eQuestState;

      // Bestow the quest
      local ciBroadcast = ::rumCreate( Player_Talk_U1_King_BroadcastID, U1_KingTalkType.QuestBestowed,
                                       eQuestState, i_eQuestType );
      i_ciPlayer.SendBroadcast( ciBroadcast );

      i_ciPlayer.SetProperty( ePropertyID, eQuestState );
    }
    else if( ( U1_KingQuestState.Bestowed1 == eQuestState ) || ( U1_KingQuestState.Bestowed2 == eQuestState ) )
    {
      // Already on the quest
      local ciBroadcast = ::rumCreate( Player_Talk_U1_King_BroadcastID, U1_KingTalkType.QuestAlreadyBestowed );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
    else if( ( U1_KingQuestState.Completed1 == eQuestState ) || ( U1_KingQuestState.Completed2 == eQuestState ) )
    {
      if( ( U1_KingQuestType.LordBritish == i_eQuestType ) ||
          ( U1_KingQuestType.Barataria == i_eQuestType )   ||
          ( U1_KingQuestType.WhiteDragon == i_eQuestType ) ||
          ( U1_KingQuestType.Olympus == i_eQuestType ) )
      {
        IncreaseRandomStat( i_ciPlayer );

        if( U1_KingQuestState.Completed1 == eQuestState )
        {
          // Allow player to start the next quest
          i_ciPlayer.SetProperty( ePropertyID, U1_KingQuestState.Unbestowed2 );
        }
        else
        {
          // Reset so that quests are repeatable
          i_ciPlayer.SetProperty( ePropertyID, U1_KingQuestState.Unbestowed1 );
        }
      }
      else
      {
        if( U1_KingQuestState.Completed1 == eQuestState )
        {
          local strHint = format( "u1_king_quest_hint_%d_server_StringID", i_eQuestType );
          strHint = ::rumGetStringByName( strHint, i_ciPlayer.m_iLanguageID );

          // Hint
          local ciBroadcast = ::rumCreate( Player_Talk_U1_King_BroadcastID, U1_KingTalkType.QuestReward,
                                           U1_KingQuestRewardType.Hint, strHint );
          i_ciPlayer.SendBroadcast( ciBroadcast );

          // Allow player to start the next quest
          i_ciPlayer.SetProperty( ePropertyID, U1_KingQuestState.Unbestowed2 );
        }
        else if( U1_KingQuestState.Completed2 == eQuestState )
        {
          // Give the player a gem if they don't already have one. Otherwise, bump a random stat.
          local iGems = i_ciPlayer.GetProperty( U1_Gems_PropertyID, 0 );
          if( !::rumBitOn( iGems, i_eQuestType ) )
          {
            iGems = ::rumBitSet( iGems, i_eQuestType );
            i_ciPlayer.SetProperty( U1_Gems_PropertyID, iGems );

            local ciBroadcast = ::rumCreate( Player_Talk_U1_King_BroadcastID, U1_KingTalkType.QuestReward,
                                             U1_KingQuestRewardType.Gem, i_eQuestType );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
          else
          {
            IncreaseRandomStat( i_ciPlayer, i_eQuestType );
          }

          // Reset so that quests are repeatable
          i_ciPlayer.SetProperty( ePropertyID, U1_KingQuestState.Unbestowed1 );
        }
      }
    }
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

    if( vargv.len() > 0 )
    {
      var1 = vargv[0]; // Type
      if( vargv.len() > 1 )
      {
        var2 = vargv[1]; // String
        if( vargv.len() > 2 )
        {
          var3 = vargv[2]; // String
        }
      }
    }
  }
}


// Sent from server when player talks to Ultima 2 Hotel California Clerk
class Player_Talk_U2_HotelClerk_Broadcast extends rumBroadcast
{
  var1 = 0; // Type
  var2 = 0; // Amount
  var3 = 0; // StatIndex


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
        }
      }
    }
  }


  function EndTransaction( i_ciPlayer )
  {
    local ciTarget = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
    if( ciTarget != null && ( ciTarget instanceof NPC ) )
    {
      ciTarget.InteractionEnd( i_ciPlayer );
    }
  }


  function IncreaseRandomStat( i_ciPlayer )
  {
    // Increase a random stat
    local iIndex = rand() % g_eU2StatPropertyArray.len();
    local ePropertyID = g_eU2StatPropertyArray[iIndex];

    local iNewValue = i_ciPlayer.GetProperty( ePropertyID, 0 ) + U2_NPC.s_iClerkStatIncrementAmount;
    i_ciPlayer.SetProperty( ePropertyID, iNewValue );

    local ciBroadcast = ::rumCreate( Player_Talk_U2_HotelClerk_BroadcastID, U2_ClerkTalkType.RequestRoomTip,
                                     U2_NPC.s_iClerkStatIncrementAmount, iIndex );
    i_ciPlayer.SendBroadcast( ciBroadcast );
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local ciTarget = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
    if( ciTarget != null && ( ciTarget instanceof NPC ) )
    {
      local ciMap = i_ciPlayer.GetMap();
      if( ciMap.GetTileDistance( i_ciPlayer.GetPosition(), ciTarget.GetPosition() ) <= 2 )
      {
        local eTalkType = var1;

        if( U2_ClerkTalkType.RequestRoom == eTalkType )
        {
          // Verify funds
          if( i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray ) >= U2_NPC.s_iRoomCost )
          {
            // Charge player
            i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -U2_NPC.s_iRoomCost );

            VisitRoom( i_ciPlayer, rand()%4 );

            local iHealAmount = 100;

            local iHitpoints = i_ciPlayer.GetProperty( U2_Hitpoints_PropertyID, 0 );
            if( iHitpoints < 9950 )
            {
              if( iHitpoints < 5000 )
              {
                iHealAmount = 300;
              }
              else if( iHitpoints < 7500 )
              {
                iHealAmount = 200;
              }

              i_ciPlayer.AffectHitpoints( iHealAmount );
              i_ciPlayer.ActionSuccess( msg_morning_client_StringID );
            }
          }
        }
        else if( U2_ClerkTalkType.RequestRoomTip == eTalkType )
        {
          // TODO - move player to 1 of 4 rooms

          // Verify funds
          if( i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray ) >= U2_NPC.s_iRoomCostWithTip )
          {
            i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -U2_NPC.s_iRoomCostWithTip );

            VisitRoom( i_ciPlayer, rand()%4 );

            IncreaseRandomStat( i_ciPlayer );

            // Show cast effect on affected player
            SendClientEffect( i_ciPlayer, ClientEffectType.Cast );
          }
        }
        else
        {
          EndTransaction( i_ciPlayer );
        }
      }
    }

    i_ciPlayer.PopPacket();
  }


  function VisitRoom( i_ciPlayer, i_iRoomIndex )
  {
    local eRoomArray = null;
    local ciMap = i_ciPlayer.GetMap();

    // Determine the data table row
    local eMapArray = ::rumGetDataTableColumn( merchant_inn_DataTableID, 0 );
    local iRow = eMapArray.find( ciMap.GetAssetID() );
    if( iRow != null )
    {
      // Fetch the row and slice off the map column
      eRoomArray = ::rumGetDataTableRow( merchant_inn_DataTableID, iRow ).slice( 1 );
    }

    // Relocate the player
    local iRoomOffset = i_iRoomIndex * 3;
    local ciPos = rumPos( eRoomArray[iRoomOffset + 1], eRoomArray[iRoomOffset + 2] );
    ciMap.MovePawn( i_ciPlayer, ciPos,
                    rumIgnoreTileCollisionMoveFlag | rumIgnorePawnCollisionMoveFlag | rumIgnoreDistanceMoveFlag );
  }
}


// Sent from server when player talks to Ultima 2 Lord British
class Player_Talk_U2_LordBritish_Broadcast extends rumBroadcast
{
  var1 = 0; // Type
  var2 = 0; // Amount
  var3 = 0; // StatIndex


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
        }
      }
    }
  }


  function EndTransaction( i_ciPlayer )
  {
    local ciTarget = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
    if( ciTarget != null && ( ciTarget instanceof NPC ) )
    {
      ciTarget.InteractionEnd( i_ciPlayer );
    }
  }


  function IncreaseNextStat( i_ciPlayer, i_iIndex )
  {
    local iStartingIndex = i_iIndex++;
    if( iIndex > g_eU2StatPropertyArray.len() )
    {
      i_iIndex = 0;
    }

    local bIncreased = false;
    while( !bIncreased && iStartingIndex != i_iIndex )
    {
      local ePropertyID = g_eU2StatPropertyArray[iIndex];
      local iCurrentValue = i_ciPlayer.GetProperty( ePropertyID, 0 );

      local iCap = i_ciPlayer.GetStatCap( ePropertyID );
      if( iCurrentValue != iCap )
      {
        i_ciPlayer.SetProperty( ePropertyID, iCurrentValue + U2_NPC.s_iLBStatIncrementAmount );

        local ciBroadcast = ::rumCreate( Player_Talk_U2_LordBritish_BroadcastID, U2_LBTalkType.StatTribute,
                                         U2_NPC.s_iLBStatIncrementAmount, iIndex );
        i_ciPlayer.SendBroadcast( ciBroadcast );

        bIncreased = true;
      }
    }
  }


  function IncreaseRandomStat( i_ciPlayer )
  {
    // Increase a random stat
    local iIndex = rand() % g_eU2StatPropertyArray.len();
    local ePropertyID = g_eU2StatPropertyArray[iIndex];

    local iCurrentValue = i_ciPlayer.GetProperty( ePropertyID, 0 );
    local iCap = i_ciPlayer.GetStatCap( ePropertyID );
    if( iCurrentValue != iCap )
    {
      i_ciPlayer.SetProperty( ePropertyID, iCurrentValue + U2_NPC.s_iLBStatIncrementAmount );

      local ciBroadcast = ::rumCreate( Player_Talk_U2_LordBritish_BroadcastID, U2_LBTalkType.StatTribute,
                                       U2_NPC.s_iLBStatIncrementAmount, iIndex );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
    else
    {
      IncreaseNextStat( i_ciPlayer, iIndex );
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local ciTarget = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
    if( ciTarget != null && ( ciTarget instanceof NPC ) )
    {
      local ciMap = i_ciPlayer.GetMap();
      if( ciMap.GetTileDistance( i_ciPlayer.GetPosition(), ciTarget.GetPosition() ) <= 1 )
      {
        local eTalkType = var1;

        if( U2_LBTalkType.HitpointTribute == eTalkType )
        {
          // Verify funds
          if( i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray ) >= U2_NPC.s_iHitpointTributeCost )
          {
            // Charge player
            i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -U2_NPC.s_iHitpointTributeCost );

            local iHealAmount = 100;

            local iHitpoints = i_ciPlayer.GetProperty( U2_Hitpoints_PropertyID, 0 );
            if( iHitpoints < 9950 )
            {
              if( iHitpoints < 5000 )
              {
                iHealAmount = 300;
              }
              else if( iHitpoints < 7500 )
              {
                iHealAmount = 200;
              }

              local ciBroadcast = ::rumCreate( Player_Talk_U2_LordBritish_BroadcastID, U2_LBTalkType.HitpointTribute,
                                               iHealAmount );
              i_ciPlayer.SendBroadcast( ciBroadcast );

              i_ciPlayer.AffectHitpoints( iHealAmount );

              // Show cast effect on affected player
              SendClientEffect( i_ciPlayer, ClientEffectType.Cast );
            }
          }
          else
          {
            i_ciPlayer.IncrementHackAttempts();
          }
        }
        else if( U2_LBTalkType.StatTribute == eTalkType )
        {
          // Verify funds
          if( i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray ) >= U2_NPC.s_iStatTributeCost )
          {
            // Charge player
            i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -U2_NPC.s_iStatTributeCost );

            IncreaseRandomStat( i_ciPlayer );

            // Show cast effect on affected player
            SendClientEffect( i_ciPlayer, ClientEffectType.Cast );
          }
          else
          {
            i_ciPlayer.IncrementHackAttempts();
          }
        }
        else if( U2_LBTalkType.Return == eTalkType )
        {
          i_ciPlayer.ChangeWorld( GameType.Ultima4 );
        }
        else
        {
          EndTransaction( i_ciPlayer );
        }
      }
    }

    i_ciPlayer.PopPacket();
  }
}


class Player_Talk_U2_OldMan_Broadcast extends rumBroadcast
{
  var1 = 0; // Dialogue
  var2 = 0; // Ring MapID


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      var1 = vargv[0];
      if( vargv.len() > 1 )
      {
        var2 = vargv[1];
      }
    }
  }
}


// Sent from server when player talks to any Ultima 2 Oracle/Seer
class Player_Talk_U2_Oracle_Broadcast extends rumBroadcast
{
  var1 = 0; // Type
  var2 = 0; // Clue


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      var1 = vargv[0];
      if( vargv.len() > 1 )
      {
        var2 = vargv[1];
      }
    }
  }


  function EndTransaction( i_ciPlayer )
  {
    local ciTarget = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
    if( ciTarget != null && ( ciTarget instanceof NPC ) )
    {
      ciTarget.InteractionEnd( i_ciPlayer );
    }
  }


  function GetCost( i_ciPlayer )
  {
    local iOracleFlags = i_ciPlayer.GetProperty( U2_Oracle_Flags_PropertyID, 0 );
    local iNumOraclesPaid = PopCount( iOracleFlags );

    return 100 * iNumOraclesPaid + 100;
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local ciTarget = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
    if( ciTarget != null && ( ciTarget instanceof NPC ) )
    {
      local ciMap = i_ciPlayer.GetMap();
      if( ciMap.GetTileDistance( i_ciPlayer.GetPosition(), ciTarget.GetPosition() ) <= 2 )
      {
        local eTalkType = var1;
        if( U2_OracleTalkType.Pay == eTalkType )
        {
          local iCost = GetCost( i_ciPlayer );

          // Verify funds
          if( i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray ) >= iCost )
          {
            // Charge player
            i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -iCost );

            // Get the NPC the player is talking to
            local ciTarget = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
            if( ciTarget != null && ( ciTarget instanceof NPC ) )
            {
              // Remember that the player has paid
              local iOracleID = ciTarget.GetProperty( U2_Oracle_ID_PropertyID, 0 );
              if( iOracleID > 0 )
              {
                local iOracleFlags = i_ciPlayer.GetProperty( U2_Oracle_Flags_PropertyID, 0 );
                iOracleFlags = ::rumBitSet( iOracleFlags, iOracleID );
                i_ciPlayer.SetProperty( U2_Oracle_Flags_PropertyID, iOracleFlags );
              }

              // Show the Oracle's clue
              local strClue = format( "u2_oracle_%d_server_StringID", iOracleID - 1 );
              strClue = ::rumGetStringByName( strClue, i_ciPlayer.m_iLanguageID );

              local ciBroadcast = ::rumCreate( Player_Talk_U2_Oracle_BroadcastID, U2_OracleTalkType.Clue, strClue );
              i_ciPlayer.SendBroadcast( ciBroadcast );
            }
          }
        }
        else
        {
          EndTransaction( i_ciPlayer );
        }
      }
    }

    i_ciPlayer.PopPacket();
  }
}


class Player_Talk_U2_Sentri_Broadcast extends rumBroadcast
{
  var1 = 0; // Dialogue


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() > 0 )
    {
      var1 = vargv[0];
    }
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
        var2 = vargv[1];
      }
    }
  }


  function EndTransaction( i_ciPlayer )
  {
    local ciTarget = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
    if( ciTarget != null && ( ciTarget instanceof NPC ) )
    {
      ciTarget.InteractionEnd( i_ciPlayer );
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local ciTarget = ::rumFetchPawn( i_ciPlayer.m_uiInteractID );
    if( ciTarget != null && ( ciTarget instanceof NPC ) )
    {
      local ciMap = i_ciPlayer.GetMap();
      if( ciMap.GetTileDistance( i_ciPlayer.GetPosition(), ciTarget.GetPosition() ) <= 1 )
      {
        local eTalkType = var1;

        if( U3_LBTalkType.Report == eTalkType )
        {
          local iLevel = i_ciPlayer.GetProperty( U3_Level_PropertyID, 1 );
          local iMaxLevel = ::rumGetMaxPropertyValue( U3_Level_PropertyID );
          if( iLevel < iMaxLevel )
          {
            // See if the player has reached a new level
            local iExpLevel = i_ciPlayer.GetExperienceLevel();
            local iMarkFlags = i_ciPlayer.GetProperty( U3_Marks_PropertyID, 0 );
            local bHasMark = ::rumBitOn( iMarkFlags, U3_MarkType.King );
            if( iExpLevel > 5 && !bHasMark )
            {
              // The player cannot advance beyond level 5 until they've found the Mark of Kings
              local ciBroadcast = ::rumCreate( Player_Talk_U3_LordBritish_BroadcastID, U3_LBTalkType.SeekMark );
              i_ciPlayer.SendBroadcast( ciBroadcast );
              iExpLevel = 5;
            }

            if( iLevel < iExpLevel )
            {
              // Player has reached a new level
              local ciBroadcast = ::rumCreate( Player_Talk_U3_LordBritish_BroadcastID, U3_LBTalkType.LevelUp );
              i_ciPlayer.SendBroadcast( ciBroadcast );

              i_ciPlayer.SetProperty( U3_Level_PropertyID, iExpLevel );

              // Give the player a free heal
              i_ciPlayer.SetProperty( U3_Hitpoints_PropertyID, i_ciPlayer.GetMaxHitpoints() );

              // Show cast effect on affected player
              SendClientEffect( i_ciPlayer, ClientEffectType.Cast );
            }
            else
            {
              // The player needs more experience
              local ciBroadcast = ::rumCreate( Player_Talk_U3_LordBritish_BroadcastID, U3_LBTalkType.NeedExp );
              i_ciPlayer.SendBroadcast( ciBroadcast );
            }
          }
          else
          {
            // The player is at level cap
            local ciBroadcast = ::rumCreate( Player_Talk_U3_LordBritish_BroadcastID, U3_LBTalkType.LevelCap );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
        }
        else if( U3_LBTalkType.Return == eTalkType )
        {
          i_ciPlayer.ChangeWorld( GameType.Ultima4 );
        }
      }
    }

    EndTransaction( i_ciPlayer );
    i_ciPlayer.PopPacket();
  }
}


// Sent from server when player talks to Ultima 3 Timelord
class Player_Talk_U3_Timelord_Broadcast extends rumBroadcast
{
  var1 = 0;
  var2 = 0;
  var3 = 0;
  var4 = 0;


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() == 4 )
    {
      var1 = vargv[0]; // Card 1
      var2 = vargv[1]; // Card 2
      var3 = vargv[2]; // Card 3
      var4 = vargv[3]; // Card 4
    }
  }
}
