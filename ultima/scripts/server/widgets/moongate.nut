class U3_Moongate_Widget extends U3_Widget
{
  m_bGatingPlayer = false;
  m_iPhaseCount = 0;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );

    SetProperty( State_PropertyID, MoongateState.Closed );
  }


  function AffectCreature( i_ciCreature )
  {
    if( m_bGatingPlayer || i_ciCreature.GetMoveType() != MoveType.Terrestrial || !i_ciCreature.IsVisible() ||
        !( i_ciCreature instanceof Player ) )
    {
      return;
    }

    local ciPlayer = i_ciCreature;

    local eState = GetProperty( State_PropertyID, MoongateState.Closed );
    if( MoongateState.Closed == eState )
    {
      return;
    }

    // Guard against re-entry
    m_bGatingPlayer = true;

    // Affect the creature only if it's still on the same map and position
    local ciMap = GetMap();
    if( ( ciPlayer.GetMap() == ciMap ) && ciPlayer.GetPosition().Equals( GetPosition() ) )
    {
      local uiWidgetID = g_ciServer.m_ciUltima3World.m_uiMoongateArray[g_ciServer.m_ciUltima4World.m_eMoonFelucca];
      local ciMoongate = ::rumFetchPawn( uiWidgetID );
      if( ciMoongate != null )
      {
        local ciMoongatePos = ciMoongate.GetPosition();

        local ciTransport = ciPlayer.GetTransport();
        if( ciTransport != null )
        {
          ciTransport.Portal( U3_Sosaria_MapID, ciMoongatePos );

          local ciCommander = ciTransport.GetCommander();
          if( ciCommander != null )
          {
            SendClientEffect( ciPlayer, ClientEffectType.Cast );
          }
        }
        else
        {
          // Move without collision check
          ciMap.MovePawn( ciPlayer, ciMoongatePos,
                          rumIgnoreTileCollisionMoveFlag | rumIgnorePawnCollisionMoveFlag |
                          rumIgnoreDistanceMoveFlag );

          SendClientEffect( ciPlayer, ClientEffectType.Cast );
        }
      }
    }

    m_bGatingPlayer = false;
  }


  function OnAddedToMap()
  {
    local eTrammelPhase = GetProperty( Widget_Moongate_Phase_PropertyID, -1 );
    if( eTrammelPhase >= MoonPhase.New && eTrammelPhase < MoonPhase.NumPhases )
    {
      // We only care about moongates that are permanently added to each map
      g_ciServer.m_ciUltima3World.m_uiMoongateArray[eTrammelPhase] = GetID();
    }

    OnMoonPhaseChange();
  }


  function OnMoonPhaseChange()
  {
    local eState = GetProperty( State_PropertyID, MoongateState.Closed );

    if( MoongateState.Open == eState )
    {
      AffectAllPlayers();
    }

    //--------------------------------------------------------------------
    // Moongate State machine
    //--------------------------------------------------------------------
    local eTrammelPhase = GetProperty( Widget_Moongate_Phase_PropertyID, 0 );
    if( g_ciServer.m_ciUltima4World.m_eMoonTrammel == eTrammelPhase )
    {
      // Begin opening the moongate if it is closed
      if( MoongateState.Closed == eState )
      {
        SetProperty( State_PropertyID, MoongateState.Open );
        m_iPhaseCount = 1;
      }
      else
      {
        ++m_iPhaseCount;
        if( m_iPhaseCount > 3)
        {
          // Signal to the client that the moongate close animation should begin
          SetProperty( State_PropertyID, MoongateState.Closed );

          // Prevent sending the client more than one closing update
          m_iPhaseCount = 0;
        }
      }
    }
    else
    {
      if( eState != MoongateState.Closed )
      {
        SetProperty( State_PropertyID, MoongateState.Closed );
      }
    }
  }
}


