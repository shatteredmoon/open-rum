class U1_Shuttle_Widget extends Ship_Transport_Widget
{
  static s_fCountdownInterval = 1.0;
  static s_iCountdownStart = 10;

  m_iCountdown = 0;
  m_iLaunchStamp = 0;


  function AbortLaunchSequence()
  {
    // Get a new launch stamp
    ++m_iLaunchStamp;
  }


  function BeginLaunchSequence()
  {
    ShowString( ::rumGetString( msg_launch_initiated_client_StringID ), g_strColorTagArray.Yellow );

    m_iCountdown = s_iCountdownStart;
    Countdown( ++m_iLaunchStamp );
  }


  function Countdown( i_iLaunchStamp )
  {
    if( i_iLaunchStamp == m_iLaunchStamp )
    {
      if( m_iCountdown > 0 )
      {
        ::rumSchedule( this, Countdown, s_fCountdownInterval, m_iLaunchStamp );

        // Show countdown number
        local strCountdown = format( "%d...", m_iCountdown-- );
        ShowString( strCountdown, g_strColorTagArray.Yellow );
      }
      else
      {
        ShowString( ::rumGetString( msg_launch_lift_off_client_StringID ), g_strColorTagArray.Yellow );
      }
    }
  }
}
