class U3_Whirlpool_Widget extends U3_Widget
{
  static s_fMoveTime = 5.0;

  m_eDestMapID = 0;
  m_ciDestPos = 0;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Aquatic );
    m_ciDestPos = rumPos();

    ::rumSchedule( this, Move, s_fMoveTime );
  }


  function AffectCreature( i_ciCreature )
  {
    if( !( i_ciCreature instanceof Player ) )
    {
      return;
    }

    local bFatal = true;

    local ciTransport = i_ciCreature.GetTransport();
    if( ciTransport != null )
    {
      if( TransportType.Ship == ciTransport.GetType() )
      {
        bFatal = false;

        // Is the transport still at this position?
        local ciPos = ciTransport.GetPosition();
        if( ciPos.Equals( GetPosition() ) )
        {
          // Get the destination map and location
          local ciMap = GetOrCreateMap( i_ciCreature, m_eDestMapID );
          if( ciMap != null && ciTransport.Portal( m_eDestMapID, m_ciDestPos ) )
          {
            foreach( uiPassengerID in ciTransport.m_uiPassengerTable )
            {
              local ciPassenger = ::rumGetPlayer( uiPassengerID );
              if( ciPassenger != null )
              {
                ciPassenger.ActionInfo( msg_whirlpool_ambrosia_client_StringID );
              }
            }

            if( ciMap instanceof U3_Ambrosia_Map )
            {
              ciTransport.Destroy();
            }
          }
        }
      }
    }
    else if( i_ciCreature.IsDead() )
    {
      // Whirlpool does not effect player ghosts
      bFatal = false;
    }

    if( bFatal )
    {
      // Kill the creature
      i_ciCreature.Kill( this, null, true );
    }
  }


  function IsHarmful( i_ciCreature )
  {
    return true;
  }


  function Move()
  {
    local ciMap = GetMap();
    ciMap.OffsetPawn( this, GetRandomDirectionVector() );

    ::rumSchedule( this, Move, s_fMoveTime );
  }


  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    base.OnPropertyUpdated( i_ePropertyID, i_vValue );

    if( Map_ID_PropertyID == i_ePropertyID )
    {
      m_eDestMapID = i_vValue;
    }
    else if( Map_PosX_PropertyID == i_ePropertyID )
    {
      m_ciDestPos.x = i_vValue;
    }
    else if( Map_PosY_PropertyID == i_ePropertyID )
    {
      m_ciDestPos.y = i_vValue;
    }
  }
}


class U4_Whirlpool_Widget extends U4_Widget
{
  static s_fMoveTime = 5.0;

  m_eDestMapID = 0;
  m_ciDestPos = 0;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Aquatic );
    m_ciDestPos = rumPos();

    ::rumSchedule( this, Move, s_fMoveTime );
  }


  function AffectCreature( i_ciCreature )
  {
    local bFatal = true;

    if( i_ciCreature instanceof Player )
    {
      local ciTransport = i_ciCreature.GetTransport();
      if( ciTransport != null )
      {
        if( TransportType.Ship == ciTransport.GetType() )
        {
          bFatal = false;

          // Is the transport still at this position?
          local ciPos = ciTransport.GetPosition();
          if( ciPos.Equals( GetPosition() ) )
          {
            // Get the destination map and location
            local ciMap = GetOrCreateMap( i_ciCreature, m_eDestMapID );
            if( ciMap != null )
            {
              ciTransport.Portal( m_eDestMapID, m_ciDestPos );
            }
          }
        }
        else if( TransportType.Balloon == ciTransport.GetType() )
        {
          bFatal = false;
        }
      }
      else if( i_ciCreature.IsDead() )
      {
        // Whirlpool does not effect player ghosts
        bFatal = false;
      }
    }

    if( bFatal )
    {
      // Kill the creature
      i_ciCreature.Kill( this, null, true );
    }
  }


  function IsHarmful( i_ciCreature )
  {
    return true;
  }


  function Move()
  {
    local ciMap = GetMap();
    ciMap.OffsetPawn( this, GetRandomDirectionVector() );

    ::rumSchedule( this, Move, s_fMoveTime );
  }


  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    base.OnPropertyUpdated( i_ePropertyID, i_vValue );

    if( Map_ID_PropertyID == i_ePropertyID )
    {
      m_eDestMapID = i_vValue;
    }
    else if( Map_PosX_PropertyID == i_ePropertyID )
    {
      m_ciDestPos.x = i_vValue;
    }
    else if( Map_PosY_PropertyID == i_ePropertyID )
    {
      m_ciDestPos.y = i_vValue;
    }
  }
}