class U3_Moongate_Alt_Widget extends U3_Widget
{
  m_bGatingPlayer = false;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );

    SetProperty( State_PropertyID, MoongateState.Open );
  }


  function AffectCreature( i_ciCreature )
  {
    if( m_bGatingPlayer || !IsVisible() || i_ciCreature.GetMoveType() != MoveType.Terrestrial ||
        !i_ciCreature.IsVisible() || !( i_ciCreature instanceof Player ) )
    {
      return;
    }

    local ciPlayer = i_ciCreature;

    local eState = GetProperty( State_PropertyID, MoongateState.Closed );
    if( MoongateState.Closed == eState )
    {
      return;
    }

    // Guard against re-entry
    m_bGatingPlayer = true;

    // Affect the creature only if it's still on the same map and position
    local ciMap = GetMap();
    if( ( ciPlayer.GetMap() == ciMap ) && ciPlayer.GetPosition().Equals( GetPosition() ) )
    {
      if( ciMap.GetAssetID() == U3_Castle_Fire_2_MapID )
      {
        local strArray =
          [
            ::rumGetString( u3_endgame_1_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_2_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_3_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_4_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_5_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_6_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_7_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_8_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_9_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_10_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_11_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_12_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_13_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_14_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_15_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_16_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_17_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_18_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_19_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_endgame_20_server_StringID, ciPlayer.m_iLanguageID ),
            ::rumGetString( u3_exodus_destroyed_server_StringID, ciPlayer.m_iLanguageID )
          ];

        local ciBroadcast = ::rumCreate( Player_Talk_U3_LordBritish_BroadcastID, U3_LBTalkType.Endgame, strArray );
        ciPlayer.SendBroadcast( ciBroadcast );
      }

      local ciDestPos = rumPos( GetProperty( Map_PosX_PropertyID, 0 ), GetProperty( Map_PosY_PropertyID, 0 ) );
      local eDestMapID = GetProperty( Map_ID_PropertyID, rumInvalidAssetID );
      if( eDestMapID != rumInvalidAssetID )
      {
        local ciTransport = ciPlayer.GetTransport();
        if( ciTransport != null )
        {
          ciTransport.Portal( eDestMapID, ciPos );

          local ciCommander = ciTransport.GetCommander();
          if( ciCommander != null )
          {
            SendClientEffect( ciPlayer, ClientEffectType.Cast );
          }
        }
        else
        {
          local ciDestMap = GetOrCreateMap( ciPlayer, eDestMapID );
          if( ciDestMap != null )
          {
            ciMap.TransferPawn( ciPlayer, ciDestMap, ciDestPos );
            SendClientEffect( ciPlayer, ClientEffectType.Cast );
          }
        }
      }
    }

    m_bGatingPlayer = false;
  }
}


