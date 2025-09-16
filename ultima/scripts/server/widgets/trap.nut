class U1_Trap_Widget extends U1_Widget
{
  static s_fBoulderTime = 20.0;
  static s_fPitTrapTime = 30.0;


  function AffectCreature( i_ciCreature )
  {
    if( i_ciCreature.GetMoveType() != MoveType.Terrestrial )
    {
      return;
    }

    if( GetState() == TrapState.Active && !i_ciCreature.IsDead() )
    {
      local bDisableTrap = false;

      local eTrapType = GetProperty( Widget_Trap_Type_PropertyID, TrapType.Rocks );
      if( TrapType.Rocks == eTrapType )
      {
        // Creature has a chance of evading the trap
        local bEvaded = i_ciCreature.SkillRoll( i_ciCreature.GetDexterity() );

        if( i_ciCreature instanceof Player )
        {
          local ciPlayer = i_ciCreature;
          ciPlayer.ActionWarning( msg_falling_rocks_client_StringID );

          if( bEvaded )
          {
            ciPlayer.ActionSuccess( msg_evaded_client_StringID );
          }
        }

        local ciBoulder = ::rumCreate( U4_Boulder_WidgetID );
        local ciMap = GetMap();
        if( ciMap.AddPawn( ciBoulder, GetPosition() ) )
        {
          ::rumSchedule( ciBoulder, ciBoulder.Expire, s_fBoulderTime );
        }

        bDisableTrap = true;

        if( !bEvaded )
        {
          local iDamage = 16 + rand() % 32;
          i_ciCreature.Damage( iDamage, this );
        }
      }
      else if( ( TrapType.Winds == eTrapType ) && ( i_ciCreature instanceof Player ) )
      {
        local ciPlayer = i_ciCreature;
        ciPlayer.ActionWarning( msg_strange_winds_client_StringID );

        // Put out torches - do not put out light spell - no skill roll
        local ciEffect = ciPlayer.GetEffect( Light_Effect );
        if( ciEffect && !ciEffect.m_bMagical )
        {
          ciPlayer.SetLightRange( 0 );
        }

        bDisableTrap = true;
      }
      else if( TrapType.Pit == eTrapType )
      {
        // Show the pit trap
        local ciMap = GetMap();
        local bFound = false;
        local ciPosData = ciMap.GetPositionData( i_ciCreature.GetPosition() );
        local ciExistingPit;
        while( ciExistingPit = ciPosData.GetNext( rumWidgetPawnType, U1_Pit_WidgetID ) )
        {
          if( ciExistingPit.IsVisible() )
          {
            bFound = true;
            ciPosData.Stop();
          }
        }

        if( !bFound )
        {
          // Create a temporarily visible pit trap
          local ciPit = ::rumCreate( U1_Pit_WidgetID );
          if( ciMap.AddPawn( ciPit, i_ciCreature.GetPosition() ) )
          {
            ::rumSchedule( ciPit, ciPit.Expire, s_fPitTrapTime );
          }
        }

        if( i_ciCreature instanceof Player )
        {
          local ciPlayer = i_ciCreature;
          ciPlayer.ActionWarning( msg_pit_trap_client_StringID );

          // Player has a chance of evading the trap
          local iRopes = ciPlayer.GetProperty( U1_Rope_Spikes_PropertyID, 0 );
          if( ( 0 == iRopes ) || !ciPlayer.SkillRoll( ciPlayer.GetDexterity() ) )
          {
            // Move the player down a level
            local eMapID = ciMap.GetProperty( Map_ID_PropertyID, rumInvalidAssetID );

            // Is the caster's position valid on the target level?
            local ciDestMap = GetOrCreateMap( ciPlayer, eMapID );
            if( ciDestMap != null )
            {
              local ciPos = ciPlayer.GetPosition();

              // Check for collisions and out-of-boundary conditions
              local eResult = ciTargetMap.MovePawn( ciPlayer, ciPos, rumTestMoveFlag );
              if( rumSuccessMoveResultType == eResult )
              {
                ciMap.TransferPawn( ciPlayer, ciDestMap, ciPos );
              }
            }

            local iDamage = 10 + rand() % 25;
            ciPlayer.Damage( iDamage, this );
          }
          else
          {
            // Consume rope & spikes if they were used
            if( iRopes > 1 )
            {
              ciPlayer.SetProperty( U1_Rope_Spikes_PropertyID, iRopes - 1 );
            }

            ciPlayer.ActionSuccess( msg_evaded_client_StringID );
          }

          // Note: Ultima 1 pit traps are never disabled!
        }
      }

      if( bDisableTrap )
      {
        // Disable the trap for a short time
        SetState( TrapState.Disarmed );
        SetVisibility( false );
        ScheduleRespawn();
      }
    }
  }


  function IsHarmful( i_ciCreature )
  {
    local eTrapType = GetProperty( Widget_Trap_Type_PropertyID, TrapType.Rocks );
    return IsVisible() && !( TrapType.Rocks != eTrapType || i_ciCreature.GetMoveType() != MoveType.Terrestrial );
  }


  function Respawn()
  {
    // Players can once again trigger this trap
    SetState( TrapState.Active );
    base.Respawn();
  }
}


