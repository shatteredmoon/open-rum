// Received from client when player meditates at a shrine
class Player_Meditate_Broadcast extends rumBroadcast
{
  static s_iDonation = 100;
  static s_iStatIncrease = 1;

  var1 = 0; // Meditation phase
  var2 = 0; // Virtue or Mantra
  var3 = 0; // Num Cycles


  constructor( ... )
  {
    base.constructor();

    if( 2 == vargv.len() )
    {
      var1 = vargv[0]; // Meditation phase | Shrine type
      var2 = vargv[1]; // Num cycles | Stat increase
    }
    if( 3 == vargv.len() )
    {
      var1 = vargv[0]; // Meditation phase
      var2 = vargv[1]; // Virtue or Mantra
      var3 = vargv[2]; // Num Cycles
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    local ciMap = i_ciPlayer.GetMap();
    if( null == ciMap )
    {
      return;
    }

    local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
    if( MapType.Shrine == eMapType )
    {
      local eVersion = i_ciPlayer.GetProperty( Ultima_Version_PropertyID, GameType.Invalid );
      if( GameType.Ultima4 == eVersion )
      {
        MeditateU4( i_ciPlayer, ciMap );
      }
      else if( GameType.Ultima3 == eVersion)
      {
        MeditateU3( i_ciPlayer, ciMap );
      }
    }
    else if( ciMap.GetAssetID() == U3_Towne_Yew_MapID )
    {
      // Is the player within the Circle of Light?
      local ciPos = i_ciPlayer.GetPosition();
      if( ciPos.x >= 45 && ciPos.x <= 51 && ciPos.y >= 45 && ciPos.y <= 51 )
      {
        local strDesc = ::rumGetString( u3_evocare_server_StringID, i_ciPlayer.m_iLanguageID );
        i_ciPlayer.ActionSuccess( strDesc, false );
      }
    }

    i_ciPlayer.PopPacket();
  }


  function MeditateU3( i_ciPlayer, i_ciMap )
  {
    local bCanMeditate = false;
    local ciPos = i_ciPlayer.GetPosition();
    local ciMap = i_ciPlayer.GetMap();

    // Is the player next to a shrine forcefield?
    local ciPawnArray = ciMap.GetPawns( ciPos, 1, false /* no LOS check */ );
    foreach( ciPawn in ciPawnArray )
    {
      if( ciPawn.IsVisible() && ( ciPawn.GetAssetID() == U3_Force_Field_WidgetID ) )
      {
        bCanMeditate = true;
        break;
      }
    }

    if( bCanMeditate )
    {
      // Does the player have the required gold?
      local iPlayerGold = i_ciPlayer.GetVersionedProperty( g_eGoldPropertyVersionArray );
      if( iPlayerGold >= s_iDonation )
      {
        // Can the player benefit from this shrine?
        local eShrineType = i_ciMap.GetProperty( U3_Shrine_Type_PropertyID, U3_ShrineType.Strength );
        local ePropertyID = g_eU3StatPropertyArray[eShrineType];
        local iStat = i_ciPlayer.GetProperty( ePropertyID, 1 );
        local iMaxValue = ::rumGetMaxPropertyValue( ePropertyID );
        if( iStat < iMaxValue )
        {
          i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, -s_iDonation );
          i_ciPlayer.SetProperty( ePropertyID, iStat + s_iStatIncrease );

          local ciBroadcast = ::rumCreate( Player_Meditate_BroadcastID, eShrineType, s_iStatIncrease );
          i_ciPlayer.SendBroadcast( ciBroadcast );
        }
        else
        {
          i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
        }
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
    }
  }


  function MeditateU4( i_ciPlayer, i_ciMap )
  {
    local ePhase = var1;
    local iNumCycles = var3;
    local eVirtue = i_ciMap.GetProperty( U4_Virtue_PropertyID, VirtueType.Honesty );

    if( U4_Meditation_Phase.Begin == ePhase )
    {
      local strPlayerVirtue = var2;
      local strShrineVirtue = ::rumGetString( g_eU4VirtueStringArray[eVirtue] );

      if( i_ciPlayer.GetProperty( U4_Weary_Mind_Cycle_Count_PropertyID, 0 ) > 0 )
      {
        // The player's mind isn't clear (the client should've caught this)
        i_ciPlayer.IncrementHackAttempts();
        i_ciPlayer.ActionFailed( msg_shrine_weary_client_StringID );
      }
      else
      {
        // The player's mind will be weary from the meditation for ten times as long as the number of cycles they
        // attempted to meditate
        i_ciPlayer.SetProperty( U4_Weary_Mind_Cycle_Count_PropertyID, iNumCycles * 10 );

        // Check that the player typed the correct virtue
        if( strPlayerVirtue.tolower() != strShrineVirtue.tolower() )
        {
          // Wrong virtue specified
          i_ciPlayer.ActionFailed( msg_shrine_focus_subject_client_StringID );
        }
        else
        {
          // Start meditation
          i_ciPlayer.SetProperty( U4_Meditating_PropertyID, true );
          i_ciPlayer.SetProperty( U4_Last_Meditation_Virtue_PropertyID, eVirtue );

          local ciBroadcast = ::rumCreate( Player_Meditate_BroadcastID, U4_Meditation_Phase.Begin, iNumCycles );
          i_ciPlayer.SendBroadcast( ciBroadcast );
        }
      }
    }
    else if( ePhase >= U4_Meditation_Phase.Cycle1 && ePhase <= U4_Meditation_Phase.Cycle3 )
    {
      local strPlayerMantra = var2;
      local strShrineMantra = format( "u4_mantra_%d_server_StringID", eVirtue );
      strShrineMantra = ::rumGetStringByName( strShrineMantra, i_ciPlayer.m_iLanguageID );
      if( strPlayerMantra.tolower() == strShrineMantra.tolower() )
      {
        if( iNumCycles == ePhase )
        {
          // Send reward string to player
          local strVision = format( "u4_shrine_vision_%d_%d_server_StringID", eVirtue, iNumCycles );
          strVision = ::rumGetStringByName( strVision, i_ciPlayer.m_iLanguageID );

          local ciBroadcast = ::rumCreate( Player_Meditate_BroadcastID, U4_Meditation_Phase.Vision, strVision );
          i_ciPlayer.SendBroadcast( ciBroadcast );

          local ePropertyID = rumInvalidAssetID;

          switch( iNumCycles )
          {
            case 1: ePropertyID = U4_Shrine_Cycle1_PropertyID; break;
            case 2: ePropertyID = U4_Shrine_Cycle2_PropertyID; break;
            case 3: ePropertyID = U4_Shrine_Cycle3_PropertyID; break;
          }

          if( ePropertyID != rumInvalidAssetID )
          {
            // Remember that the player has meditated for the current cycle
            local iFlags = i_ciPlayer.GetProperty( ePropertyID, 0 );
            iFlags = ::rumBitSet( iFlags, eVirtue );
            i_ciPlayer.SetProperty( ePropertyID, iFlags );

            // Check Seer Hawkwind elevation permission for the affected virtue
            iFlags = i_ciPlayer.GetProperty( U4_Seer_Bestowals_PropertyID, 0 );
            if( ::rumBitOn( iFlags, eVirtue ) )
            {
              // Has the player gained elevation for this virtue?
              local iFlags1 = i_ciPlayer.GetProperty( U4_Shrine_Cycle1_PropertyID, 0 );
              local iFlags2 = i_ciPlayer.GetProperty( U4_Shrine_Cycle2_PropertyID, 0 );
              local iFlags3 = i_ciPlayer.GetProperty( U4_Shrine_Cycle3_PropertyID, 0 );

              if( ::rumBitOn( iFlags1, eVirtue ) && ::rumBitOn( iFlags2, eVirtue ) && ::rumBitOn( iFlags3, eVirtue ) )
              {
                // The player has gained elevation for this virtue
                iFlags = i_ciPlayer.GetProperty( U4_Virtue_Elevation_PropertyID, 0 );
                iFlags = ::rumBitSet( iFlags, eVirtue );
                i_ciPlayer.SetProperty( U4_Virtue_Elevation_PropertyID, iFlags );

                // Send elevation notification to the player
                ciBroadcast = ::rumCreate( Player_Meditate_BroadcastID, U4_Meditation_Phase.Elevation, eVirtue );
                i_ciPlayer.SendBroadcast( ciBroadcast );
              }
            }
          }

          StopMeditation( i_ciPlayer );
        }
        else
        {
          // Continue next phase
          local ciBroadcast = ::rumCreate( Player_Meditate_BroadcastID, ePhase + 1, iNumCycles );
          i_ciPlayer.SendBroadcast( ciBroadcast );
        }
      }
      else
      {
        // Wrong mantra specified
        i_ciPlayer.ActionFailed( msg_shrine_focus_mantra_client_StringID );
        StopMeditation( i_ciPlayer );
      }
    }
    else if( U4_Meditation_Phase.Done == ePhase )
    {
      StopMeditation( i_ciPlayer );
    }
  }


  function StopMeditation( i_ciPlayer )
  {
    i_ciPlayer.RemoveProperty( U4_Meditating_PropertyID );
  }
}