class U4_Moongate_Widget extends U4_Widget
{
  m_iPhaseCount = 0;
  m_bPlayerCreated = false;
  m_bGatingPlayer = false;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );

    SetProperty( State_PropertyID, MoongateState.Closed );
  }


  function AffectCreature( i_ciCreature )
  {
    if( m_bGatingPlayer || i_ciCreature.GetMoveType() != MoveType.Terrestrial || !i_ciCreature.IsVisible() ||
        !( i_ciCreature instanceof Player ) )
    {
      return;
    }

    local ciPlayer = i_ciCreature;

    local eState = GetProperty( State_PropertyID, MoongateState.Closed );
    if( eState == MoongateState.Closed )
    {
      return;
    }

    // Guard against re-entry
    m_bGatingPlayer = true;

    // Affect the creature only if it's still on the same map and position
    local ciMap = GetMap();
    if( ( ciPlayer.GetMap() == ciMap ) && ciPlayer.GetPosition().Equals( GetPosition() ) )
    {
      // Special case for the Shrine of Spirituality
      if( ( MoonPhase.Full == g_ciServer.m_ciUltima4World.m_eMoonTrammel ) &&
          ( MoonPhase.Full == g_ciServer.m_ciUltima4World.m_eMoonFelucca ) )
      {
        local ciTransport = ciPlayer.GetTransport();
        if( null == ciTransport )
        {
          local eMapID = GetProperty( Map_ID_PropertyID, rumInvalidAssetID );
          if( eMapID != rumInvalidAssetID )
          {
            local ciDestMap = GetOrCreateMap( ciPlayer, eMapID );
            if( ciDestMap != null )
            {
              local eVirtue = ciDestMap.GetProperty( U4_Virtue_PropertyID, VirtueType.Honesty );
              if( ciPlayer.HasRune( eVirtue ) )
              {
                local ciDestPos = rumPos( 0, 0 );
                ciDestPos.x = GetProperty( Map_PosX_PropertyID, 0 );
                ciDestPos.y = GetProperty( Map_PosY_PropertyID, 0 );

                ciMap.TransferPawn( ciPlayer, ciDestMap, ciDestPos );
              }
              else
              {
                ciPlayer.ActionFailed( msg_missing_rune_client_StringID );
              }
            }
          }
        }
        else
        {
          ciPlayer.ActionFailed( msg_only_on_foot_client_StringID );
        }
      }
      else
      {
        local uiWidgetID = g_ciServer.m_ciUltima4World.m_uiMoongateArray[g_ciServer.m_ciUltima4World.m_eMoonFelucca];
        local ciMoongate = ::rumFetchPawn( uiWidgetID );
        if( ciMoongate != null )
        {
          local ciMoongatePos = ciMoongate.GetPosition();

          local ciTransport = ciPlayer.GetTransport();
          if( ciTransport != null )
          {
            ciTransport.Portal( U4_Britannia_MapID, ciMoongatePos );

            local ciCommander = ciTransport.GetCommander();
            if( ciCommander != null )
            {
              SendClientEffect( ciPlayer, ClientEffectType.Cast );
            }
          }
          else
          {
            // Move without collision check
            ciMap.MovePawn( ciPlayer, ciMoongatePos,
                            rumIgnoreTileCollisionMoveFlag | rumIgnorePawnCollisionMoveFlag |
                            rumIgnoreDistanceMoveFlag );

            SendClientEffect( ciPlayer, ClientEffectType.Cast );
          }
        }
      }
    }

    m_bGatingPlayer = false;
  }


  function Expire()
  {
    if( m_bPlayerCreated )
    {
      // Signal to the client that the moongate close animation should begin
      SetProperty( State_PropertyID, MoongateState.Closed );

      // Prevent sending the client more than one closing update
      m_iPhaseCount = 0;

      ::rumSchedule( this, Remove, Ultima4World.s_fMoonPhaseInterval );
    }
  }


  function OnAddedToMap()
  {
    local eTrammelPhase = GetProperty( Widget_Moongate_Phase_PropertyID, -1 );
    if( eTrammelPhase >= MoonPhase.New && eTrammelPhase < MoonPhase.NumPhases )
    {
      // We only care about moongates that are permanently added to each map
      g_ciServer.m_ciUltima4World.m_uiMoongateArray[eTrammelPhase] = GetID();
    }

    OnMoonPhaseChange();
  }


  function OnMoonPhaseChange()
  {
    local eState = GetProperty( State_PropertyID, MoongateState.Closed );

    if( MoongateState.Open == eState )
    {
      AffectAllPlayers();
    }

    //--------------------------------------------------------------------
    // Moongate State machine
    //--------------------------------------------------------------------
    if( m_bPlayerCreated )
    {
      // Begin opening the moongate if it is closed
      if( MoongateState.Closed == eState )
      {
        SetProperty( State_PropertyID, MoongateState.Open );
      }
    }
    else
    {
      local eTrammelPhase = GetProperty( Widget_Moongate_Phase_PropertyID, 0 );
      if( g_ciServer.m_ciUltima4World.m_eMoonTrammel == eTrammelPhase )
      {
        // Begin opening the moongate if it is closed
        if( MoongateState.Closed == eState )
        {
          SetProperty( State_PropertyID, MoongateState.Open );
          m_iPhaseCount = 1;
        }
        else
        {
          ++m_iPhaseCount;
          if( m_iPhaseCount > 3)
          {
            // Signal to the client that the moongate close animation should begin
            SetProperty( State_PropertyID, MoongateState.Closed );

            // Prevent sending the client more than one closing update
            m_iPhaseCount = 0;
          }
        }
      }
      else
      {
        if( eState != MoongateState.Closed )
        {
          SetProperty( State_PropertyID, MoongateState.Closed );
        }
      }
    }
  }


  function Remove()
  {
    if( m_bPlayerCreated )
    {
      local ciMap = GetMap();
      ciMap.RemovePawn( this );
    }
  }
}
