class U1_Gem_Immortality_Widget extends U1_Widget
{
  static s_fVulnerableDuration = 10.0;


  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );

    // TODO - move to properties that are serialized
    SetProperty( State_PropertyID, GemImmortalityState.Invulnerable );
    SetProperty( Hitpoints_PropertyID, 9999 );
  }


  function Damage( i_iAmount, i_ciSource, i_eWeaponType = rumInvalidAssetID, i_bSendClientEffect = true )
  {
    if( i_iAmount <= 0 )
    {
      return false;
    }

    if( i_bSendClientEffect )
    {
      SendClientEffect( this, ClientEffectType.Damage );
    }

    local eState = GetProperty( State_PropertyID, GemImmortalityState.Invulnerable );
    if( GemImmortalityState.Vulnerable == eState )
    {
      local iHitpoints = GetProperty( Hitpoints_PropertyID, 0 ) - i_iAmount;
      SetProperty( Hitpoints_PropertyID, iHitpoints );

      if( iHitpoints <= 0 )
      {
        if( i_ciSource != null )
        {
          SendClientEffect( i_ciSource, ClientEffectType.ScreenShake );
          i_ciSource.ActionSuccess( msg_mondain_gem_destroyed_client_StringID );
        }

        SetVisibility( false );
      }

      return true;
    }

    // The Gem cannot be damaged while Mondain is alive
    return false;
  }


  function Invulnerable()
  {
    SetProperty( State_PropertyID, GemImmortalityState.Invulnerable );
  }


  function Open( i_ciPlayer, i_bUnused )
  {
    local eState = GetProperty( State_PropertyID, GemImmortalityState.Invulnerable );
    if( GemImmortalityState.Vulnerable == eState )
    {
      i_ciPlayer.Damage( 5000, this );
      if( !i_ciPlayer.IsDead() )
      {
        // The gem is destroyed
        SendClientEffect( i_ciPlayer, ClientEffectType.ScreenShake );
        i_ciPlayer.ActionSuccess( msg_mondain_gem_destroyed_client_StringID );

        SetVisibility( false );
      }
    }
    else
    {
      // The gem can't be taken unless it's vulnerable
      i_ciPlayer.Damage( rand()%400 + 100, this );
      i_ciPlayer.ActionFailed( msg_failed_client_StringID );
    }
  }


  function Vulnerable()
  {
    SetProperty( State_PropertyID, GemImmortalityState.Vulnerable );
    ::rumSchedule( this, Invulnerable, s_fVulnerableDuration );
  }
}
