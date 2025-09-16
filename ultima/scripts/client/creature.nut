class Creature extends rumCreature
{
  m_ciEffectsTable = null;


  constructor()
  {
    base.constructor();

    m_ciEffectsTable = {};
  }


  function AddClientEffect( i_ciClientEffect )
  {
    // A weakref is not used here because this is the only thing that keeps the object from being garbage collected
    m_ciEffectsTable[i_ciClientEffect] <- i_ciClientEffect;
  }


  function GetActionDelay( i_eDelayType )
  {
    // Based on Dexterity values of 0 to 100, durations can be from (seconds):

    // Dex:      0      99
    // -----------------------
    // Short  =  0.2 to 0.1
    // Medium =  0.3 to 0.2
    // Long   =  0.4 to 0.3

    return i_eDelayType > ActionDelay.None ? ( 100.0 - GetDexterity() ) / 1000.0 + ( i_eDelayType / 10.0 ) : 0.0;
  }


  function GetAlignment()
  {
    return GetProperty( Alignment_PropertyID, AlignmentType.Good );
  }


  function GetCharisma()
  {
    return GetProperty( Creature_Charisma_PropertyID, 15 );
  }


  function GetDescription()
  {
    return ::rumGetStringByName( GetName() + "_Creature_client_StringID" );
  }


  function GetDexterity()
  {
    return GetProperty( Creature_Dexterity_PropertyID, 15 );
  }


  function GetHitpoints()
  {
    return GetProperty( Hitpoints_PropertyID, 1 );
  }


  function GetIntelligence()
  {
    return GetProperty( Creature_Intelligence_PropertyID, 15 );
  }


  function GetMana()
  {
    return GetProperty( Mana_PropertyID, 0 );
  }


  function GetMaxHitpoints()
  {
    return GetProperty( Max_Hitpoints_PropertyID, 99 );
  }


  function GetMaxMana()
  {
    return GetProperty( Max_Mana_PropertyID, 0 );
  }


  function GetStamina()
  {
    return GetProperty( Creature_Stamina_PropertyID, 15 );
  }


  function GetStrength()
  {
    return GetProperty( Creature_Strength_PropertyID, 15 );
  }


  function GetWisdom()
  {
    return GetProperty( Creature_Wisdom_PropertyID, 15 );
  }


  function IsBurning()
  {
    return GetProperty( Burning_PropertyID, false );
  }


  function IsDead()
  {
    return GetHitpoints() < 1;
  }


  function IsCamouflaged()
  {
    return GetProperty( Camouflaged_PropertyID, false );
  }


  function IsCriminal()
  {
    return false;
  }


  function IsFlying()
  {
    return false;
  }


  function IsFrozen()
  {
    return GetProperty( Frozen_PropertyID, false );
  }


  function IsIncapacitated()
  {
    return IsUnconscious() || IsFrozen();
  }


  function IsJinxed()
  {
    return GetProperty( Jinxed_PropertyID, false );
  }


  function IsNegated()
  {
    return GetProperty( Negated_PropertyID, false );
  }


  function IsPoisoned()
  {
    return GetProperty( Poisoned_PropertyID, false );
  }


  function IsProtected()
  {
    return GetProperty( Protected_PropertyID, false );
  }


  function IsQuickened()
  {
    return GetProperty( Quickened_PropertyID, false );
  }


  function IsUnconscious()
  {
    return GetProperty( Unconscious_PropertyID, false );
  }


  function OnAnimate( i_uiCurrentFrame )
  {
    return ( rand()%2 == 0 ? i_uiCurrentFrame + 1 : i_uiCurrentFrame );
  }


  function OnInventoryAdded( i_ciItem )
  {}


  function OnInventoryRemoved( i_ciItem )
  {}


  function OnObjectReleased()
  {
    // Release all affects since they potentially hold a reference to the creature
    m_ciEffectsTable.clear();
  }


  function OnPositionUpdated( i_ciNewPos, i_ciOldPos )
  {
    local ciPlayer = ::rumGetMainPlayer();
    if( this != ciPlayer )
    {
      local ciMap = ciPlayer.GetMap();
      if( ciMap != null )
      {
        local uiCreatureID = GetID();

        local ciPlayerPos = ciPlayer.GetPosition();
        local bVisible = IsVisible() &&
                         ciMap.IsPositionWithinTileDistance( i_ciNewPos, ciPlayerPos, g_ciCUO.s_iLOSRadius ) &&
                         ciMap.TestLOS( i_ciNewPos, ciPlayerPos, g_ciCUO.s_iLOSRadius );
        if( bVisible )
        {
          // Add the creature to the visible list (it might already be there)
          g_ciCUO.m_uiVisibleTargetIDsTable[uiCreatureID] <- uiCreatureID;
        }
        else if( uiCreatureID in g_ciCUO.m_uiVisibleTargetIDsTable )
        {
          // Creature is out of range - make sure it's not in the player's nearby pawn list
          delete g_ciCUO.m_uiVisibleTargetIDsTable[uiCreatureID];
          if( g_ciCUO.m_uiLockedTargetID == uiCreatureID )
          {
            g_ciCUO.m_uiLockedTargetID = rumInvalidGameID;
            g_ciCUO.m_ciTargetPos = ciPlayer.GetPosition();
          }
        }

        if( GetMoveType() == MoveType.Terrestrial )
        {
          PlaySound3D( Step_SoundID, i_ciNewPos );
        }

        // If the targeting reticle is locked on something, adjust its position
        if( g_ciCUO.m_uiLockedTargetID == uiCreatureID )
        {
          if( bVisible )
          {
            g_ciCUO.m_ciTargetPos = i_ciNewPos;
          }
          else
          {
            ciPlayer.GetNewTarget( 0 );
          }
        }
      }
    }

    // Update any client effects that would be affected by the creature's move
    local ciMap = GetMap();
    if( ciMap != null )
    {
      foreach( idx, ciClientEffect in m_ciEffectsTable )
      {
        if( ciClientEffect && ( ClientEffectType.Damage == ciClientEffect.m_eType ) )
        {
          ciMap.MovePawn( ciClientEffect.m_ciObject, i_ciNewPos,
                          rumIgnoreTileCollisionMoveFlag | rumIgnorePawnCollisionMoveFlag |
                          rumIgnoreDistanceMoveFlag );
        }
      }
    }
  }


  function OnPropertyRemoved( i_ePropertyID )
  {
    if( ( Frozen_PropertyID == i_ePropertyID ) || ( Poisoned_PropertyID == i_ePropertyID ) )
    {
      UpdateEffects();
    }

    if( ( Frozen_PropertyID == i_ePropertyID ) || ( Unconscious_PropertyID == i_ePropertyID ) )
    {
      StartAnimating();
    }
  }


  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    if( ( Frozen_PropertyID == i_ePropertyID ) || ( Poisoned_PropertyID == i_ePropertyID ) )
    {
      UpdateEffects();
    }

    if( ( Frozen_PropertyID == i_ePropertyID ) || ( Unconscious_PropertyID == i_ePropertyID ) )
    {
      // Animate based on change
      i_vValue ? StopAnimating() : StartAnimating();
    }
  }


  function OnVisibilityUpdated( i_bVisible )
  {
    local uiCreatureID = GetID();

    local ciPlayer = ::rumGetMainPlayer();
    if( ciPlayer != null && ciPlayer.GetID() != uiCreatureID )
    {
      local ciMap = ciPlayer.GetMap();
      if( ciMap!= null )
      {
        if( i_bVisible )
        {
          // LOS is checked during targeting
          if( ciMap.IsPositionWithinTileDistance( GetPosition(), ciPlayer.GetPosition(), g_ciCUO.s_iLOSRadius ) )
          {
            // Add the creature to the visible list (it might already be there)
            g_ciCUO.m_uiVisibleTargetIDsTable[uiCreatureID] <- uiCreatureID;
          }
        }
        else if( uiCreatureID in g_ciCUO.m_uiVisibleTargetIDsTable )
        {
          // The creature can no longer be seen
          delete g_ciCUO.m_uiVisibleTargetIDsTable[uiCreatureID];
          if( g_ciCUO.m_uiLockedTargetID == uiCreatureID )
          {
            ciPlayer.GetNewTarget( 0 );
          }
        }
      }
    }
  }


  function RemoveClientEffect( i_ciClientEffect )
  {
    delete m_ciEffectsTable[i_ciClientEffect];
  }


  function UpdateEffects()
  {
    SetBlendType( rumBlendType_None );

    // Frozen effects take precedence over poison
    if( IsFrozen() )
    {
      // Make the creature look like they're covered in ice
      SetBlendType( rumBlendType_Translucent );
      SetBufferColor( ::rumColorCyan );
      SetRestoreAlphaPostBlend( true );
      SetBlendColor( g_ciUI.s_ciColorIce );
    }
    else if( IsPoisoned() )
    {
      // Make the creature look sickly
      SetBlendType( rumBlendType_Translucent );
      SetBufferColor( ::rumColorGreen );
      SetRestoreAlphaPostBlend( true );
      SetBlendColor( g_ciUI.s_ciColorPoison );
    }
  }
}
