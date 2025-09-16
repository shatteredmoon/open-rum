class U1_Mondain_Creature extends U1_NPC
{
  static s_fBatDuration = 10.0;
  static s_fCorpseDuration = 15.0;

  m_bPermaDeath = false;


  constructor()
  {
    base.constructor();
    SetProperty( Camouflaged_PropertyID, true );
  }


  function AIAttack( i_ciAttackTarget )
  {
    if( IsCamouflaged() )
    {
      // Come out of hiding
      SetProperty( Camouflaged_PropertyID, false );
    }

    return base.AIAttack( i_ciAttackTarget );
  }


  function CastSpell( i_eSpellID, i_eDirection, i_eType, i_ciTarget )
  {
    local eTokenID = rumInvalidStringToken;

    switch( i_eSpellID )
    {
      case U1_Mind_Blaster_Spell_CustomID:  eTokenID = msg_mondain_mind_blaster_client_StringID;  break;
      case U1_Psionic_Shock_Spell_CustomID: eTokenID = msg_mondain_psionic_shock_client_StringID; break;
      case U1_Magic_Missile_Spell_CustomID: eTokenID = msg_mondain_magic_missile_client_StringID; break;
    }

    if( eTokenID != rumInvalidStringToken )
    {
      // Send to all players
      local ciMap = GetMap();
      local ciPlayerArray = ciMap.GetAllPlayers();
      foreach( ciPlayer in ciPlayerArray )
      {
        ciPlayer.ActionInfo( eTokenID );
      }
    }

    base.CastSpell( i_eSpellID, i_eDirection, i_eType, i_ciTarget );
  }


  function ConvertToBat( i_ciSource )
  {
    if( !IsDead() )
    {
      m_uiFleeTargetID = i_ciSource.GetID();
      ::rumSchedule( this, ConvertToHumanoid, s_fBatDuration );
      SetProperty( State_PropertyID, MondainState.Transformed );
    }
  }


  function ConvertToHumanoid()
  {
    // Only convert back to human form when Mondain is a bat
    if( GetProperty( State_PropertyID, MondainState.Passive ) == MondainState.Transformed )
    {
      StopFleeing();
      SetProperty( State_PropertyID, MondainState.Aggressive );
    }
  }


  function Damage( i_iAmount, i_ciSource, i_eWeaponType = rumInvalidAssetID, i_bSendClientEffect = true )
  {
    local bDamaged = base.Damage( i_iAmount, i_ciSource, i_eWeaponType, i_bSendClientEffect );
    if( bDamaged && HasAttackTarget() )
    {
      local fRatio = GetHitpoints().tofloat() / GetMaxHitpoints().tofloat();
      if( fRatio < 0.4 )
      {
        ConvertToBat( i_ciSource );
      }
    }

    return bDamaged;
  }


  function GetGemOfImmortality()
  {
    local ciMap = GetMap();
    local ciPawnArray = ciMap.GetAllPawns();
    foreach( ciPawn in ciPawnArray )
    {
      if( ciPawn.GetAssetID() == U1_Gem_Immortality_WidgetID )
      {
        return ciPawn;
      }
    }

    return null;
  }


  function GetMaxAggroRange()
  {
    if( IsCamouflaged() )
    {
      return 1;
    }

    return s_iMaxAggroRange;
  }


  function GetSpell()
  {
    switch( rand() % 3 )
    {
      case 0: return U1_Mind_Blaster_Spell_CustomID;
      case 1: return U1_Psionic_Shock_Spell_CustomID;
      default:
        break;
    }

    return U1_Magic_Missile_Spell_CustomID;
  }


  function IsBat()
  {
    local eState = GetProperty( State_PropertyID, MondainState.Aggressive );
    return ( MondainState.Transformed == eState );
  }


  function KillFailed( i_ciCaster )
  {
    AffectHitpoints( rand() % GetMaxHitpoints() + 1 );

    if( i_ciCaster != null )
    {
      i_ciCaster.ActionFailed( msg_mondain_stronger_client_StringID );
    }
  }


  function OnDeath( i_ciSource, i_bKillCredit = true )
  {
    ForgetPath();

    SetProperty( State_PropertyID, MondainState.Corpse );

    local ciGemOfImmortality = GetGemOfImmortality();
    if( !ciGemOfImmortality )
    {
      return;
    }

    if( ciGemOfImmortality.IsVisible() )
    {
      // Mondain doesn't die if the gem has not yet been destroyed!
      ::rumSchedule( this, OnResurrect, s_fCorpseDuration );

      // With Mondain down, the gem can now be damaged
      ciGemOfImmortality.Vulnerable();

      i_ciSource.ActionSuccess( msg_mondain_dead_client_StringID );
    }
    else
    {
      SendClientEffect( i_ciSource, ClientEffectType.ScreenShake );

      local ciMap = GetMap();
      ciMap.StopExplosions();

      i_ciSource.ActionSuccess( msg_mondain_victory_client_StringID );

      // Remember that the player has killed Mondain
      local ciPlayerArray = ciMap.GetAllPlayers();
      foreach( ciPlayer in ciPlayerArray )
      {
        local iFlags = ciPlayer.GetProperty( Ultima_Completions_PropertyID, 0 );
        iFlags = ::rumBitSet( iFlags, UltimaCompletions.KilledMondain );
        ciPlayer.SetProperty( Ultima_Completions_PropertyID, iFlags );
      }

      m_bPermaDeath = true;
    }
  }


  function OnResurrect()
  {
    if( m_bPermaDeath )
    {
      return;
    }

    // Resurrect Mondain's corpse
    local iMaxHitpoints = GetProperty( Max_Hitpoints_PropertyID, 255 );
    SetProperty( Hitpoints_PropertyID, iMaxHitpoints );

    SetProperty( State_PropertyID, MondainState.Aggressive );
    m_uiFleeTargetID = rumInvalidGameID;

    AIDetermine( m_iActionIndex );

    local ciGemOfImmortality = GetGemOfImmortality();
    if( !ciGemOfImmortality )
    {
      return;
    }

    if( !ciGemOfImmortality.IsVisible() )
    {
      // Mondain can now be destroyed
      local ciMap = GetMap();
      local ciPlayerArray = ciMap.GetAllPlayers();
      foreach( ciPlayer in ciPlayerArray )
      {
        ciPlayer.ActionSuccess( msg_mondain_diminished_client_StringID );
      }
    }
  }


  function OnSteal( i_ciPlayer )
  {
    if( i_ciPlayer != null )
    {
      if( IsDead() || IsBat() )
      {
        i_ciPlayer.ActionFailed( msg_no_effect_client_StringID );
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_mondain_steal_client_StringID );
      }
    }
  }


  function Talk( i_ciPlayer )
  {
    if( i_ciPlayer != null )
    {
      if( IsDead() || IsBat() )
      {
        i_ciPlayer.ActionFailed( msg_no_response_client_StringID );
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_mondain_talk_client_StringID );
      }
    }
  }
}
