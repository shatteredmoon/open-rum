class U1_Chest_Widget extends U1_Widget
{
  function Open( i_ciPlayer, i_bOpenSpell )
  {
    if( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() )
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      return;
    }

    SetVisibility( false );

    // See if chest has a trap
    if( !i_bOpenSpell )
    {
      TrapTrigger( i_ciPlayer );
    }

    // Give contents to player
    local iCoin = rand() % 20 + 1;

    // Send an update on the amount of coin found
    local ciBroadcast = ::rumCreate( Player_Get_BroadcastID, iCoin );
    i_ciPlayer.SendBroadcast( ciBroadcast );

    i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, iCoin );

    // Chance of finding weapons, spells, or equipment
    local iRoll = rand() % 100;
    if( iRoll < 2 )
    {
      // Gain a random spell
      local ePropertyID = rand() % g_eU1SpellPropertyArray.len();
      i_ciPlayer.SetProperty( ePropertyID, i_ciPlayer.GetProperty( ePropertyID, 0 ) + 1 );
    }
    else if( iRoll < 10 )
    {
      // Gain a potion
      i_ciPlayer.SetProperty( U1_Potions_PropertyID, i_ciPlayer.GetProperty( U1_Potions_PropertyID, 0 ) + 1 );
    }
    else if( iRoll < 20 )
    {
      // Gain rope and spikes
      i_ciPlayer.SetProperty( U1_Rope_Spikes_PropertyID, i_ciPlayer.GetProperty( U1_Rope_Spikes_PropertyID, 0 ) + 1 );
    }
    else if( iRoll < 30 )
    {
      // Gain torch
      i_ciPlayer.SetProperty( U1_Torches_PropertyID, i_ciPlayer.GetProperty( U1_Torches_PropertyID, 0 ) + 1 );
    }

    // Set respawn timer if necessary
    local eChestType = GetProperty( Widget_Chest_Type_PropertyID, ChestType.Treasure );
    if( ChestType.Treasure == eChestType )
    {
      ScheduleRespawn();
    }
  }


  function TrapTrigger( i_ciPlayer )
  {
    // There is a 50% chance that the chest will be trapped
    local iTrapChance = rand() % 100;
    if( iTrapChance < 50 )
    {
      // Chest is definitely trapped, pre-check to see if player successfully avoids trap
      local bAvoided = i_ciPlayer.SkillRoll( i_ciPlayer.GetDexterity() );

      if( iTrapChance > 25 )
      {
        i_ciPlayer.ActionWarning( msg_acid_trap_client_StringID );

        if( !bAvoided )
        {
          local iDamage = 16 + rand() % 32;
          i_ciPlayer.Damage( iDamage, this );
        }
      }

      // Trap successfully evaded
      if( bAvoided )
      {
        i_ciPlayer.ActionSuccess( msg_evaded_client_StringID );
      }
    }
  }
}


class U2_Chest_Widget extends U2_Widget
{
  function Open( i_ciPlayer, i_bOpenSpell )
  {
    SetVisibility( false );

    // Give contents to player
    local iFound = rand() % 99 + 1;

    // Send an update on the amount of gold found
    local ciBroadcast = ::rumCreate( Player_Get_BroadcastID, iFound );
    i_ciPlayer.SendBroadcast( ciBroadcast );

    i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, iFound );

    local eChestType = GetProperty( Widget_Chest_Type_PropertyID, ChestType.Treasure );
    if( ChestType.Guarded == eChestType )
    {
      // See if the player was caught stealing
      if( !i_ciPlayer.SkillRoll( i_ciPlayer.GetDexterity() ) )
      {
        i_ciPlayer.AdjustVersionedProperty( g_eNotorietyPropertyVersionArray, 1 );
      }
    }

    // Set respawn timer if necessary
    if( ( ChestType.Treasure == eChestType ) || ( ChestType.Guarded == eChestType ) )
    {
      ScheduleRespawn();
    }
  }
}


