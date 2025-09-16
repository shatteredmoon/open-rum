// Sent from client when player meditates at a shrine
class Player_Meditate_Broadcast extends rumBroadcast
{
  static s_fMeditationUpdateTime = 1.5;
  static s_iDonation = 100;
  static s_iSubCycles = 30;

  var1 = 0; // Phase
  var2 = 0; // Num cycles
  var3 = 0;

  m_ePhase = 0;
  m_iSubCycleIndex = 0;


  constructor( ... )
  {
    base.constructor();

    if( vargv.len() >= 1 )
    {
      var1 = vargv[0]; // Phase

      if( vargv.len() >= 3 )
      {
        var2 = vargv[1]; // VirtueType enum
        var3 = vargv[2]; // Num cycles
      }
    }
  }


  function MantraCallback( i_strMantra )
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    if( ( null == i_strMantra ) || ( "" == i_strMantra ) )
    {
      MeditationInterrupted( msg_shrine_focus_mantra_client_StringID );
      return;
    }

    ShowString( i_strMantra + "<b>" );

    local iNumCycles = var2;
    local ciBroadcast = ::rumCreate( Player_Meditate_BroadcastID, m_ePhase, i_strMantra, iNumCycles );
    ::rumSendBroadcast( ciBroadcast );
  }


  function MeditationInterrupted( i_strReason )
  {
    local ciPlayer = ::rumGetMainPlayer();
    ciPlayer.m_ciMeditationInstance = null;

    ShowString( ::rumGetString( i_strReason ), g_strColorTagArray.Red );

    // Notify the server so that meditation will end
    local ciBroadcast = ::rumCreate( Player_Meditate_BroadcastID, U4_Meditation_Phase.Done );
    ::rumSendBroadcast( ciBroadcast );
  }


  function MeditationUpdate()
  {
    ++m_iSubCycleIndex;
    if( m_iSubCycleIndex < s_iSubCycles )
    {
      ::rumSchedule( this, MeditationUpdate, s_fMeditationUpdateTime );

      if( m_iSubCycleIndex == 1 )
      {
        ShowString( "." );
      }
      else
      {
        g_ciUI.m_ciGameTextView.PopText();
        local strOut = ".";
        for( local i = 1; i < m_iSubCycleIndex; ++i )
        {
          strOut += ".";
        }
        ShowString( strOut );
      }
    }
    else
    {
      // Get mantra from player
      local strPrompt = ::rumGetString( u4_command_meditate_mantra_client_StringID );
      ShowString( strPrompt );

      g_ciUI.m_ciGameInputTextBox.ShowPrompt( false );
      g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Text_Response, MantraCallback.bindenv( this ) );
    }
  }


  function OnKeyPressed()
  {
    g_ciUI.m_eOverrideGraphicID = rumInvalidAssetID;
  }


  function OnRecv()
  {
    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();
    local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
    if( MapType.Shrine != eMapType )
    {
      return;
    }

    if( GameType.Ultima3 == g_ciCUO.m_eVersion )
    {
      ShowString( ::rumGetString( msg_shazam_client_StringID ), g_strColorTagArray.Green );
      DoSpellEffect( ciPlayer, true );

      local strStatDesc = null;

      local eShrineType = var1;
      switch( eShrineType )
      {
        case U3_ShrineType.Strength:
          strStatDesc = ::rumGetString( msg_strength_increased_client_StringID );
          break;

        case U3_ShrineType.Dexterity:
          strStatDesc = ::rumGetString( msg_dexterity_increased_client_StringID );
          break;

        case U3_ShrineType.Intelligence:
          strStatDesc = ::rumGetString( msg_intelligence_increased_client_StringID );
          break;

        case U3_ShrineType.Wisdom:
          strStatDesc = ::rumGetString( msg_wisdom_increased_client_StringID );
          break;
      }

      if( strStatDesc != null )
      {
        local iStatIncrease = var2;
        local strDesc = format( strStatDesc, iStatIncrease );
        ShowString( strDesc, g_strColorTagArray.Green );
      }
    }
    if( GameType.Ultima4 == g_ciCUO.m_eVersion )
    {
      m_ePhase = var1;

      ciPlayer.m_ciMeditationInstance = this;

      if( U4_Meditation_Phase.Begin == m_ePhase )
      {
        local iNumCycles = var2;

        ShowString( ::rumGetString( u4_command_meditate_begin_client_StringID ), g_strColorTagArray.Blue );
        m_ePhase = U4_Meditation_Phase.Cycle1;
      }

      if( m_ePhase >= U4_Meditation_Phase.Cycle1 && m_ePhase <= U4_Meditation_Phase.Cycle3 )
      {
        ::rumSchedule( this, MeditationUpdate, s_fMeditationUpdateTime );
      }
      else if( U4_Meditation_Phase.Vision == m_ePhase )
      {
        local strVision = format( "%s<b><b>%s%s%s<b>",
                                  ::rumGetString( msg_shrine_vision_client_StringID ),
                                  g_strColorTagArray.Blue,
                                  var2,
                                  g_strColorTagArray.White );
        ShowString( strVision );
      }
      else if( U4_Meditation_Phase.Elevation == m_ePhase )
      {
        local eVirtue = var2;
        ShowString( ::rumGetString( msg_shrine_partial_client_StringID ) + "<b>" );

        g_ciUI.m_eOverrideGraphicID = U4_Visions_GraphicID;
        g_ciUI.m_ciOverrideGraphicOffset = rumPoint( 0, ciPlayer.GetProperty( U4_Last_Meditation_Virtue_PropertyID, 0 ) );
        g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.PressAnyKey, OnKeyPressed );
      }
    }
  }
}
