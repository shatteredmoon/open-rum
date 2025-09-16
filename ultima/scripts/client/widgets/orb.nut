class U4_Orb_Widget extends U4_Widget
{
  function GetDescription()
  {
    local eState = GetState();
    if( OrbState.Unused == eState )
    {
      return base.GetDescription();
    }

    return ::rumGetString( widget_orb_used_client_StringID );
  }


  function Look()
  {
    local eState = GetState();
    if( OrbState.Unused == eState )
    {
      local strPrompt = ::rumGetString( u4_command_look_orb_client_StringID );
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
