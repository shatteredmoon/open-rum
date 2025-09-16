class U4_Orb_Widget extends U4_Widget
{
  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Stationary );

    SetState( OrbState.Unused );
  }


  function Respawn()
  {
    // Players can once again use this orb
    SetState( OrbState.Unused );
    base.Respawn();
  }


  function Use( i_ciPlayer )
  {
    if( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() )
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      return;
    }

    local eState = GetState();
    if( OrbState.Used == eState )
    {
      // The object cannot be used by the player because it has recently been used by someone else. This should
      // not be considered a hack attempt since multiple players can attempt to touch an orb simultaneously,
      // allowing multiple requests to reach the server
      i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
      return;
    }

    local eOrbType = GetProperty( Widget_Orb_Type_PropertyID, 0 );

    // Has the player recently touched this kind of orb?
    local eLastOrbType = i_ciPlayer.GetProperty( U4_Last_Orb_Type_PropertyID, -1 );
    if( eOrbType == eLastOrbType )
    {
      // The player is immune to this orb's effects until the player touches a different kind of orb
      i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
      return;
    }

    // Will the player permanently benefit from touching this orb?
    local bBenefit = false;
    local ePlayerClassID = i_ciPlayer.GetProperty( U4_PlayerClass_PropertyID, U4_Avatar_Class_CustomID );
    local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );

    local iStrength = 0;
    if( eOrbType & OrbType.Strength )
    {
      iStrength = i_ciPlayer.GetProperty( U4_Strength_PropertyID, 0 );
      if( iStrength < ciPlayerClass.GetProperty( Class_Strength_Cap_PropertyID, 99 ) )
      {
        bBenefit = true;
      }
    }

    local iDexterity = 0;
    if( eOrbType & OrbType.Dexterity )
    {
      iDexterity = i_ciPlayer.GetProperty( U4_Dexterity_PropertyID, 0 );
      if( iDexterity < ciPlayerClass.GetProperty( Class_Dexterity_Cap_PropertyID, 99 ) )
      {
        bBenefit = true;
      }
    }

    local iIntelligence = 0;
    if( eOrbType & OrbType.Intelligence )
    {
      iIntelligence = i_ciPlayer.GetProperty( U4_Intelligence_PropertyID, 0 );
      if( iIntelligence < ciPlayerClass.GetProperty( Class_Intelligence_Cap_PropertyID, 99 ) )
      {
        bBenefit = true;
      }
    }

    if( !bBenefit )
    {
      // The player's stats are already capped as far as the permanent benefits this orb provides
      i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
      return;
    }

    // Temporarily disable this orb
    SetState( OrbState.Used );
    ScheduleRespawn();

    local iDamage = 0;

    if( eOrbType & OrbType.Strength )
    {
      i_ciPlayer.SetProperty( U4_Strength_PropertyID, iStrength + 5 );
      iDamage += 200;

      local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), OrbType.Strength );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }

    if( eOrbType & OrbType.Dexterity )
    {
      i_ciPlayer.SetProperty( U4_Dexterity_PropertyID, iDexterity + 5 );
      iDamage += 200;

      local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), OrbType.Dexterity );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }

    if( eOrbType & OrbType.Intelligence )
    {
      i_ciPlayer.SetProperty( U4_Intelligence_PropertyID, iIntelligence + 5 );
      iDamage += 200;

      local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), OrbType.Intelligence );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }

    i_ciPlayer.Damage( iDamage, this )

    // Save this orb type so that the player will have to touch another kind of orb to recieve benefits
    i_ciPlayer.SetProperty( U4_Last_Orb_Type_PropertyID, eOrbType );
  }
}
