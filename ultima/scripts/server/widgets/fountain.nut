class U3_Fountain_Widget extends U3_Widget
{
  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );

    SetState( FountainState.Flowing );
  }


  function Respawn()
  {
    // Players can once again use this fountain
    SetState( FountainState.Flowing );
  }


  function Use( i_ciPlayer )
  {
    if( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() )
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      return false;
    }

    local eState = GetState();
    if( FountainState.Dry == eState )
    {
      // The object cannot be used by the player because it has recently been used by someone else. This should
      // not be considered a hack attempt since multiple players can attempt to drink from a fountain
      // simultaneously, allowing multiple requests to reach the server
      i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
      return false;
    }

    // Temporarily disable this fountain
    SetState( FountainState.Dry );
    ScheduleRespawn();

    local eFountainType = GetProperty( Widget_Fountain_Type_PropertyID, FountainType.Health );
    switch( eFountainType )
    {
      case FountainType.Health:
        local iHitpoints = i_ciPlayer.GetHitpoints();
        local iMaxHitpoints = i_ciPlayer.GetMaxHitpoints();
        if( iHitpoints < iMaxHitpoints )
        {
          // Heal the player fully
          i_ciPlayer.SetVersionedProperty( g_eHitpointsPropertyVersionArray, iMaxHitpoints );
          i_ciPlayer.ActionSuccess( msg_refreshing_client_StringID );
        }
        else
        {
          i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
        }
        break;

      case FountainType.Poison:
        i_ciPlayer.ActionWarning( msg_argh_choke_gasp_client_StringID );
        i_ciPlayer.Poison( true );
        break;

      case FountainType.Cure:
        if( i_ciPlayer.IsPoisoned() )
        {
          // Cure the player
          local ciEffect = i_ciPlayer.GetEffect( Poison_Effect );
          if( ciEffect )
          {
            ciEffect.Remove();
          }

          i_ciPlayer.Cure();
          i_ciPlayer.ActionSuccess( msg_delicious_client_StringID );
        }
        else
        {
          i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
        }
        break;

      case FountainType.Acid:
        i_ciPlayer.ActionWarning( msg_bleck_nasty_client_StringID );
        i_ciPlayer.Damage( rand() % 100 + 100, this );
        break;
    }

    return true;
  }
}


class U4_Fountain_Widget extends U4_Widget
{
  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );

    SetState( FountainState.Flowing );
  }


  function Respawn()
  {
    // Players can once again use this fountain
    SetState( FountainState.Flowing );
  }


  function Use( i_ciPlayer )
  {
    if( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() )
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      return false;
    }

    local eState = GetState();
    if( FountainState.Dry == eState )
    {
      // The object cannot be used by the player because it has recently been used by someone else. This should
      // not be considered a hack attempt since multiple players can attempt to drink from a fountain
      // simultaneously, allowing multiple requests to reach the server
      i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
      return false;
    }

    // Temporarily disable this fountain
    SetState( FountainState.Dry );
    ScheduleRespawn();

    local eFountainType = GetProperty( Widget_Fountain_Type_PropertyID, FountainType.Health );
    switch( eFountainType )
    {
      case FountainType.Health:
        local iHitpoints = i_ciPlayer.GetHitpoints();
        local iMaxHitpoints = i_ciPlayer.GetMaxHitpoints();
        if( iHitpoints < iMaxHitpoints )
        {
          // Heal the player fully
          i_ciPlayer.SetVersionedProperty( g_eHitpointsPropertyVersionArray, iMaxHitpoints );
          i_ciPlayer.ActionSuccess( msg_refreshing_client_StringID );
        }
        else
        {
          i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
        }
        break;

      case FountainType.Poison:
        i_ciPlayer.ActionWarning( msg_argh_choke_gasp_client_StringID );
        i_ciPlayer.Poison( true );
        break;

      case FountainType.Cure:
        if( i_ciPlayer.IsPoisoned() )
        {
          // Cure the player
          local ciEffect = i_ciPlayer.GetEffect( Poison_Effect );
          if( ciEffect )
          {
            ciEffect.Remove();
          }

          i_ciPlayer.Cure();
          i_ciPlayer.ActionSuccess( msg_delicious_client_StringID );
        }
        else
        {
          i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
        }
        break;

      case FountainType.Mana:
        local iMana = i_ciPlayer.GetProperty( U4_Mana_PropertyID, 0 );
        local iMaxMana = i_ciPlayer.GetMaxMana();
        if( iMana < iMaxMana )
        {
          i_ciPlayer.SetProperty( U4_Mana_PropertyID, iMaxMana );
          i_ciPlayer.ActionSuccess( msg_invigorating_client_StringID );
        }
        else
        {
          i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
        }
        break;

      case FountainType.Acid:
        i_ciPlayer.ActionWarning( msg_bleck_nasty_client_StringID );
        i_ciPlayer.Damage( rand() % 100 + 100, this );
        break;
    }

    return true;
  }
}
