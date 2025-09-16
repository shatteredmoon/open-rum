class U4_Telescope_Widget extends U4_Widget
{
  function Look()
  {
    local strDesc = ::rumGetString( u4_command_look_telescope_client_StringID );
    ShowString( strDesc );
  }
}
