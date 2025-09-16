class U3_Fountain_Widget extends U4_Widget
{
  function Look()
  {
    local eState = GetState();
    if( FountainState.Flowing == eState )
    {
      local strPrompt = ::rumGetString( u4_command_look_fountain_client_StringID );
      ShowString( strPrompt );

      g_ciUI.m_ciYesNoListView.Clear();
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                         rumKeypress.KeyY() );
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                         rumKeypress.KeyN() );

      g_ciUI.m_ciYesNoListView.m_funcAccept = LookCallback.bindenv( this );
      g_ciUI.m_ciYesNoListView.m_funcCancel = Ultima_ListSelectionEnd;
      g_ciUI.m_ciYesNoListView.SetActive( true );
      g_ciUI.m_ciYesNoListView.Focus();
    }
  }
}


class U4_Fountain_Widget extends U4_Widget
{
  function Look()
  {
    local eState = GetState();
    if( FountainState.Flowing == eState )
    {
      local strPrompt = ::rumGetString( u4_command_look_fountain_client_StringID );
      ShowString( strPrompt );

      g_ciUI.m_ciYesNoListView.Clear();
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.Yes, ::rumGetString( talk_yes_client_StringID ),
                                         rumKeypress.KeyY() );
      g_ciUI.m_ciYesNoListView.SetEntry( YesNoResponse.No, ::rumGetString( talk_no_client_StringID ),
                                         rumKeypress.KeyN() );

      g_ciUI.m_ciYesNoListView.m_funcAccept = LookCallback.bindenv( this );
      g_ciUI.m_ciYesNoListView.m_funcCancel = Ultima_ListSelectionEnd;
      g_ciUI.m_ciYesNoListView.SetActive( true );
      g_ciUI.m_ciYesNoListView.Focus();
    }
  }
}