class U2_Shield_Widget extends U2_Widget
{
  function CanOpen( i_ciPlayer )
  {
    local iRingIndex = GetProperty( U2_Ring_Index_PropertyID, -1 );
    local bGivesRing = ( iRingIndex != -1 );
    if( bGivesRing )
    {
      local bPlayerHasRing = i_ciPlayer.GetProperty( U2_Magic_Ring_PropertyID, false );
      if( !bPlayerHasRing )
      {
        // The player is allowed to take the ring if they have been given directions to it
        local uiRingQuestState = i_ciPlayer.GetProperty( U2_Ring_Quest_State_PropertyID, RingQuestState.Started );
        return ( RingQuestState.ReceivedDirections == uiRingQuestState );
      }

      return false;
    }

    return true;
  }


  function Open( i_ciPlayer, i_bOpenSpell )
  {
    SetVisibility( false );

    local iRingIndex = GetProperty( U2_Ring_Index_PropertyID, -1 );
    local bGivesRing = ( iRingIndex != -1 );
    if( bGivesRing )
    {
      i_ciPlayer.SetProperty( U2_Magic_Ring_PropertyID, true );
      i_ciPlayer.ActionInfo( msg_ring_found_client_StringID );
    }
    else
    {
      local ciMap = GetMap();
      local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
      if( eMapType != MapType.Dungeon && eMapType != MapType.Tower )
      {
        // See if the player was caught stealing
        if( !i_ciPlayer.SkillRoll( i_ciPlayer.GetDexterity() ) )
        {
          i_ciPlayer.AdjustVersionedProperty( g_eNotorietyPropertyVersionArray, 1 );
        }

        // Gain random armour
        local iRoll = rand()%100;
        local iArmourIndex = 0;
        foreach( iIndex, iValue in g_iU2ArmourInventoryPercentageArray )
        {
          iArmourIndex = iIndex;
          if( iRoll <= iValue )
          {
            break;
          }
        }

        local eArmourType = g_eU2ArmourInventoryArray[iArmourIndex];
        if( i_ciPlayer.AddOrCreateItem( eArmourType, 1 ) )
        {
          local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), eArmourType );
          i_ciPlayer.SendBroadcast( ciBroadcast );
        }
      }
    }

    ScheduleRespawn();
  }
}


class U2_Sword_Widget extends U2_Widget
{
  function CanOpen( i_ciPlayer )
  {
    local iQuickswordIndex = GetProperty( U2_Quicksword_Material_Index_PropertyID, -1 );
    if( iQuickswordIndex != -1 )
    {
      local uiMaterialFlags = i_ciPlayer.GetProperty( U2_Item_Quicksword_Materials_PropertyID, 0 );
      return !::rumBitOn( uiMaterialFlags, iQuickswordIndex );
    }

    return true;
  }


  function Open( i_ciPlayer, i_bOpenSpell )
  {
    SetVisibility( false );

    local iQuickswordIndex = GetProperty( U2_Quicksword_Material_Index_PropertyID, -1 );
    if( iQuickswordIndex != -1 )
    {
      local uiMaterialFlags = i_ciPlayer.GetProperty( U2_Item_Quicksword_Materials_PropertyID, 0 );
      uiMaterialFlags = ::rumBitSet( uiMaterialFlags, iQuickswordIndex );
      i_ciPlayer.SetProperty( U2_Item_Quicksword_Materials_PropertyID, uiMaterialFlags );
    }
    else
    {
      local ciMap = GetMap();
      local eMapType = ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
      if( eMapType != MapType.Dungeon && eMapType != MapType.Tower )
      {
        // See if the player was caught stealing
        if( !i_ciPlayer.SkillRoll( i_ciPlayer.GetDexterity() ) )
        {
          i_ciPlayer.AdjustVersionedProperty( g_eNotorietyPropertyVersionArray, 1 );
        }
      }

      // Gain a random weapon
      local iRoll = rand()%100;
      local iWeaponIndex = 0;
      foreach( iIndex, iValue in g_iU2WeaponInventoryPercentageArray )
      {
        iWeaponIndex = iIndex;
        if( iRoll <= iValue )
        {
          break;
        }
      }

      local eWeaponType = g_eU2WeaponInventoryArray[iWeaponIndex];
      if( i_ciPlayer.AddOrCreateItem( eWeaponType, 1 ) )
      {
        local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), eWeaponType );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
    }