class U3_Trap_Widget extends U3_Widget
{
  static s_fBoulderTime = 20.0;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );

    SetState( TrapState.Active );
  }


  function AffectCreature( i_ciCreature )
  {
    if( i_ciCreature.GetMoveType() != MoveType.Terrestrial )
    {
      return;
    }

    if( GetState() == TrapState.Active && !i_ciCreature.IsDead() )
    {
      local eTrapType = GetProperty( Widget_Trap_Type_PropertyID, 0 );
      if( ( TrapType.Winds == eTrapType ) && i_ciCreature instanceof Player )
      {
        local ciPlayer = i_ciCreature;
        ciPlayer.ActionWarning( msg_strange_winds_client_StringID );

        // Put out torches - do not put out light spell - no skill roll
        local ciEffect = ciPlayer.GetEffect( Light_Effect );
        if( ciEffect && !ciEffect.m_bMagical )
        {
          ciPlayer.SetLightRange( 0 );
        }
      }
      else if( TrapType.Pit == eTrapType )
      {
        // Creature has a chance of evading the trap
        local bEvaded = i_ciCreature.SkillRoll( i_ciCreature.GetDexterity() );

        if( i_ciCreature instanceof Player )
        {
          local ciPlayer = i_ciCreature;
          ciPlayer.ActionWarning( msg_pit_trap_client_StringID );

          if( bEvaded )
          {
            ciPlayer.ActionSuccess( msg_evaded_client_StringID );
          }
        }

        if( !bEvaded )
        {
          local iDamage = 10 + rand() % 25;
          i_ciCreature.Damage( iDamage, this );
        }
      }

      // Disable the trap for a short time
      SetState( TrapState.Disarmed );
      SetVisibility( false );
      ScheduleRespawn();
    }
  }


  function IsHarmful( i_ciCreature )
  {
    local eTrapType = GetProperty( Widget_Trap_Type_PropertyID, 0 );
    return IsVisible() && !( ( TrapType.Winds == eTrapType ) || i_ciCreature.GetMoveType() != MoveType.Terrestrial );
  }


  function Respawn()
  {
    // Players can once again trigger this trap
    SetState( TrapState.Active );
    base.Respawn();
  }
}


class U4_Trap_Widget extends U4_Widget
{
  static s_fBoulderTime = 20.0;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );

    SetState( TrapState.Active );
  }


  function AffectCreature( i_ciCreature )
  {
    if( i_ciCreature.GetMoveType() != MoveType.Terrestrial )
    {
      return;
    }

    if( GetState() == TrapState.Active && !i_ciCreature.IsDead() )
    {
      local eTrapType = GetProperty( Widget_Trap_Type_PropertyID, 0 );
      if( TrapType.Rocks == eTrapType )
      {
        // Creature has a chance of evading the trap
        local bEvaded = i_ciCreature.SkillRoll( i_ciCreature.GetDexterity() );

        if( i_ciCreature instanceof Player )
        {
          local ciPlayer = i_ciCreature;
          ciPlayer.ActionWarning( msg_falling_rocks_client_StringID );

          if( bEvaded )
          {
            ciPlayer.ActionSuccess( msg_evaded_client_StringID );
          }
        }

        local ciBoulder = ::rumCreate( U4_Boulder_WidgetID );
        local ciMap = GetMap();
        if( ciMap.AddPawn( ciBoulder, GetPosition() ) )
        {
          ::rumSchedule( ciBoulder, ciBoulder.Expire, s_fBoulderTime );
        }

        if( !bEvaded )
        {
          local iDamage = 16 + rand() % 32;
          i_ciCreature.Damage( iDamage, this );
        }
      }
      else if( ( TrapType.Winds == eTrapType ) && ( i_ciCreature instanceof Player ) )
      {
        local ciPlayer = i_ciCreature;
        ciPlayer.ActionWarning( msg_strange_winds_client_StringID );

        // Put out torches - do not put out light spell - no skill roll
        local ciEffect = ciPlayer.GetEffect( Light_Effect );
        if( ciEffect && !ciEffect.m_bMagical )
        {
          ciPlayer.SetLightRange( 0 );
        }
      }
      else if( TrapType.Pit == eTrapType )
      {
        // Creature has a chance of evading the trap
        local bEvaded = i_ciCreature.SkillRoll( i_ciCreature.GetDexterity() );

        if( i_ciCreature instanceof Player )
        {
          local ciPlayer = i_ciCreature;
          ciPlayer.ActionWarning( msg_pit_trap_client_StringID );

          if( bEvaded )
          {
            ciPlayer.ActionSuccess( msg_evaded_client_StringID );
          }
        }

        if( !bEvaded )
        {
          local iDamage = 10 + rand() % 25;
          i_ciCreature.Damage( iDamage, this );
        }
      }

      // Disable the trap for a short time
      SetState( TrapState.Disarmed );
      SetVisibility( false );
      ScheduleRespawn();
    }
  }


  function IsHarmful( i_ciCreature )
  {
    local eTrapType = GetProperty( Widget_Trap_Type_PropertyID, 0 );
    return IsVisible() && !( ( TrapType.Winds == eTrapType ) || i_ciCreature.GetMoveType() != MoveType.Terrestrial );
  }


  function Respawn()
  {
    // Players can once again trigger this trap
    SetState( TrapState.Active );
    base.Respawn();
  }
}
