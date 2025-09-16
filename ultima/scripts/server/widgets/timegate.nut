class U2_Timegate_Widget extends U2_Widget
{
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

    local eState = GetProperty( State_PropertyID, MoongateState.Closed );
    if( MoongateState.Closed == eState )
    {
      return;
    }

    // Guard against re-entry
    m_bGatingPlayer = true;

    // Get the destination map and location
    local eDestMapID = GetProperty( Map_ID_PropertyID, rumInvalidAssetID );
    if( eDestMapID != rumInvalidAssetID )
    {
      local ciDestMap = GetOrCreateMap( i_ciCreature, eDestMapID );
      if( ciDestMap != null )
      {
        local iDestPosX = GetProperty( Map_PosX_PropertyID, 0 );
        local iDestPosY = GetProperty( Map_PosY_PropertyID, 0 );
        local ciDestPos = rumPos( iDestPosX, iDestPosY );

        // The player has successfully used the timegate
        local ciTransport = i_ciCreature.GetTransport();
        if( ciTransport != null )
        {
          ciTransport.Portal( ciDestMap, ciDestPos );
        }
        else
        {
          local ciMap = GetMap();
          ciMap.TransferPawn( i_ciCreature, ciDestMap, ciDestPos );
        }
      }
    }
  }


  function OnAddedToMap()
  {
    // Register this timegate with the ultima world
    local uiWidgetID = GetID();
    g_ciServer.m_ciUltima2World.m_uiTimeGateTable[uiWidgetID] <- uiWidgetID;

    OnPeriodChange();
  }


  function OnPeriodChange()
  {
    local eState = GetProperty( State_PropertyID, MoongateState.Closed );

    if( MoongateState.Open == eState )
    {
      AffectAllPlayers();
    }

    //--------------------------------------------------------------------
    // Moongate State machine
    //--------------------------------------------------------------------
    local ePeriod = GetProperty( Widget_Moongate_Phase_PropertyID, 0 );
    if( g_ciServer.m_ciUltima2World.m_eTimeGatePeriod == ePeriod )
    {
      // Begin opening the moongate if it is closed
      if( MoongateState.Closed == eState )
      {
        SetProperty( State_PropertyID, MoongateState.Open );
      }
    }
    else if( eState != MoongateState.Closed )
    {
      SetProperty( State_PropertyID, MoongateState.Closed );
    }
  }
}


class U2_Timegate_Facade_Widget extends U2_Widget
{
  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );
    SetProperty( State_PropertyID, MoongateState.Open );
  }
}