    ScheduleRespawn();
  }
}


class U3_Chest_Widget extends U3_Widget
{
  function Open( i_ciPlayer, i_bOpenSpell )
  {
    if( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() )
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      return;
    }

    local bDestroyed = false;
    SetVisibility( false );

    // See if chest has a trap
    if( !i_bOpenSpell )
    {
      bDestroyed = TrapTrigger( i_ciPlayer );
    }

    if( !bDestroyed )
    {
      // Give contents to player
      local iChestGold = rand() % 99 + 1;

      // Send an update on the amount of gold found
      local ciBroadcast = ::rumCreate( Player_Get_BroadcastID, iChestGold );
      i_ciPlayer.SendBroadcast( ciBroadcast );

      i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, iChestGold );
    }

    local eChestType = GetProperty( Widget_Chest_Type_PropertyID, ChestType.Treasure );
    if( ChestType.Guarded == eChestType )
    {
      // See if the player was caught stealing
      if( !i_ciPlayer.SkillRoll( i_ciPlayer.GetDexterity() ) )
      {
        i_ciPlayer.AdjustVersionedProperty( g_eNotorietyPropertyVersionArray, 1 );
      }
    }

    // Set respawn timer if necessary
    if( ( ChestType.Treasure == eChestType ) || ( ChestType.Guarded == eChestType ) )
    {
      ScheduleRespawn();
    }
  }


  function TrapTrigger( i_ciPlayer )
  {
    local bDestroyed = false;

    // There is a 50% chance that the chest will be trapped
    local iTrapChance = rand() % 100;
    if( iTrapChance < 50 )
    {
      // Chest is definitely trapped, pre-check to see if player successfully avoids trap
      local bAvoided = i_ciPlayer.SkillRoll( i_ciPlayer.GetDexterity() );

      if( iTrapChance > 25 )
      {
        i_ciPlayer.ActionWarning( msg_acid_trap_client_StringID );

        if( !bAvoided )
        {
          local iDamage = 16 + rand() % 32;
          i_ciPlayer.Damage( iDamage, this );
        }
      }
      else if( iTrapChance > 15 )
      {
        i_ciPlayer.ActionWarning( msg_poison_trap_client_StringID );

        if( !bAvoided )
        {
          i_ciPlayer.Poison();
        }
      }
      else if( iTrapChance > 5 )
      {
        i_ciPlayer.ActionWarning( msg_gas_trap_client_StringID );

        if( !bAvoided )
        {
          local ciMap = GetMap();
          local ciPlayerArray = ciMap.GetPlayers( GetPosition(), 1, false );
          foreach( ciTarget in ciPlayerArray )
          {
            SendClientEffect( ciTarget, ClientEffectType.Damage );
            ciTarget.Poison();
          }
        }
      }
      else
      {
        i_ciPlayer.ActionWarning( msg_bomb_trap_client_StringID );

        if( !bAvoided )
        {
          bDestroyed = true;

          local ciMap = GetMap();
          local ciPlayerArray = ciMap.GetPlayers( GetPosition(), 1, false );
          foreach( ciTarget in ciPlayerArray )
          {
            if( ciTarget.IsVisible() && !ciTarget.IsDead() )
            {
              local iDamage = 32 + rand() % 32;
              ciTarget.Damage( iDamage, this );
            }
          }
        }
      }

      // Trap successfully evaded
      if( bAvoided )
      {
        i_ciPlayer.ActionSuccess( msg_evaded_client_StringID );
      }
    }

    return bDestroyed;
  }
}


class U4_Chest_Widget extends U4_Widget
{
  function Open( i_ciPlayer, i_bOpenSpell )
  {
    if( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() )
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      return;
    }

    local bDestroyed = false;
    SetVisibility( false );

