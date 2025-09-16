class U3_Mark_Rod_Widget extends U3_Widget
{
  function Look()
  {
    local strPrompt = ::rumGetString( u3_command_look_mark_client_StringID );
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
