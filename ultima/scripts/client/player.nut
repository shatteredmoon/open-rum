class Player extends Creature
{
  static s_strDescription = "creature_player";
  static s_fDeathFadeInterval = 1.0;

  static s_fNotorietyReductionInterval = 30.0;

  lastKeyword = "";

  m_ciMeditationInstance = null;

  m_ciNameLabel = null;

  m_iDeathIndex = 0;


  constructor()
  {
    base.constructor();

    SetMoveType( MoveType.Terrestrial );

    // The player should appear above most things, except for attacks and damage indicators
    SetDrawOrder( -50 );
  }


  function AmbrosiaWelcome()
  {
    ShowString( ::rumGetString( msg_ambrosia_welcome_client_StringID ), g_strColorTagArray.Yellow );
  }


  function CastFailed( i_eReason )
  {
    ShowString( ::rumGetString( i_eReason ), g_strColorTagArray.Red );
    PlaySound( Player_Miss_SoundID );
  }


  function CastSpell( i_eSpellID )
  {
    local eDelay = ActionDelay.Short;
    local bDelay = true;
    local bDone = true;

    // The caster cannot cast if affected by a Negation spell
    if( IsNegated() )
    {
      CastFailed( msg_cant_client_StringID );
    }
    else
    {
      local ciSpell = ::rumGetCustomAsset( i_eSpellID );
      if( null == ciSpell )
      {
        CastFailed( msg_cant_client_StringID );
      }

      local ePropertyID = ciSpell.GetProperty( Spell_ID_PropertyID, 0 );
      local iNumSpells = GetProperty( ePropertyID, 0 );
      if( g_ciCUO.m_eVersion != GameType.Ultima3 && ( 0 == iNumSpells ) )
      {
        CastFailed( msg_none_owned_client_StringID );
      }
      else
      {
        local iManaCost = ciSpell.GetProperty( Spell_Mana_Cost_PropertyID );

        local iMana = GetMana();
        if( iMana >= iManaCost )
        {
          local bDirectional = ciSpell.GetProperty( Spell_Directional_PropertyID );
          local bTargetable = ciSpell.GetProperty( Spell_Targetable_PropertyID );

          local strSpell = ::rumGetStringByName( ciSpell.GetName() + "_client_StringID" );

          if( U4_Energy_Field_Spell_CustomID == i_eSpellID )
          {
            local strDesc = format( "%s: %s", ::rumGetString( token_spell_client_StringID ), strSpell );
            ShowString( strDesc );

            g_ciUI.m_ciGameListView.Clear();
            g_ciUI.m_ciGameListView.DisableMultiSelect();
            g_ciUI.m_ciGameListView.SetFormat( "0.05" );

            g_ciUI.m_ciGameListView.SetEntry( U4_MagicFieldType.Fire,
                                              ::rumGetString( u4_energy_field_type_fire_client_StringID ),
                                              rumKeypress.KeyA() );
            g_ciUI.m_ciGameListView.SetEntry( U4_MagicFieldType.Lightning,
                                              ::rumGetString( u4_energy_field_type_lightning_client_StringID ),
                                              rumKeypress.KeyB() );
            g_ciUI.m_ciGameListView.SetEntry( U4_MagicFieldType.Poison,
                                              ::rumGetString( u4_energy_field_type_poison_client_StringID ),
                                              rumKeypress.KeyC() );
            g_ciUI.m_ciGameListView.SetEntry( U4_MagicFieldType.Sleep,
                                              ::rumGetString( u4_energy_field_type_sleep_client_StringID ),
                                              rumKeypress.KeyD() );

            CalculateListViewSize( g_ciUI.m_ciGameListView, "default", 6 );

            g_ciUI.m_ciGameListView.m_funcAccept = EnergyFieldSelected.bindenv( this );
            g_ciUI.m_ciGameListView.m_funcCancel = Ultima_ListSelectionEnd;
            g_ciUI.m_ciGameListView.m_funcIndexChanged = EnergyFieldSelectionChanged.bindenv( this );
            g_ciUI.m_ciGameListView.SetActive( true );
            g_ciUI.m_ciGameListView.Focus();

            bDelay = false;
            bDone = false;
          }
          else if( bDirectional )
          {
            g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Direction,
                                                      CastSpellDirection.bindenv( this ),
                                                      i_eSpellID );

            local strDesc = format( "%s: %s: <%s>",
                                    ::rumGetString( token_spell_client_StringID ),
                                    strSpell,
                                    ::rumGetString( token_direction_client_StringID ) );
            g_ciUI.m_ciGameInputTextBox.SetText( strDesc );

            bDelay = false;
            bDone = false;
          }
          else if( bTargetable )
          {
            g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Target, CastSpellTarget.bindenv( this ), i_eSpellID );

            local strDesc = format( "%s: %s: <%s>",
                                    ::rumGetString( token_spell_client_StringID ),
                                    strSpell,
                                    ::rumGetString( token_target_client_StringID ) );
            g_ciUI.m_ciGameInputTextBox.SetText( strDesc );

            local iRange = ciSpell.GetProperty( Spell_Range_PropertyID, 1 );
            local bFriendly = ciSpell.GetProperty( Spell_Altruistic_PropertyID, false );
            InitTarget( iRange, bFriendly ? ComparatorFriendly : ComparatorEnemy );

            bDelay = false;
            bDone = false;
          }
          else
          {
            local ciBroadcast = ::rumCreate( Player_Cast_BroadcastID, i_eSpellID );
            ::rumSendBroadcast( ciBroadcast );

            eDelay = ActionDelay.Medium;
          }
        }
        else
        {
          CastFailed( msg_low_mana_client_StringID );
        }
      }
    }

    if( bDelay )
    {
      BlockInput( GetActionDelay( eDelay ) );
    }

    return bDone;
  }


  function CastSpellDirection( i_eSpellID, i_eDir )
  {
    local ciBroadcast = ::rumCreate( Player_Cast_BroadcastID, i_eSpellID, i_eDir );
    ::rumSendBroadcast( ciBroadcast );

    CastSpellFinished( ActionDelay.Medium );
  }


  function CastSpellTarget( i_eSpellID, i_uiTargetID )
  {
    if( i_uiTargetID != rumInvalidGameID )
    {
      // Send the target's ID
      local ciBroadcast = ::rumCreate( Player_Cast_BroadcastID, i_eSpellID, Direction.None, null, i_uiTargetID );
      ::rumSendBroadcast( ciBroadcast );

      CastSpellFinished( ActionDelay.Medium );
    }
  }


  function EnergyFieldDirectionSpecified( i_eFieldType, i_eDir )
  {
    local ciBroadcast = ::rumCreate( Player_Cast_BroadcastID, U4_Energy_Field_Spell_CustomID, i_eDir, i_eFieldType );
    ::rumSendBroadcast( ciBroadcast );

    U4_CastSpellFinished( ActionDelay.Long );
  }


  function EnergyFieldSelected()
  {
    local strDesc = g_ciUI.m_ciGameInputTextBox.GetText();

    local eFieldType = g_ciUI.m_ciGameListView.GetSelectedKey();
    U4_CastSpellFinished( ActionDelay.None );

    g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.Direction,
                                              EnergyFieldDirectionSpecified.bindenv( this ),
                                              eFieldType );

    strDesc = format( "%s: <%s>", strDesc, ::rumGetString( token_direction_client_StringID ) );
    g_ciUI.m_ciGameInputTextBox.SetText( strDesc );
  }


  function EnergyFieldSelectionChanged( i_iIndex )
  {
    local strSpell = g_ciUI.m_ciGameListView.GetCurrentEntry();
    if( strSpell )
    {
      local strFmt = format( "%s: %s", ::rumGetString( u4_spell_energy_field_type_client_StringID ), strSpell );
      g_ciUI.m_ciGameInputTextBox.SetText( strFmt );
    }
    else
    {
      g_ciUI.m_ciGameInputTextBox.Clear();
    }
  }


  function GetCharisma()
  {
    return GetVersionedProperty( g_eCharismaPropertyVersionArray, null );
  }


  function GetDexterity()
  {
    return GetVersionedProperty( g_eDexterityPropertyVersionArray, null );
  }


  function GetDiscountPercent()
  {
    // At 99 Charisma points, players can expect a 40% discount on all merchant transactions
    local iCharisma = GetCharisma();
    return iCharisma != null ? iCharisma / 2.475 / 100.0 : 0;
  }


  function GetEquippedArmour()
  {
    local uiArmourID = GetEquippedArmourID();
    if( uiArmourID != rumInvalidGameID )
    {
      return GetInventory( uiArmourID );
    }

    return null;
  }


  function GetEquippedArmourID()
  {
    return GetProperty( Equipped_Armour_PropertyID, rumInvalidGameID );
  }


  function GetEquippedWeapon()
  {
    local uiWeaponID = GetEquippedWeaponID();
    if( uiWeaponID != rumInvalidGameID )
    {
      return GetInventory( uiWeaponID );
    }

    return null;
  }


  function GetEquippedWeaponID()
  {
    return GetProperty( Equipped_Weapon_PropertyID, rumInvalidGameID );
  }


  function GetHitpoints()
  {
    return GetVersionedProperty( g_eHitpointsPropertyVersionArray, 1 );
  }


  function GetIntelligence()
  {
    return GetVersionedProperty( g_eIntelligencePropertyVersionArray, null );
  }


  function GetInventoryByType( i_eAssetType )
  {
    local ciInventory = GetInventory();
    local ciItem;
    while( ciItem = ciInventory.GetNextObject() )
    {
      if( ciItem.GetAssetID() == i_eAssetType )
      {
        ciInventory.Stop();
        return ciItem;
      }
    }

    return null;
  }


  function GetMana()
  {
    return GetVersionedProperty( g_eManaPropertyVersionArray );
  }


  function GetMaxHitpoints()
  {
    local iMaxHitpoints = 0;

    if( GameType.Ultima4 == g_ciCUO.m_eVersion )
    {
      local iLevel = GetProperty( U4_Level_PropertyID, 1 );
      iMaxHitpoints = iLevel * 100;
    }
    else if( GameType.Ultima3 == g_ciCUO.m_eVersion )
    {
      local iLevel = GetProperty( U3_Level_PropertyID, 1 );
      iMaxHitpoints = iLevel * 100 + 50;
    }
    else if( GameType.Ultima2 == g_ciCUO.m_eVersion )
    {
      // Unlike in the original, hitpoints cannot exceed experience cap for each level, or 400 at a minimum
      local iExp = GetProperty( U2_Experience_PropertyID, 0 );
      local iLvl = iExp / 100 + 1;
      iMaxHitpoints = max( iLvl * 100, 400 );
      iMaxHitpoints = min( iMaxHitpoints, 9999 );
    }
    else if( GameType.Ultima1 == g_ciCUO.m_eVersion )
    {
      // Unlike in the original, hitpoints cannot exceed experience cap for each level, or 999 at a minimum
      local iExp = GetProperty( U1_Experience_PropertyID, 0 );
      local iLvl = iExp / 1000 + 1;
      iMaxHitpoints = max( iLvl * 1000, 1000 );
      iMaxHitpoints = min( iMaxHitpoints, 9999 );
    }

    return iMaxHitpoints;
  }


  function GetNewTarget( i_iIndex, i_funcComparator = null )
  {
    local ciMap = GetMap();
    if( null == ciMap )
    {
      return;
    }

    local bTargetFound = false;
    local ciPlayerPos = GetPosition();
    local bMapRequiresLight = MapRequiresLight( ciMap );

    // Target defaults to player's position
    g_ciCUO.m_uiLockedTargetID = rumInvalidGameID;
    g_ciCUO.m_ciTargetPos = ciPlayerPos;

    if( i_iIndex >= 0 && i_iIndex < g_ciCUO.m_uiVisibleTargetIDsTable.len() )
    {
      local iIndex = 0;

      // Get the target at the matching index
      foreach( uiTargetID in g_ciCUO.m_uiVisibleTargetIDsTable )
      {
        if( iIndex >= i_iIndex )
        {
          local ciTarget = ::rumFetchPawn( uiTargetID );
          if( ciTarget != null )
          {
            if( ( null == i_funcComparator ) || i_funcComparator( this, ciTarget ) )
            {
              local ciTargetPos = ciTarget.GetPosition();

              local ciScreenPos = g_ciUI.s_ciCenterTilePos;
              ciScreenPos.x += ciTargetPos.x - ciPlayerPos.x;
              ciScreenPos.y += ciTargetPos.y - ciPlayerPos.y;

              if( !bMapRequiresLight || ciMap.IsPositionLit( ciScreenPos ) )
              {
                if( ciMap.IsPositionWithinRadius( ciPlayerPos, ciTargetPos, g_ciCUO.m_uiTargetRange ) )
                {
                  if( ciMap.TestLOS( ciPlayerPos, ciTargetPos, g_ciCUO.s_iLOSRadius ) )
                  {
                    g_ciCUO.m_uiLockedTargetID = uiTargetID;
                    g_ciCUO.m_ciTargetPos = ciTarget.GetPosition();
                    g_ciCUO.m_iLockedTargetIndex = iIndex;
                    bTargetFound = true;
                    break;
                  }
                }
              }
            }
          }
        }

        ++iIndex;
      }
    }

    if( !bTargetFound )
    {
      g_ciCUO.m_iLockedTargetIndex = -1;
    }
  }


  function GetPlayerClass()
  {
    local ePlayerClassID = GetVersionedProperty( g_ePlayerClassPropertyVersionArray, rumInvalidAssetID );
    return( ePlayerClassID != rumInvalidAssetID ? ::rumGetCustomAsset( ePlayerClassID ) : null );
  }


  function GetScreenShakeIntensity()
  {
    local fIntensity = 0.0;

    foreach( ciClientEffect in m_ciEffectsTable )
    {
      if( ciClientEffect instanceof ScreenShakeClientEffect )
      {
        fIntensity += ciClientEffect.m_fIntensity;
      }
    }

    return fIntensity;
  }


  function GetStamina()
  {
    return GetVersionedProperty( g_eStaminaPropertyVersionArray, null );
  }


  function GetStrength()
  {
    return GetVersionedProperty( g_eStrengthPropertyVersionArray, null );
  }


  function GetTransport()
  {
    local uiTransportID = GetTransportID();
    return uiTransportID != rumInvalidGameID ? ::rumFetchPawn( uiTransportID ) : null;
  }


  function GetTransportID()
  {
    return GetVersionedProperty( g_eTransportIDPropertyVersionArray, rumInvalidGameID );
  }


  function GetVersionedProperty( i_ePropertyArray, i_vDefault = 0 )
  {
    local ePropertyID = i_ePropertyArray[g_ciCUO.m_eVersion];
    return ePropertyID != null ? GetProperty( ePropertyID, i_vDefault ) : i_vDefault;
  }


  function GetWisdom()
  {
    return GetVersionedProperty( g_eWisdomPropertyVersionArray, null );
  }


  function GhostFade( i_iDeathIndex )
  {
    if( ( i_iDeathIndex == m_iDeathIndex ) && IsDead() )
    {
      local iTransparency = GetTransparencyLevel();
      if( iTransparency > 0 )
      {
        // Fade out the ghost a little more
        SetTransparencyLevel( max( 0, iTransparency - 2 ) );

        // Schedule next fade
        ::rumSchedule( this, GhostFade, s_fDeathFadeInterval, i_iDeathIndex );
      }
    }
  }


  function IsAdmin()
  {
    return GetProperty( Admin_PropertyID, false );
  }


  function IsCriminal()
  {
    return GetVersionedProperty( g_eCriminalPropertyVersionArray, false );
  }


  function IsDaemonImmune()
  {
    return GetProperty( U4_Shrine_Daemon_Immunity_PropertyID, false );
  }


  function IsDead()
  {
    return GetVersionedProperty( g_eHitpointsPropertyVersionArray, 1 ) <= 0;
  }


  function IsFlying()
  {
    local ciTransport = GetTransport();
    if( ciTransport != null )
    {
      local eMoveType = ciTransport.GetMoveType();
      if( ( MoveType.Drifts == eMoveType ) || ( MoveType.Flies == eMoveType ) )
      {
        return ciTransport.GetProperty( State_PropertyID, FlyingStateType.Landed ) == FlyingStateType.Flying;
      }
    }

    return false;
  }


  function IsIncapacitated()
  {
    return IsMeditating() || base.IsIncapacitated();
  }


  function IsMeditating()
  {
    return GetProperty( U4_Meditating_PropertyID, false );
  }


  function IsPoisoned()
  {
    return GetVersionedProperty( g_ePoisonedPropertyVersionArray, false );
  }


  function IsStarving()
  {
    return GetVersionedProperty( g_eFoodPropertyVersionArray ) == 0;
  }


  function LaunchTimeMachine( i_strArray )
  {
    if( i_strArray.len() == 0 )
    {
      return;
    }

    local strDesc = format( "<b>%s", i_strArray[0] );
    ShowString( strDesc, g_strColorTagArray.Yellow );

    local ciMap = GetMap();
    local eMapAssetID = ciMap.GetAssetID();
    local eLaunchType = ( U1_Time_Machine_MapID == eMapAssetID ) ? TransportCommandType.TimeMachineLaunchMondain
                                                                 : TransportCommandType.TimeMachineLaunchEnd;

    // Consume elements until there aren't any more
    i_strArray.remove( 0 );

    if( i_strArray.len() > 0 )
    {
      if( ( TransportCommandType.TimeMachineLaunchMondain == eLaunchType ) &&
          ( i_strArray.len() == 2 || i_strArray.len() == 3 ) )
      {
        DoScreenShakeEffect();
      }

      ::rumSchedule( this, LaunchTimeMachineNext, 1.0, i_strArray );
    }
    else
    {
      // Let the server know we're ready to launch
      local ciBroadcast = ::rumCreate( Player_Transport_BroadcastID, eLaunchType );
      ::rumSendBroadcast( ciBroadcast );

      g_ciUI.m_bInputBlocked = false;
      g_ciUI.m_fBlockTimer = 0.0;

      g_ciUI.m_ciGameInputTextBox.ResetInputMode( false );
    }
  }


  function LaunchTimeMachineNext( i_strArray )
  {
    g_ciUI.m_ciGameInputTextBox.SetInputMode( InputMode.PressAnyKey, LaunchTimeMachine.bindenv( this ), i_strArray );
  }


  function Move( i_eDir )
  {
    if( g_ciUI.m_bShowMoves )
    {
      ShowString( "<G#Prompt:vcenter>" + GetDirectionString( i_eDir ) );
    }

    local bSendMove = false;

    // If a static object is in the way, we can block the player without sending a move update
    local ciMap = GetMap();
    local ciPos = GetPosition() + GetDirectionVector( i_eDir );

    local eResult = rumErrorMoveResultType;

    // If player is on a transport, move the actual transport
    local ciTransport = GetTransport();
    if( ciTransport != null )
    {
      local uiCommanderID = ciTransport.GetCommanderID();
      if( GetID() == uiCommanderID )
      {
        eResult = ciMap.MovePawn( ciTransport, ciPos, rumTestMoveFlag );
      }
      else
      {
        eResult = -1;
      }
    }
    else
    {
      eResult = ciMap.MovePawn( this, ciPos, rumTestMoveFlag );
    }

    local eDelay = ActionDelay.Short;

    if( rumSuccessMoveResultType == eResult )
    {
      bSendMove = true;

      if( ciTransport && ( ciTransport.GetMoveType() == MoveType.Sails ) )
      {
        // Determine movement delay based on wind direction
        local iMinDistance = 0;

        if( i_eDir != g_ciCUO.m_eWindDirection )
        {
          local iClockwiseTurns = 0;
          local iCounterClockwiseTurns = 0;
          if( i_eDir > g_ciCUO.m_eWindDirection )
          {
            iClockwiseTurns = ( g_ciCUO.m_eWindDirection + 8 ) - i_eDir;
            iCounterClockwiseTurns = i_eDir - g_ciCUO.m_eWindDirection;
          }
          else
          {
            iClockwiseTurns = g_ciCUO.m_eWindDirection - i_eDir;
            iCounterClockwiseTurns = i_eDir - ( g_ciCUO.m_eWindDirection - 8 );
          }

          iMinDistance = min( iClockwiseTurns, iCounterClockwiseTurns );
        }

        if( iMinDistance > 2 )
        {
          // Sailing against the wind
          eDelay = ActionDelay.Long;
        }
        else if( iMinDistance > 0 )
        {
          // Sailing somewhat with or perpendicular to the wind
          eDelay = ActionDelay.Medium;
        }
      }
      else
      {
        // Determine movement delay based on the tile traversed
        local ciPosData = ciMap.GetPositionData( ciPos );
        local ciTile = ::rumGetTileAsset( ciPosData.GetTileID() );
        local fWeight = ciTile.GetWeight();
        if( fWeight >= 2.0 )
        {
          eDelay = ActionDelay.Long;
        }
        else if( fWeight >= 1.0 )
        {
          eDelay = ActionDelay.Medium;
        }
        else
        {
          eDelay = ActionDelay.Short;
        }
      }
    }
    else if( rumOffMapMoveResultType == eResult )
    {
      bSendMove = true;
      eDelay = ActionDelay.Long;
    }
    else
    {
      bSendMove = false;

      switch( eResult )
      {
        case rumTileCollisionMoveResultType:
          ShowString( ::rumGetString( msg_blocked_client_StringID ), g_strColorTagArray.Red );
          PlaySound( Blocked_SoundID );
          break;

        case rumPawnCollisionMoveResultType:
          // See if the collision was with an NPC
          local ciPosData = ciMap.GetPositionData( ciPos );
          local ciCreature = null;
          while( ciCreature = ciPosData.GetNext( rumCreaturePawnType ) )
          {
            if( ciCreature instanceof NPC )
            {
              // There's a chance we could nudge the NPC out of the way
              bSendMove = true;
              break;
            }
          }

          if( !bSendMove )
          {
            ShowString( ::rumGetString( msg_blocked_client_StringID ), g_strColorTagArray.Red );
            PlaySound( Blocked_SoundID );
          }
          break;

        case rumErrorMoveResultType:
          ShowString( ::rumGetString( msg_failed_client_StringID ), g_strColorTagArray.Red );
          break;

        case -1:
          ShowString( ::rumGetString( msg_not_commander_client_StringID ), g_strColorTagArray.Red );
          break;
      }
    }

    if( bSendMove )
    {
      local ciBroadcast = ::rumCreate( Player_Move_BroadcastID, i_eDir );
      ::rumSendBroadcast( ciBroadcast );
    }

    BlockInput( GetActionDelay( eDelay ) );
  }


  function OnCombatChange( i_bCombatStarting )
  {
    if( this == ::rumGetMainPlayer() )
    {
      if( i_bCombatStarting )
      {
        // Combat starting
        PlayCombatMusic();
      }
      else
      {
        // Combat ending
        StopCombatMusic();
      }
    }
  }


  function OnDeath()
  {
    UpdateGraphic();

    // Start a slow-fadeout of the ghost
    ++m_iDeathIndex;
    ::rumSchedule( this, GhostFade, s_fDeathFadeInterval, m_iDeathIndex );

    if( ::rumGetMainPlayer() == this )
    {
      // Stop targeting if it is active
      if( InputMode.Target == g_ciUI.m_ciGameInputTextBox.m_eInputMode )
      {
        g_ciUI.m_ciGameInputTextBox.Clear();
        g_ciUI.m_ciGameInputTextBox.ResetInputMode( false );
      }

      SetMoveType( MoveType.Incorporeal );

      // Ghosts can't look at the world map
      g_ciUI.m_eOverlayGraphicID = rumInvalidAssetID;

      // Ghosts don't have equipment, spells, runes, etc.
      switch( g_ciCUO.m_eVersion )
      {
        case GameType.Ultima1: g_ciUI.m_eCurrentStatPage = U1_StatPage.Main; break;
        case GameType.Ultima2: g_ciUI.m_eCurrentStatPage = U2_StatPage.Main; break;
        case GameType.Ultima3: g_ciUI.m_eCurrentStatPage = U3_StatPage.Main; break;
        case GameType.Ultima4: g_ciUI.m_eCurrentStatPage = U4_StatPage.Main; break;
      }

      Ultima_Stat_Update();

      ShowString( ::rumGetString( msg_resurrect_void_client_StringID ), g_strColorTagArray.Yellow );

      local bBound = GetVersionedProperty( g_eSpiritBoundPropertyVersionArray );
      if( bBound )
      {
        ShowString( ::rumGetString( msg_resurrect_body_client_StringID ), g_strColorTagArray.Yellow );
      }
    }
  }


  function OnInventoryAdded( i_ciItem )
  {
    UpdateInventory( i_ciItem );
  }


  function OnInventoryRemoved( i_ciItem )
  {
    UpdateInventory( i_ciItem );
  }


  function OnLightRangeUpdated( i_iRange )
  {
    local ciPlayer = ::rumGetMainPlayer();
    if( ( this == ciPlayer ) && ciPlayer.IsDead() )
    {
      // Client-side lighting only
      SetLightRange( i_iRange );
    }
  }


  function OnPositionUpdated( i_ciNewPos, i_ciOldPos )
  {
    local ciPlayer = ::rumGetMainPlayer();
    if( this == ciPlayer )
    {
      ::rumSetListenerPosition( i_ciNewPos );

      local eTransportType = TransportType.None;
      local ciTransport = GetTransport();
      if( ciTransport != null )
      {
        eTransportType = ciTransport.GetType();
      }

      if( ( TransportType.None == eTransportType ) || ( TransportType.Horse == eTransportType ) )
      {
        PlaySound( Step_SoundID );
      }

      // The player itself moved, so rebuild all nearby pawn lists
      local ciMap = ciPlayer.GetMap();
      if( ciMap != null )
      {
        g_ciCUO.m_uiVisibleTargetIDsTable.clear();

        // Determine which pawns are viable targets
        local ciPawnArray = ciMap.GetPawns( i_ciNewPos, g_ciCUO.s_iLOSRadius, false );
        foreach( ciPawn in ciPawnArray )
        {
          if( ciPawn.IsVisible() &&
              ( ( ciPawn instanceof Creature ) && ciPawn != ciPlayer ) ||
              ( ( ciPawn instanceof Widget ) && ciPawn.GetProperty( Widget_Destructible_PropertyID, false ) ) )
          {
            local ciPawnPosition = ciPawn.GetPosition();
            if( ciMap.IsPositionWithinTileDistance( ciPawnPosition, i_ciNewPos, g_ciCUO.s_iLOSRadius ) &&
                ciMap.TestLOS( ciPawnPosition, i_ciNewPos, g_ciCUO.s_iLOSRadius ) )
            {
              // Found a viable target
              local uiPawnID = ciPawn.GetID();
              g_ciCUO.m_uiVisibleTargetIDsTable[uiPawnID] <- uiPawnID;
            }
          }
        }

        // Special map labels for U1 world map
        local eMapID = ciMap.GetAssetID();
        if( U1_Sosaria_MapID == eMapID )
        {
          local ciPlayer = ::rumGetMainPlayer();

          local iHorizMid = ciMap.GetNumColumns() / 2;
          local iVertMid = ciMap.GetNumRows() / 2;

          local strCurrentLand = g_ciUI.m_ciMapNameLabel.GetText();
          local strLand = "";

          // Determine map label based on quadrant
          if( i_ciNewPos.x < iVertMid )
          {
            if( i_ciNewPos.y < iHorizMid )
            {
              strLand = ::rumGetString( U1_Sosaria_Lands_Lord_British_Map_client_StringID );
            }
            else
            {
              strLand = ::rumGetString( U1_Sosaria_Lands_Dark_Unknown_Map_client_StringID );
            }
          }
          else
          {
            if( i_ciNewPos.y < iHorizMid )
            {
              strLand = ::rumGetString( U1_Sosaria_Lands_Feudal_Lords_Map_client_StringID );
            }
            else
            {
              strLand = ::rumGetString( U1_Sosaria_Lands_Danger_Despair_Map_client_StringID );
            }
          }

          if( strLand != strCurrentLand )
          {
            g_ciUI.m_ciMapNameLabel.SetText( strLand );
          }
        }
      }

      Ultima_UpdateMapLabel();
    }

    base.OnPositionUpdated( i_ciNewPos, i_ciOldPos );
  }


  function OnPropertyRemoved( i_ePropertyID )
  {
    base.OnPropertyRemoved( i_ePropertyID );

    local ciPlayer = ::rumGetMainPlayer();
    local bUpdatePartyStats = false;

    if( Combat_PropertyID == i_ePropertyID )
    {
      // Combat ended, so stop the music
      StopCombatMusic();
    }
    else if( ( Unconscious_PropertyID == i_ePropertyID ) ||
             ( ( U4_Meditating_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) )
    {
      UpdateGraphic();
      bUpdatePartyStats = true;
    }
    else if( ( ( U4_Transport_ID_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( ( U3_Transport_ID_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima3 ) ) ||
             ( ( U2_Transport_ID_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima2 ) ) ||
             ( ( U1_Transport_ID_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima1 ) ) )
    {
      UpdateGraphic();

      if( ciPlayer == this )
      {
        g_ciCUO.m_bAloft = ciPlayer.IsFlying();

        local ciMap = ciPlayer.GetMap();
        if( ciMap != null )
        {
          ciMap.SetPeeringAttributes();
        }
      }
    }
    else if( Character_Prep_PropertyID == i_ePropertyID )
    {
      g_ciCUO.m_bReceivingData = false;
    }
    else if( ( (      Burning_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( (       Frozen_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( (       Jinxed_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( (      Negated_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( (    Protected_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( (    Quickened_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( (  U4_Poisoned_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( (  U3_Poisoned_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima3 ) ) ||
             ( (  U2_Poisoned_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima2 ) ) ||
             ( (  Unconscious_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) )
    {
      UpdateEffects();
      bUpdatePartyStats = true;
    }

    if( !g_ciCUO.m_bReceivingData && ( GameMode.Game == g_ciCUO.m_eCurrentGameMode ) )
    {
      if( ( g_ciUI.m_eCurrentStatPage == U4_StatPage.Party ) ||
          ( g_ciUI.m_eCurrentStatPage == U3_StatPage.Party ) ||
          ( g_ciUI.m_eCurrentStatPage == U2_StatPage.Party ) ||
          ( g_ciUI.m_eCurrentStatPage == U1_StatPage.Party ) )
      {
        if( bUpdatePartyStats )
        {
          Ultima_Stat_Update();
        }
      }
      else
      {
        // TODO - this is kind of lame, something like the food stat being changed will cause the entire party
        // window to update.
        Ultima_Stat_Update();
      }
    }
  }


  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    base.OnPropertyUpdated( i_ePropertyID, i_vValue );

    local ciPlayer = ::rumGetMainPlayer();
    local bUpdatePartyStats = false;

    // This property may be reflected in the current stat window, so go ahead and update it
    if( Character_Prep_PropertyID == i_ePropertyID )
    {
      g_ciCUO.m_bReceivingData = i_vValue;
    }
    else if( Combat_PropertyID == i_ePropertyID )
    {
      OnCombatChange( i_vValue );
    }
    else if( ( ( U4_Hitpoints_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( ( U3_Hitpoints_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima3 ) ) ||
             ( ( U2_Hitpoints_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima2 ) ) ||
             ( ( U1_Hitpoints_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima1 ) ) )
    {
      if( i_vValue <= 0 )
      {
        OnDeath();
      }

      bUpdatePartyStats = true;
    }
    else if( ( ( U4_Food_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( ( U3_Food_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima3 ) ) ||
             ( ( U2_Food_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima2 ) ) ||
             ( ( U1_Food_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima1 ) ) )
    {
      if( ciPlayer == this )
      {
        UpdateFood( i_vValue );
      }

      bUpdatePartyStats = true;
    }
    else if( ( ( U4_Gold_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( ( U3_Gold_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima3 ) ) ||
             ( ( U2_Gold_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima2 ) ) ||
             ( ( U1_Coin_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima1 ) ) )
    {
      if( ciPlayer == this )
      {
        UpdateGold( i_vValue );
      }
    }
    else if( Equipped_Weapon_PropertyID == i_ePropertyID )
    {
      local ciWeapon = ciPlayer.GetInventory( i_vValue );
      UpdateInventory( ciWeapon );
    }
    else if( Equipped_Armour_PropertyID == i_ePropertyID )
    {
      local ciArmour = ciPlayer.GetInventory( i_vValue );
      UpdateInventory( ciArmour );
    }
    else if( Ultima_Version_PropertyID == i_ePropertyID )
    {
      OnVersionChange( i_vValue );
    }
    else if( Admin_PropertyID == i_ePropertyID )
    {
      if( ciPlayer == this )
      {
        ShowString( ::rumGetString( msg_admin_granted_client_StringID ), g_strColorTagArray.Yellow );
      }

      UpdateNameLabel();
    }
    else if( ( ( U4_Criminal_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( ( U3_Criminal_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima3 ) ) ||
             ( ( U2_Criminal_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima2 ) ) ||
             ( ( U1_Criminal_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima1 ) ) )
    {
      UpdateNameLabel();
    }
    else if( ( Unconscious_PropertyID == i_ePropertyID )                                                     ||
             ( ( U4_Meditating_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( ( U4_Graphic_ID_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( ( U3_Graphic_ID_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima3 ) ) ||
             ( ( U2_Graphic_ID_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima2 ) ) ||
             ( ( U1_Graphic_ID_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima1 ) ) )
    {
      UpdateGraphic();
      bUpdatePartyStats = true;
    }
    else if( ( ( U4_Transport_ID_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( ( U3_Transport_ID_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima3 ) ) ||
             ( ( U2_Transport_ID_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima2 ) ) ||
             ( ( U1_Transport_ID_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima1 ) ) )
    {
      UpdateGraphic();

      if( ciPlayer == this )
      {
        g_ciCUO.m_bAloft = ciPlayer.IsFlying();

        local ciMap = ciPlayer.GetMap();
        if( ciMap != null )
        {
          ciMap.SetPeeringAttributes();
        }
      }
    }
    else if( U4_Wind_Direction_Override_PropertyID == i_ePropertyID )
    {
      Ultima_UpdateMapLabel();
    }
    else if( ( (      Burning_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( (       Frozen_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( (       Jinxed_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( (      Negated_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( (    Protected_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( (    Quickened_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( (  U4_Poisoned_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) ||
             ( (  U3_Poisoned_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima3 ) ) ||
             ( (  U2_Poisoned_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima2 ) ) ||
             ( (  Unconscious_PropertyID == i_ePropertyID ) && ( g_ciCUO.m_eVersion == GameType.Ultima4 ) ) )
    {
      UpdateEffects();
      bUpdatePartyStats = true;
    }

    if( !g_ciCUO.m_bReceivingData && ( GameMode.Game == g_ciCUO.m_eCurrentGameMode ) )
    {
      if( ( g_ciUI.m_eCurrentStatPage == U4_StatPage.Party ) ||
          ( g_ciUI.m_eCurrentStatPage == U3_StatPage.Party ) ||
          ( g_ciUI.m_eCurrentStatPage == U2_StatPage.Party ) ||
          ( g_ciUI.m_eCurrentStatPage == U1_StatPage.Party ) )
      {
        if( bUpdatePartyStats )
        {
          Ultima_Stat_Update();
        }
      }
      else
      {
        // TODO - this is kind of lame, something like the food stat being changed will cause the entire party
        // window to update.
        Ultima_Stat_Update();
      }
    }
  }


  function OnVersionChange( i_eVersion )
  {
    if( this != ::rumGetMainPlayer() )
    {
      return;
    }

    // Update the current version
    g_ciCUO.m_eVersion = i_eVersion;

    VerifyInstallation( i_eVersion );

    // Always force a gold update to fix the wording (coin/gold)
    UpdateGold( GetProperty( g_eGoldPropertyVersionArray[ i_eVersion ], 0 ) );

    // Reset the map label
    g_ciUI.m_eMapLabelType = MapLabelType.MapName;
    Ultima_UpdateMapLabel();

    // Set the player's name on the stat bar
    g_ciUI.m_ciStatLabel.SetText( GetPlayerName() );

    // Set the main stat page
    switch( i_eVersion )
    {
      case GameType.Ultima1: g_ciUI.m_eCurrentStatPage = U1_StatPage.Main; break;
      case GameType.Ultima2: g_ciUI.m_eCurrentStatPage = U2_StatPage.Main; break;
      case GameType.Ultima3: g_ciUI.m_eCurrentStatPage = U3_StatPage.Main; break;
      case GameType.Ultima4: g_ciUI.m_eCurrentStatPage = U4_StatPage.Main; break;
    }

    g_ciUI.m_ePreviousStatPage = g_ciUI.m_eCurrentStatPage;

    DoSpellEffect( this, true /* play sound */ );

    g_ciCUO.m_bAloft = IsFlying();
    g_ciCUO.m_bPeering = false;

    local ciMap = GetMap();
    if( ciMap != null )
    {
      ciMap.SetPeeringAttributes();
    }

    Ultima_Stat_Update();
    UpdateNameLabel();

    UpdateGraphic();
  }


  function PlayerInParty( i_ciPlayer )
  {
    if( i_ciPlayer != null && ( i_ciPlayer instanceof Player ) && g_ciCUO.m_uiPartyIDTable != null )
    {
      return ( i_ciPlayer.GetID() in g_ciCUO.m_uiPartyIDTable );
    }

    return false
  }


  function ShowNameLabel( i_iX, i_iY )
  {
    if( null == m_ciNameLabel )
    {
      UpdateNameLabel();
    }

    m_ciNameLabel.Display( rumPoint( i_iX, i_iY ) );
  }


  // Called when the main client player selects journey onward and all player data has been received
  function StartPlaying()
  {
    g_ciCharGen = null;

    g_ciUI.m_ciFoodLabel.SetActive( true );
    g_ciUI.m_ciGoldLabel.SetActive( true );
    g_ciUI.m_ciMapNameLabel.SetActive( true );
    g_ciUI.m_ciGameTextView.SetActive( true );
    g_ciUI.m_ciStatLabel.SetActive( true );
    g_ciUI.m_ciStatListView.SetActive( true );

    g_ciUI.m_ciChatTextView.PushText( ::rumGetString( welcome_message0_client_StringID ) );
    g_ciUI.m_ciChatTextView.PushText( ::rumGetString( welcome_message1_client_StringID ) );
    g_ciUI.m_ciChatTextView.PushText( ::rumGetString( welcome_message3_client_StringID ) );

    g_ciUI.m_ciGameRegion.SetActive( true );
    g_ciUI.m_ciGameRegion.m_funcMouseButtonPress = MousePressedGame;
    g_ciUI.m_ciGameRegion.m_funcMouseMoved = MouseMovedGame;

    g_ciUI.m_ciGameInputTextBox.SetActive( true );
    g_ciUI.m_ciGameInputTextBox.m_funcKeyPress = KeyPressedGame;
    g_ciUI.m_ciGameInputTextBox.m_funcKeyPress = KeyPressedGame;
    g_ciUI.m_ciGameInputTextBox.Focus();

    // Verification re-focus
    if( g_ciUI.m_bNeedsInstallVerification )
    {
      // Re-focus the verification controls
      g_ciUI.m_ciExplorerDriveListView.SetActive( true );
      g_ciUI.m_ciExplorerFileListView.SetActive( true );
    }
    else
    {
      g_ciUI.m_ciChatTextView.SetActive( true );
    }

    SetProperty( U4_Wind_Direction_Override_PropertyID, Direction.None );

    // Force position update
    local ciPos = GetPosition();
    OnPositionUpdated( ciPos, rumPos( 0, 0 ) );

    // Refresh stats here?
    Ultima_Stat_Update();

    // Transition to game mode
    g_ciCUO.m_eCurrentGameMode = GameMode.Game;
  }


  function UpdateFood( i_iAmount )
  {
    local strFood = format( "%s: %d", ::rumGetString( Food_Property_client_StringID ), i_iAmount );
    g_ciUI.m_ciFoodLabel.SetText( strFood );
  }


  function UpdateGold( i_iAmount )
  {
    local strGold;

    if( GameType.Ultima1 == g_ciCUO.m_eVersion )
    {
      strGold = format( "%s: %d", ::rumGetString( token_coin_client_StringID ), i_iAmount );
    }
    else
    {
      strGold = format( "%s: %d", ::rumGetString( Gold_Property_client_StringID ), i_iAmount );
    }

    g_ciUI.m_ciGoldLabel.SetText( strGold );
  }


  function UpdateGraphic()
  {
    // How transport graphics are handled:
    // When a player boards a transport, the transport is hidden and the local player graphic is switched to that of
    // the transport. All other players on the same transport are hidden. This is to keep players from temporarily
    // seeing multiple copies of transports or players because of latency as the transport is moved.

    local iAnimSet = 0;

    local ciTransport = GetTransport();
    if( ciTransport != null )
    {
      // The player graphic and animation set changes to that of the transport
      local eGraphicID = ciTransport.GetGraphic();
      if( ( ciTransport instanceof U4_Horse_Widget ) ||
          ( ciTransport instanceof U3_Horse_Widget ) ||
          ( ciTransport instanceof U2_Horse_Widget ) ||
          ( ciTransport instanceof U1_Horse_Widget ) )
      {
        eGraphicID = U4_Horserider_GraphicID;
      }

      SetGraphic( eGraphicID );

      iAnimSet = ciTransport.GetAnimationSet();
    }
    else if( IsDead() )
    {
      SetGraphic( U4_Ghost_GraphicID );
      SetTransparencyLevelFromFloat( 0.66 );
    }
    else if( GetProperty( Unconscious_PropertyID, false ) )
    {
      SetGraphic( U4_Body_GraphicID );
    }
    else if( GetProperty( U4_Meditating_PropertyID, false ) )
    {
      SetGraphic( U4_Beggar_GraphicID );
    }
    else
    {
      // The player graphic changes back to their default player graphic
      local eGraphicID = GetVersionedProperty( g_eGraphicIDPropertyVersionArray, U4_Adventurer_GraphicID );
      SetGraphic( eGraphicID );
      SetTransparencyLevel( rumAlphaOpaque );
    }

    UseAnimationSet( iAnimSet );
  }


  function UpdateInventory( i_ciItem )
  {
    if( null == i_ciItem )
    {
      return;
    }

    local eType = i_ciItem.GetProperty( Inventory_Type_PropertyID, InventoryType.Standard );
    if( InventoryType.Weapon == eType )
    {
      switch( g_ciCUO.m_eVersion )
      {
        case GameType.Ultima1: if( U1_StatPage.Weapons == g_ciUI.m_eCurrentStatPage ) U1_Stat_Weapons(); break;
        case GameType.Ultima2: if( U2_StatPage.Weapons == g_ciUI.m_eCurrentStatPage ) U2_Stat_Weapons(); break;
        case GameType.Ultima3: if( U3_StatPage.Weapons == g_ciUI.m_eCurrentStatPage ) U3_Stat_Weapons(); break;
        case GameType.Ultima4: if( U4_StatPage.Weapons == g_ciUI.m_eCurrentStatPage ) U4_Stat_Weapons(); break;
      }
    }
    else if( InventoryType.Armour == eType )
    {
      switch( g_ciCUO.m_eVersion )
      {
        case GameType.Ultima1: if( U1_StatPage.Armour == g_ciUI.m_eCurrentStatPage ) U1_Stat_Armour(); break;
        case GameType.Ultima2: if( U2_StatPage.Armour == g_ciUI.m_eCurrentStatPage ) U2_Stat_Armour(); break;
        case GameType.Ultima3: if( U3_StatPage.Armour == g_ciUI.m_eCurrentStatPage ) U3_Stat_Armour(); break;
        case GameType.Ultima4: if( U4_StatPage.Armour == g_ciUI.m_eCurrentStatPage ) U4_Stat_Armour(); break;
      }
    }
  }


  function UpdateNameLabel()
  {
    if( null == m_ciNameLabel )
    {
      // Width plus padding
      local iWidth = ::rumGetTextWidth( "small", GetPlayerName() ) + ( g_ciUI.s_iTextPixelPadding * 2 );

      m_ciNameLabel = ::rumCreateControl( Label );
      m_ciNameLabel.SetBackgroundColor( g_ciUI.s_ciColorBlackAlpha );
      m_ciNameLabel.SetWidth( iWidth );
      m_ciNameLabel.SetHeight( g_ciUI.s_iTextPixelPadding * 3 );
      m_ciNameLabel.AlignCenter();
    }

    local strName = "<F#small>";

    if( IsCriminal() )
    {
      strName += g_strColorTagArray.Red;
    }
    else if( IsAdmin() )
    {
      strName += g_strColorTagArray.Yellow;
    }

    strName += GetPlayerName();
    m_ciNameLabel.SetText( strName );
  }
}