    // See if chest has a trap
    if( !i_bOpenSpell )
    {
      bDestroyed = TrapTrigger( i_ciPlayer );
    }

    if( !bDestroyed )
    {
      // Give contents to player
      local iChestGold = rand() % 99 + 1;

      // Send an update on the amount of gold found
      local ciBroadcast = ::rumCreate( Player_Get_BroadcastID, iChestGold );
      i_ciPlayer.SendBroadcast( ciBroadcast );

      i_ciPlayer.AdjustVersionedProperty( g_eGoldPropertyVersionArray, iChestGold );
    }

    local eChestType = GetProperty( Widget_Chest_Type_PropertyID, 0 );
    if( ChestType.Guarded == eChestType )
    {
      // See if the player was caught stealing
      if( !i_ciPlayer.SkillRoll( i_ciPlayer.GetDexterity() ) )
      {
        i_ciPlayer.AdjustVersionedProperty( g_eNotorietyPropertyVersionArray, 1 );
      }

      // Deduct virtues for stealing
      i_ciPlayer.AffectVirtue( VirtueType.Honesty, -1, true, true );
      i_ciPlayer.AffectVirtue( VirtueType.Justice, -1, true, false );
      i_ciPlayer.AffectVirtue( VirtueType.Honor, -1, true, false );
    }

    // Set respawn timer if necessary
    if( ( ChestType.Treasure == eChestType ) || ( ChestType.Guarded == eChestType ) )
    {
      ScheduleRespawn();
    }
  }


  function TrapTrigger( i_ciPlayer )
  {
    local bDestroyed = false;

    // There is a 50% chance that the chest will be trapped
    local iTrapChance = rand() % 100;
    if( iTrapChance < 50 )
    {
      // Chest is definitely trapped, pre-check to see if player successfully avoids trap
      local bAvoided = false;

      if( iTrapChance > 25 )
      {
        i_ciPlayer.ActionWarning( msg_acid_trap_client_StringID );

        bAvoided = i_ciPlayer.SkillRoll( i_ciPlayer.GetDexterity() );
        if( !bAvoided )
        {
          local iDamage = 16 + rand() % 32;
          i_ciPlayer.Damage( iDamage, this );
        }
      }
      else if( iTrapChance > 15 )
      {
        i_ciPlayer.ActionWarning( msg_poison_trap_client_StringID );
        if( !i_ciPlayer.PoisonTrap() )
        {
          bAvoided = true;
        }
      }
      else if( iTrapChance > 5 )
      {
        i_ciPlayer.ActionWarning( msg_sleep_trap_client_StringID );
        if( !i_ciPlayer.Incapacitate() )
        {
          bAvoided = true;
        }
      }
      else
      {
        i_ciPlayer.ActionWarning( msg_bomb_trap_client_StringID );

        bAvoided = i_ciPlayer.SkillRoll( i_ciPlayer.GetDexterity() );
        if( !bAvoided )
        {
          bDestroyed = true;

          local iDamage = 32 + rand() % 32;
          i_ciPlayer.Damage( iDamage, this );

          for( local eDir = Direction.Start; eDir < Direction.End; ++eDir )
          {
            local ciField = ::rumCreate( U4_Field_Fire_WidgetID );
            if( ciField )
            {
              local ciMap = GetMap();
              local ciPos = i_ciPlayer.GetPosition() + GetDirectionVector( eDir );
              local eMoveResultType = ciMap.MovePawn( ciField, ciPos, rumIgnoreDistanceMoveFlag | rumTestMoveFlag );
              if( ( rumSuccessMoveResultType == eMoveResultType ) && ciMap.AddPawn( ciField, ciPos ) )
              {
                local fDuration = 5.0;
                ::rumSchedule( ciField, ciField.Expire, fDuration );
              }
            }
          }
        }
      }

      // Trap successfully evaded
      if( bAvoided )
      {
        i_ciPlayer.ActionSuccess( msg_evaded_client_StringID );
      }
    }

    return bDestroyed;
  }
}
