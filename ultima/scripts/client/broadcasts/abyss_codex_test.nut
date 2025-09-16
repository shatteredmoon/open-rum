// Received from the server after looking at the Codex or interacting with the Codex chamber door
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


  function OnKeyPressed( i_ePhase )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();
    g_ciUI.m_eOverrideGraphicID = rumInvalidAssetID;

    local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, i_ePhase );
    ::rumSendBroadcast( ciBroadcast );
  }


  function OnKeyPressed2( i_ePhase )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, i_ePhase );
    ::rumSendBroadcast( ciBroadcast );
  }


  function QuestionCallback( ... )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    local ePhase = vargv[0];

    if( vargv.len() > 1 )
    {
      local strResponse = vargv[1];
      if( strResponse != null )
      {
        ShowString( strResponse );
      }
      else
      {
        ShowString( "" );
      }

      local ciBroadcast = ::rumCreate( Abyss_Codex_Test_BroadcastID, ePhase, strResponse );
      ::rumSendBroadcast( ciBroadcast );
    }
    else
    {
      // The player is locked into this test, so just push the input mode again
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Text_Response,
                                                Abyss_Codex_Test_Broadcast.QuestionCallback,
                                                ePhase );
    }
  }


  function OnRecv()
  {
    local ePhase = var1;

    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();

    if( U4_AbyssCodexPhaseType.ChamberDoor == ePhase )
    {
      ShowString( "" );
      ShowString( ::rumGetString( u4_codex_key_client_StringID ), g_strColorTagArray.Yellow );

      g_ciUI.m_eOverrideGraphicID = U4_TLC_Key_GraphicID;
      g_ciUI.m_ciOverrideGraphicOffset = rumPoint( 0, 0 );

      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.PressAnyKey, OnKeyPressed, ePhase );
    }
    else if( U4_AbyssCodexPhaseType.WordOfPassage == ePhase )
    {
      local strDesc;
      local iAttemptNum = ciPlayer.GetProperty( U4_Codex_Question_Attempt_PropertyID, 0 );
      if( 1 == iAttemptNum )
      {
        strDesc = format( "%s<b>%s", ::rumGetString( u4_codex_voice0_client_StringID ),
                                     ::rumGetString( u4_codex_word_of_passage_client_StringID ) );
      }
      else
      {
        strDesc = format( "%s<b>%s", ::rumGetString( u4_codex_question_missed_client_StringID ),
                                     ::rumGetString( u4_codex_word_of_passage_client_StringID ) );
      }

      ShowString( "" );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      g_ciUI.m_ciGameInputTextBox.Clear();
      g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Text_Response, QuestionCallback, ePhase );
    }
    else if( U4_AbyssCodexPhaseType.PassageGranted == ePhase )
    {
      ShowString( "" );
      ShowString( ::rumGetString( u4_codex_word_of_passage_granted_client_StringID ), g_strColorTagArray.Cyan );

      PlayMusic( U4_Shrines_SoundID );

      TestComplete();
    }
    else if( U4_AbyssCodexPhaseType.PassageNotGranted == ePhase )
    {
      ShowString( "" );
      ShowString( ::rumGetString( u4_codex_word_of_passage_failed_client_StringID ), g_strColorTagArray.Cyan );

      TestComplete();
    }
    else if( U4_AbyssCodexPhaseType.VirtuesPassed == ePhase )
    {
      ShowString( "" );
      ShowString( ::rumGetString( u4_codex_virtue_test_passed_client_StringID ), g_strColorTagArray.Cyan );

      g_ciUI.m_ciOverrideGraphicOffset = rumPoint( 0, VirtueType.Humility );
    }
    else if( U4_AbyssCodexPhaseType.PrinciplesPassed == ePhase )
    {
      ShowString( "" );
      ShowString( ::rumGetString( u4_codex_principle_test_passed_client_StringID ), g_strColorTagArray.Cyan );

      g_ciUI.m_ciOverrideGraphicOffset = rumPoint( 0, VirtueType.NumVirtues + PrincipleType.Courage );
    }
    else if( U4_AbyssCodexPhaseType.TestFailed == ePhase )
    {
      ShowString( "" );
      ShowString( ::rumGetString( u4_codex_test_failed_client_StringID ), g_strColorTagArray.Cyan );

      TestComplete();
    }
    else if( U4_AbyssCodexPhaseType.AxiomTestFailed == ePhase )
    {
      ShowString( "" );
      ShowString( ::rumGetString( u4_codex_axiom_test_failed_client_StringID ), g_strColorTagArray.Cyan );

      TestComplete();
    }
    else if( ePhase >= U4_AbyssCodexPhaseType.Virtue1 && ePhase <= U4_AbyssCodexPhaseType.Virtue8 )
    {
      // Determine which question to ask based on the current phase
      local eVirtue = ePhase - U4_AbyssCodexPhaseType.Virtue1;
      local strQuestion = format( "u4_codex_virtue_test%d_client_StringID", eVirtue );

      local strDesc;
      local iAttemptNum = ciPlayer.GetProperty( U4_Codex_Question_Attempt_PropertyID, 0 );
      if( 1 == iAttemptNum )
      {
        strDesc = format( "%s<b>%s", ::rumGetString( u4_codex_voice3_client_StringID ),
                                     ::rumGetStringByName( strQuestion ) );
      }
      else
      {
        strDesc = format( "%s<b>%s", ::rumGetString( u4_codex_question_missed_client_StringID ),
                                     ::rumGetStringByName( strQuestion ) );
      }

      ShowString( "" );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      if( ePhase > U4_AbyssCodexPhaseType.Virtue1 )
      {
        g_ciUI.m_eOverrideGraphicID = U4_Endgame_GraphicID;
        g_ciUI.m_ciOverrideGraphicOffset = rumPoint( 0, eVirtue - 1 );
      }

      g_ciUI.m_ciGameInputTextBox.Clear();
      g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Text_Response, QuestionCallback, ePhase );
    }
    else if( ePhase >= U4_AbyssCodexPhaseType.Principle1 && ePhase <= U4_AbyssCodexPhaseType.Principle3 )
    {
      // Determine which question to ask based on the current phase
      local ePrinciple = ePhase - U4_AbyssCodexPhaseType.Principle1;
      local strQuestion = format( "u4_codex_principle_test%d_client_StringID", ePrinciple );

      local strDesc;
      local iAttemptNum = ciPlayer.GetProperty( U4_Codex_Question_Attempt_PropertyID, 0 );
      if( 1 == iAttemptNum )
      {
        strDesc = format( "%s<b>%s", ::rumGetString( u4_codex_voice3_client_StringID ),
                                     ::rumGetStringByName( strQuestion ) );
      }
      else
      {
        strDesc = format( "%s<b>%s", ::rumGetString( u4_codex_question_missed_client_StringID ),
                                     ::rumGetStringByName( strQuestion ) );
      }

      ShowString( "" );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      g_ciUI.m_eOverrideGraphicID = U4_Endgame_GraphicID;
      g_ciUI.m_ciOverrideGraphicOffset = rumPoint( 0, ePhase - U4_AbyssCodexPhaseType.Virtue1 - 1 );

      g_ciUI.m_ciGameInputTextBox.Clear();
      g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Text_Response, QuestionCallback, ePhase );
    }
    else if( U4_AbyssCodexPhaseType.Axiom == ePhase )
    {
      local strDesc;
      local iAttemptNum = ciPlayer.GetProperty( U4_Codex_Question_Attempt_PropertyID, 0 );
      if( 1 == iAttemptNum )
      {
        strDesc = format( "%s<b>%s", ::rumGetString( u4_codex_voice2_client_StringID ),
                                     ::rumGetString( u4_codex_axiom_test_client_StringID ) );
      }
      else
      {
        strDesc = format( "%s<b>%s", ::rumGetString( u4_codex_question_missed_client_StringID ),
                                     ::rumGetString( u4_codex_axiom_test_client_StringID ) );
      }

      ShowString( "" );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      g_ciUI.m_ciGameInputTextBox.Clear();
      g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Text_Response, QuestionCallback, ePhase );
    }
    else if( U4_AbyssCodexPhaseType.Enlightenment == ePhase )
    {
      ShowString( "" );
      ShowString( ::rumGetString( u4_codex_axiom_test_passed_client_StringID ), g_strColorTagArray.Cyan );

      g_ciUI.m_eOverrideGraphicID = U4_Endgame_GraphicID;
      g_ciUI.m_ciOverrideGraphicOffset = rumPoint( 0, ePhase - U4_AbyssCodexPhaseType.Virtue1 - 1 );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.PressAnyKey, OnKeyPressed2, ePhase + 1 );
    }
    else if( ePhase >= U4_AbyssCodexPhaseType.Endgame1 && ePhase <= U4_AbyssCodexPhaseType.Endgame10 )
    {
      local strDesc = var2;
      ShowString( "" );
      ShowString( strDesc, g_strColorTagArray.Cyan );

      if( ePhase < U4_AbyssCodexPhaseType.Endgame6 )
      {
        g_ciUI.m_eOverrideGraphicID = U4_Endgame_GraphicID;
      }
      else if( U4_AbyssCodexPhaseType.Endgame6 == ePhase )
      {
        // TODO: Some way of showing just a black screen?
        // Set g_ciUI.m_eOverrideGraphicID to 1 and don't draw anything on the client in RenderGameOverride?
        g_ciUI.m_eOverrideGraphicID = rumInvalidAssetID;
      }
      else if( U4_AbyssCodexPhaseType.Endgame7 == ePhase )
      {
        g_ciUI.m_eOverrideGraphicID = U4_Transfer_GraphicID;
      }

      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.PressAnyKey, OnKeyPressed2, ePhase + 1 );
    }
    else if( U4_AbyssCodexPhaseType.Complete == ePhase )
    {
      TestComplete();
    }
  }


  function TestComplete()
  {
    ShowString( "" );
    g_ciUI.m_eOverrideGraphicID = rumInvalidAssetID;
    g_ciUI.m_ciGameInputTextBox.ResetInputMode( false );
  }
}
