// Sent to client when a player looks at the Codex or interacts with the Codex chamber door
class Abyss_Codex_Test_Broadcast extends rumBroadcast
{
  var1 = 0; // Test phase
  var2 = 0; // Response


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


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local ePhase = var1;

    local ciPos = i_ciPlayer.GetPosition();
    local ciMap = i_ciPlayer.GetMap();
    if( ciMap.GetAssetID() != U4_Dungeon_Codex_Chamber_MapID )
    {
      return;
    }

    if( null == var2 )
    {
      var2 = "";
    }

    local ciChamberDoor = null;

    if( ( U4_AbyssCodexPhaseType.ChamberDoor == ePhase ) ||
        ( U4_AbyssCodexPhaseType.WordOfPassage == ePhase ) )
    {
      local ciDoor = null;

      // Player must be adjacent to the chamber door
      local ciPawnArray = ciMap.GetPawns( ciPos, 1, false );
      foreach( ciPawn in ciPawnArray )
      {
        if( ciPawn instanceof U4_Door_Codex_Widget )
        {
          ciDoor = ciPawn;
          break;
        }
      }

      if( null == ciDoor )
      {
        return;
      }

      if( U4_AbyssCodexPhaseType.ChamberDoor == ePhase )
      {
        // Ask player for the Word of Passage - the player gets 3 attempts to get it right
        i_ciPlayer.SetProperty( U4_Codex_Question_Attempt_PropertyID, 1 );

        local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, U4_AbyssCodexPhaseType.WordOfPassage );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else if( U4_AbyssCodexPhaseType.WordOfPassage == ePhase )
      {
        // Verify that 'veramocor' was given (or whatever translation is being used)
        local strResponse = var2.tolower();
        local strWord = ::rumGetString( u4_codex_wordofpassage_server_StringID, i_ciPlayer.m_iLanguageID );
        if( strWord == strResponse )
        {
          i_ciPlayer.SetProperty( U4_Codex_Question_Attempt_PropertyID, 1 );

          // Move the player past the codex chamber door
          local ciNewPos = ciDoor.GetPosition() + GetDirectionVector( Direction.North );
          ciMap.MovePawn( i_ciPlayer, ciNewPos,
                          rumIgnoreTileCollisionMoveFlag | rumIgnorePawnCollisionMoveFlag |
                          rumIgnoreDistanceMoveFlag );

          SendClientEffect( i_ciPlayer, ClientEffectType.Cast );

          local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, U4_AbyssCodexPhaseType.PassageGranted );
          i_ciPlayer.SendBroadcast( ciBroadcast );
        }
        else
        {
          local iAttemptNum = i_ciPlayer.GetProperty( U4_Codex_Question_Attempt_PropertyID, 0 );
          if( 3 == iAttemptNum )
          {
            // Boot the player from the Abyss - this is brutal but part of the original!
            local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, U4_AbyssCodexPhaseType.PassageNotGranted );
            i_ciPlayer.SendBroadcast( ciBroadcast );

            local ciDestMap = GetOrCreateMap( i_ciPlayer, U4_Britannia_MapID );
            if( ciDestMap != null )
            {
              // Find the abyss entrance
              local ciPawnArray = ciDestMap.GetAllPawns();
              foreach( ciPawn in ciPawnArray )
              {
                if( ciPawn.GetAssetID() == U4_Volcano_PortalID )
                {
                  ciMap.TransferPawn( i_ciPlayer, ciDestMap, ciPawn.GetPosition() );
                  SendClientEffect( i_ciPlayer, ClientEffectType.Cast );
                  break;
                }
              }
            }
          }
          else
          {
            // Give the player another chance
            i_ciPlayer.SetProperty( U4_Codex_Question_Attempt_PropertyID, iAttemptNum + 1 );

            local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, ePhase );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
        }
      }
    }
    else
    {
      local ciCodex = null;

      // Player must be adjacent to the codex
      local ciPawnArray = ciMap.GetPawns( ciPos, 1, false );
      foreach( ciPawn in ciPawnArray )
      {
        if( ciPawn instanceof U4_Codex_Widget )
        {
          ciCodex = ciPawn;
          break;
        }
      }

      if( null == ciCodex )
      {
        return;
      }

      if( ePhase >= U4_AbyssCodexPhaseType.Virtue1 && ePhase <= U4_AbyssCodexPhaseType.Virtue8 )
      {
        local eVirtue = ePhase - U4_AbyssCodexPhaseType.Virtue1;
        local eStringID = g_eU4VirtueStringArray[eVirtue];

        // Verify that the correct virtue was given
        local strResponse = var2.tolower();
        local strWord = ::rumGetString( eStringID, i_ciPlayer.m_iLanguageID ).tolower();
        if( strWord == strResponse )
        {
          // Next question
          i_ciPlayer.SetProperty( U4_Codex_Question_Attempt_PropertyID, 1 );

          if( U4_AbyssCodexPhaseType.Virtue8 == ePhase )
          {
            local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, U4_AbyssCodexPhaseType.VirtuesPassed );
            i_ciPlayer.SendBroadcast( ciBroadcast );

            ::rumSchedule( i_ciPlayer, i_ciPlayer.StartPrincipleTest, 4.0 );
          }
          else
          {
            local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, ePhase + 1 );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
        }
        else
        {
          local iAttemptNum = i_ciPlayer.GetProperty( U4_Codex_Question_Attempt_PropertyID, 0 );
          if( 3 == iAttemptNum )
          {
            // Boot the player from the Abyss - this is brutal but part of the original!
            local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, U4_AbyssCodexPhaseType.TestFailed );
            i_ciPlayer.SendBroadcast( ciBroadcast );

            // Determine the destination based on the missed virtue question
            local ePlayerClassID = i_ciPlayer.GetProperty( U4_PlayerClass_PropertyID, U4_Mage_Class_CustomID );
            local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );
            local eDestMapID = ciPlayerClass.GetProperty( Class_Starting_Map_ID_PropertyID, rumInvalidAssetID );
            local ciDestMap = GetOrCreateMap( i_ciPlayer, eDestMapID );
            if( ciDestMap != null )
            {
              local ciNewPos = rumPos( ciPlayerClass.GetProperty( Class_Starting_Pos_X_PropertyID, 0 ),
                                       ciPlayerClass.GetProperty( Class_Starting_Pos_Y_PropertyID, 0 ) );
              ciMap.TransferPawn( i_ciPlayer, ciDestMap, ciNewPos );
            }
          }
          else
          {
            // Give the player another chance
            i_ciPlayer.SetProperty( U4_Codex_Question_Attempt_PropertyID, iAttemptNum + 1 );

            local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, ePhase );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
        }
      }
      else if( ePhase >= U4_AbyssCodexPhaseType.Principle1 && ePhase <= U4_AbyssCodexPhaseType.Principle3 )
      {
        local ePrinciple = ePhase - U4_AbyssCodexPhaseType.Principle1;

        // Verify that the correct principle was given
        local strResponse = var2.tolower();
        local strWord = ::rumGetString( g_eU4PrincipleStringArray[ePrinciple], i_ciPlayer.m_iLanguageID ).tolower();
        if( strWord == strResponse )
        {
          // Next question
          i_ciPlayer.SetProperty( U4_Codex_Question_Attempt_PropertyID, 1 );

          if( U4_AbyssCodexPhaseType.Principle3 == ePhase )
          {
            local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, U4_AbyssCodexPhaseType.PrinciplesPassed );
            i_ciPlayer.SendBroadcast( ciBroadcast );

            SendClientEffect( i_ciPlayer, ClientEffectType.ScreenShake );

            ::rumSchedule( i_ciPlayer, i_ciPlayer.StartAxiomTest, 4.0 );
          }
          else
          {
            local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, ePhase + 1 );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
        }
        else
        {
          local iAttemptNum = i_ciPlayer.GetProperty( U4_Codex_Question_Attempt_PropertyID, 0 );
          if( 3 == iAttemptNum )
          {
            // Boot the player from the Abyss - this is brutal but part of the original!
            local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, U4_AbyssCodexPhaseType.TestFailed );
            i_ciPlayer.SendBroadcast( ciBroadcast );

            local ciDestMap = null;

            // Determine the destination based on the missed principle question
            switch( ePhase )
            {
              case U4_AbyssCodexPhaseType.Principle1:
                ciDestMap = GetOrCreateMap( i_ciPlayer, U4_Keep_Lycaeum_MapID );
                break;

              case U4_AbyssCodexPhaseType.Principle2:
                ciDestMap = GetOrCreateMap( i_ciPlayer, U4_Keep_Empath_Abbey_MapID );
                break;

              case U4_AbyssCodexPhaseType.Principle3:
                ciDestMap = GetOrCreateMap( i_ciPlayer, U4_Keep_Serpents_Hold_MapID );
                break;
            }

            if( ciDestMap != null )
            {
              ciMap.TransferPawn( i_ciPlayer, ciDestMap, rumPos( 15, 31 ) );
            }
          }
          else
          {
            // Give the player another chance
            i_ciPlayer.SetProperty( U4_Codex_Question_Attempt_PropertyID, iAttemptNum + 1 );

            local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, ePhase );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
        }
      }
      else if( U4_AbyssCodexPhaseType.Axiom == ePhase )
      {
        // Verify that the correct axiom was given
        local strResponse = var2.tolower();
        local strWord = ::rumGetString( u4_codex_axiom_server_StringID );
        if( strWord == strResponse )
        {
          // The player has successfully finished the test
          local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, U4_AbyssCodexPhaseType.Enlightenment );
          i_ciPlayer.SendBroadcast( ciBroadcast );

          // Mark the completion
          local iFlags = i_ciPlayer.GetProperty( Ultima_Completions_PropertyID, 0 );
          iFlags = ::rumBitSet( iFlags, UltimaCompletions.CodexTest );
          i_ciPlayer.SetProperty( Ultima_Completions_PropertyID, iFlags );

          // The player is now an Avatar!
          i_ciPlayer.SetProperty( U4_PlayerClass_PropertyID, U4_Avatar_Class_CustomID );

          // Update the player's graphic
          local ciPlayerClass = ::rumGetCustomAsset( U4_Avatar_Class_CustomID );
          i_ciPlayer.SetProperty( U4_Graphic_ID_PropertyID,
                                  ciPlayerClass.GetProperty( Class_Graphic_ID_PropertyID, rumInvalidAssetID ) );
        }
        else
        {
          local iAttemptNum = i_ciPlayer.GetProperty( U4_Codex_Question_Attempt_PropertyID, 0 );
          if( 3 == iAttemptNum )
          {
            // Boot the player from the Abyss - this is brutal but part of the original!
            local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID,
                                             U4_AbyssCodexPhaseType.AxiomTestFailed );
            i_ciPlayer.SendBroadcast( ciBroadcast );

            local ciDestMap = GetOrCreateMap( i_ciPlayer, U4_Castle_British_1_MapID );
            if( ciDestMap != null )
            {
              ciMap.TransferPawn( i_ciPlayer, ciDestMap, rumPos( 15, 31 ) );
            }
          }
          else
          {
            // Give the player another chance
            i_ciPlayer.SetProperty( U4_Codex_Question_Attempt_PropertyID, iAttemptNum + 1 );

            local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, ePhase );
            i_ciPlayer.SendBroadcast( ciBroadcast );
          }
        }
      }
      else if( ePhase >= U4_AbyssCodexPhaseType.Endgame1 && ePhase <= U4_AbyssCodexPhaseType.Endgame10 )
      {
        local iOffset = ePhase - U4_AbyssCodexPhaseType.Endgame1 + 1;
        local strEndgameText = format( "u4_codex_endgame%d_server_StringID", iOffset );
        strEndgameText = ::rumGetStringByName( strEndgameText, i_ciPlayer.m_iLanguageID );

        local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, ePhase, strEndgameText );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else if( U4_AbyssCodexPhaseType.Complete == ePhase )
      {
        local ciDestMap = GetOrCreateMap( i_ciPlayer, U4_Castle_British_1_MapID );
        if( ciDestMap != null )
        {
          ciMap.TransferPawn( i_ciPlayer, ciDestMap, rumPos( 15, 31 ) );
        }

        local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, ePhase );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
    }

    i_ciPlayer.PopPacket();
  }
}
