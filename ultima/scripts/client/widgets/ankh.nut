class U4_Ankh_Widget extends U4_Widget
{
  function Look()
  {
    local strPrompt = ::rumGetString( u4_command_look_ankh_client_StringID );
    ShowString( strPrompt );

    g_ciUI.m_ciYesNoListView.Clear();
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                       rumKeypress.KeyY() );
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                       rumKeypress.KeyN() );

    g_ciUI.m_ciYesNoListView.m_funcAccept = MeditateCallback.bindenv( this );
    g_ciUI.m_ciYesNoListView.m_funcCancel = Ultima_ListSelectionEnd;
    g_ciUI.m_ciYesNoListView.SetActive( true );
    g_ciUI.m_ciYesNoListView.Focus();
  }


  function MeditateCallback()
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    // Push the player's response to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();
    if( YesNoResponse.Yes == eResponse )
    {
      local strPrompt = ::rumGetString( u4_command_meditate_virtue_client_StringID );
      ShowString( strPrompt );

      g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Text_Response, VirtueCallback.bindenv( this ) );
    }

    Ultima_ListSelectionEnd();
  }


  function VirtueCallback( i_strVirtue )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    if( null == i_strVirtue )
    {
      ShowString( "" );
      return;
    }

    ShowString( i_strVirtue + "<b>" );

    local strPrompt = ::rumGetString( u4_command_meditate_cycles_client_StringID );
    ShowString( strPrompt );

    g_ciUI.m_ciGameListView.Clear();
    g_ciUI.m_ciGameListView.DisableMultiSelect();
    g_ciUI.m_ciGameListView.SetFormat( "0.05" );
    g_ciUI.m_ciGameListView.SetEntry( 0, "0", rumKeypress.Key0() );
    g_ciUI.m_ciGameListView.SetEntry( 1, "1", rumKeypress.Key1() );
    g_ciUI.m_ciGameListView.SetEntry( 2, "2", rumKeypress.Key2() );
    g_ciUI.m_ciGameListView.SetEntry( 3, "3", rumKeypress.Key3() );
    g_ciUI.m_ciGameListView.ShowPrompt( false );

    CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

    g_ciUI.m_ciGameListView.m_funcAccept = CycleCallback;
    g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
    g_ciUI.m_ciGameListView.SetActive( true );
    g_ciUI.m_ciGameListView.Focus();

    g_ciUI.m_ciGameInputTextBox.m_vPayload = i_strVirtue;
  }


  function CycleCallback()
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    local strCycles = g_ciUI.m_ciGameListView.GetCurrentEntry();
    ShowString( strCycles + "<b>" );

    local iCycles = strCycles.tointeger();
    local ciPlayer = ::rumGetMainPlayer();

    if( iCycles > 0 && ciPlayer != null )
    {
      local iNumCycles = ciPlayer.GetProperty( U4_Weary_Mind_Cycle_Count_PropertyID, 0 );
      if( iNumCycles > 0 )
      {
        // The player is unable to focus
        ShowString( ::rumGetString( msg_shrine_weary_client_StringID ), g_strColorTagArray.Red );
      }
      else
      {
        // Send the meditation request to the server
        local strVirtue = g_ciUI.m_ciGameInputTextBox.m_vPayload;
        local ciBroadcast = ::rumCreate( Player_Meditate_BroadcastID, U4_Meditation_Phase.Begin, strVirtue, iCycles );
        ::rumSendBroadcast( ciBroadcast );
      }
    }

    Ultima_ListSelectionEnd();
  }
}