// Received from the server after looking at an Abyss altar
class Abyss_Altar_Test_Broadcast extends rumBroadcast
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


  function OnRecv()
  {
    local ePhase = var1;

    if( U4_AbyssAltarPhaseType.Virtue == ePhase )
    {
      ShowString( "" );

      local ciPlayer = ::rumGetMainPlayer();
      local ciMap = ciPlayer.GetMap();
      local iLevel = ciMap.GetProperty( Map_Level_PropertyID, 0 );
      if( iLevel > 0 )
      {
        local strQuestion = format( "u4_command_look_abyss_altar_question_%d_client_StringID", iLevel );
        local strPrompt = format( "%s \"%s\"", ::rumGetString( u4_command_look_abyss_altar_client_StringID ),
                                               ::rumGetStringByName( strQuestion ) );
        ShowString( strPrompt, g_strColorTagArray.Cyan );

        g_ciUI.m_ciGameInputTextBox.Clear();
        g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
        g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Text_Response, QuestionCallback );
      }
    }
    else if( U4_AbyssAltarPhaseType.Stone == ePhase )
    {
      ShowString( "" );
      ShowString( ::rumGetString( u4_command_look_abyss_altar_prompt_client_StringID ), g_strColorTagArray.Cyan );

      local ciPlayer = ::rumGetMainPlayer();
      local iFlags = ciPlayer.GetProperty( U4_Item_Stones_PropertyID, 0 );
      if( iFlags )
      {
        g_ciUI.m_ciGameListView.Clear();
        g_ciUI.m_ciGameListView.DisableMultiSelect();
        g_ciUI.m_ciGameListView.SetFormat( "0.05" );

        local strEntry;
        for( local eVirtue = 0; eVirtue < VirtueType.NumVirtues; ++eVirtue )
        {
          if( ::rumBitOn( iFlags, eVirtue ) )
          {
            strEntry = ::rumGetString( g_eU4StoneStringArray[eVirtue] );
            g_ciUI.m_ciGameListView.SetEntry( eVirtue, strEntry );
          }
        }

        CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

        g_ciUI.m_ciGameListView.m_funcAccept = StoneCallback;
        g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
        g_ciUI.m_ciGameListView.SetActive( true );
        g_ciUI.m_ciGameListView.Focus();
      }
    }
    else if( U4_AbyssAltarPhaseType.Codex == ePhase )
    {
      ShowString( "" );
      ShowString( ::rumGetString( u4_codex_chamber_client_StringID ), g_strColorTagArray.Yellow );
    }
  }


  function StoneCallback()
  {
    // The player selected a stone
    local eStoneType = g_ciUI.m_ciGameListView.GetSelectedKey();

    local strDesc = g_ciUI.m_ciGameListView.GetCurrentEntry();
    ShowString( strDesc );

    local ciBroadcast = ::rumCreate( Abyss_Altar_Test_BroadcastID, U4_AbyssAltarPhaseType.Stone, eStoneType );
    ::rumSendBroadcast( ciBroadcast );

    Ultima_ListSelectionEnd();
  }


  function QuestionCallback( ... )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    if( vargv.len() > 0 )
    {
      // The player provided an answer, send it to the server for handling
      local strAnswer = vargv[0];
      ShowString( strAnswer );

      local strVirtue = strAnswer.tolower();

      // Rebuild the localized virtue table
      VirtueLoc.resize( VirtueType.NumVirtues );
      for( local eVirtue = 0; eVirtue < VirtueType.NumVirtues; ++eVirtue )
      {
        VirtueLoc[eVirtue] = ::rumGetString( g_eU4VirtueStringArray[eVirtue] ).tolower();
      }

      // See if the provided answer matches anything in the localized virtue table
      local eVirtue = -1;
      for( local i = 0; -1 == eVirtue, i < VirtueLoc.len(); ++i )
      {
        // Does the typed keyword match any virtues in the localized virtue table?
        if( strVirtue == VirtueLoc[i] )
        {
          // The index of the virtue in the table
          eVirtue = i;
        }
      }

      local ciBroadcast = ::rumCreate( Abyss_Altar_Test_BroadcastID, U4_AbyssAltarPhaseType.Virtue, eVirtue );
      ::rumSendBroadcast( ciBroadcast );
    }
    else
    {
      // The player did not respond with an answer
      ShowString( "" );
    }
  }
}
