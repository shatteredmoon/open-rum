class U4_Codex_Widget extends U4_Widget
{
  function Look()
  {
    local strPrompt = ::rumGetString( u4_command_look_codex_client_StringID );
    ShowString( strPrompt );

    g_ciUI.m_ciYesNoListView.Clear();
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                       rumKeypress.KeyY() );
    g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                       rumKeypress.KeyN() );

    g_ciUI.m_ciYesNoListView.m_funcAccept = OpenCallback.bindenv( this );
    g_ciUI.m_ciYesNoListView.m_funcCancel = Ultima_ListSelectionEnd;
    g_ciUI.m_ciYesNoListView.SetActive( true );
    g_ciUI.m_ciYesNoListView.Focus();
  }


  function OpenCallback()
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();
    if( ( YesNoResponse.Yes == eResponse ) && ( GetOriginType() == rumServerOriginType ) )
    {
      Ultima_ListSelectionEnd();

      PlayMusic( U4_Shrines_SoundID );
      BlockInput( RAND_MAX );

      local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetID() );
      ::rumSendBroadcast( ciBroadcast );
    }
    else
    {
      Ultima_ListSelectionEnd();
    }
  }
}