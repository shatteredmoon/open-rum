class U4_Altar_Widget extends U4_Widget
{
  function Look()
  {
    local ciPlayer = ::rumGetMainPlayer();
    local ciMap = ciPlayer.GetMap();
    local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
    if( MapType.Altar == eMapType )
    {
      local strPrompt = ::rumGetString( u4_command_look_dungeon_altar_client_StringID );
      ShowString( strPrompt );

      local iFlags = ciPlayer.GetProperty( U4_Item_Stones_PropertyID, 0 );
      if( iFlags )
      {
        local strPrompt = ::rumGetString( u4_command_look_dungeon_altar_prompt_client_StringID );
        ShowString( strPrompt );

        g_ciUI.m_ciGameListView.Clear();
        g_ciUI.m_ciGameListView.EnableMultiSelect( 4, 4 );
        g_ciUI.m_ciGameListView.SetFormat( "0.05" );

        for( local eVirtue = 0; eVirtue < VirtueType.NumVirtues; ++eVirtue )
        {
          if( ::rumBitOn( iFlags, eVirtue ) )
          {
            local strStone = ::rumGetString( g_eU4StoneStringArray[eVirtue] );
            g_ciUI.m_ciGameListView.SetEntry( eVirtue, strStone );
          }
        }

        CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

        g_ciUI.m_ciGameListView.m_funcAccept = OnStonesInserted.bindenv( this );
        g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
        g_ciUI.m_ciGameListView.SetActive( true );
        g_ciUI.m_ciGameListView.Focus();
      }
    }
    else if( MapType.Abyss == eMapType )
    {
      base.Look();
    }
  }


  function OnStonesInserted()
  {
    local strDesc = "";

    local i_eStoneArray = g_ciUI.m_ciGameListView.GetSelectedKeys();
    for( local iIndex = 0; iIndex < i_eStoneArray.len(); ++iIndex )
    {
      strDesc += g_ciUI.m_ciGameListView.GetEntry( i_eStoneArray[iIndex] );
      if( ( iIndex + 2 ) == i_eStoneArray.len() )
      {
        strDesc += ", and ";
      }
      else if( ( iIndex + 1 ) < i_eStoneArray.len() )
      {
        strDesc += ", ";
      }
    }

    g_ciUI.m_ciGameListView.Clear();
    ShowString( strDesc );

    if( GetOriginType() == rumServerOriginType )
    {
      local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetID(), i_eStoneArray );
      ::rumSendBroadcast( ciBroadcast );
    }

    Ultima_ListSelectionEnd();
  }
}
