function Cast( i_ciSpell, i_ciCaster, i_vValue, i_eType )
{
  local ciTarget = i_vValue;
  local eDir = i_vValue;

  if( null == i_ciSpell )
  {
    return false;
  }

  SendClientEffect( i_ciCaster, ClientEffectType.Cast );

  local eSpellID = i_ciSpell.GetAssetID();
  switch( eSpellID )
  {
    ///////////////
    // U4 Spells //
    ///////////////

    case U4_Awaken_Spell_CustomID:        return CastAwaken( i_ciCaster, ciTarget, i_ciSpell );
    case U4_Blink_Spell_CustomID:         return CastBlink( i_ciCaster, i_ciSpell, eDir );
    case U4_Cure_Spell_CustomID:          return CastCure( i_ciCaster, ciTarget, i_ciSpell, true /* affect virtue */ );
    case U4_Dispell_Spell_CustomID:       return CastDispell( i_ciCaster, eDir, U4_Magic_Field_Widget );
    case U4_Energy_Field_Spell_CustomID:
    {
      local eWidgetID = rumInvalidAssetID;

      switch( i_eType )
      {
        case U4_MagicFieldType.Fire:      eWidgetID = U4_Field_Fire_WidgetID;      break;
        case U4_MagicFieldType.Lightning: eWidgetID = U4_Field_Lightning_WidgetID; break;
        case U4_MagicFieldType.Poison:    eWidgetID = U4_Field_Poison_WidgetID;    break;
        case U4_MagicFieldType.Sleep:     eWidgetID = U4_Field_Sleep_WidgetID;     break;
      }

      local ciPos = i_ciCaster.GetPosition();
      local fDuration = i_ciSpell.GetProperty( Spell_Duration_PropertyID, 10.0 );
      return CastEnergyField( i_ciCaster, ciPos, eDir, eWidgetID, fDuration, true /* primary field */ );
    }
    case U4_Fireball_Spell_CustomID:      return CastProjectile( i_ciCaster, ciTarget,
                                                                 U4_Fireball_Weapon_InventoryID );
    case U4_Gate_Travel_Spell_CustomID:   return CastGateTravel( i_ciCaster, eDir );
    case U4_Heal_Spell_CustomID:
    {
      // Heal 200-399 Points
      local iAmount = rand()%200 + 200;
      return CastHeal( i_ciCaster, ciTarget, i_ciSpell, iAmount, true /* affect virtue */ );
    }
    case U4_Iceball_Spell_CustomID:       return CastProjectile( i_ciCaster, ciTarget, U4_Iceball_Weapon_InventoryID );
    case U4_Jinx_Spell_CustomID:          return CastJinx( i_ciCaster, i_ciSpell );
    case U4_Kill_Spell_CustomID:          return CastKill( i_ciCaster, ciTarget, i_ciSpell );
    case U4_Light_Spell_CustomID:         return CastLight( i_ciCaster, i_ciSpell );
    case U4_Magic_Missile_Spell_CustomID: return CastProjectile( i_ciCaster, ciTarget,
                                                                 U4_Magic_Missile_Weapon_InventoryID );
    case U4_Negate_Spell_CustomID:        return CastNegateMagic( i_ciCaster, i_ciSpell );
    case U4_Open_Spell_CustomID:          return CastOpen( i_ciCaster, U4_Chest_WidgetID );
    case U4_Protection_Spell_CustomID:    return CastProtection( i_ciCaster, i_ciSpell );
    case U4_Quickness_Spell_CustomID:
    {
      // The quicken spell will speed up as many party members as the caster's level
      local iNumTargets = i_ciCaster.GetProperty( U4_Level_PropertyID, 1 );
      return CastQuickness( i_ciCaster, i_ciSpell, iNumTargets );
    }
    case U4_Resurrect_Spell_CustomID:     return CastResurrect( i_ciCaster, eDir, null, U4_Body_Dead_WidgetID,
                                                                true /* affect virtue */ );
    case U4_Sleep_Spell_CustomID:         return CastSleep( i_ciCaster, i_ciSpell );
    case U4_Tremor_Spell_CustomID:        return CastTremor( i_ciCaster, i_ciSpell );
    case U4_Undead_Spell_CustomID:
    {
      // Damage as many targets as the caster's level (up to 8)
      local iNumTargets = i_ciCaster.GetProperty( U4_Level_PropertyID, 1 );
      iNumTargets = clamp( iNumTargets, 1, 8 );
      return CastRangedDamage( i_ciCaster, i_ciSpell, iNumTargets );
    }
    case U4_View_Spell_CustomID:          return CastView( i_ciCaster );
    case U4_Wind_Change_Spell_CustomID:   return CastWindChange( i_ciCaster, i_ciSpell, eDir );
    case U4_Xit_Spell_CustomID:           return CastXit( i_ciCaster, i_ciSpell );
    case U4_Yup_Spell_CustomID:           return CastYUp( i_ciCaster, i_ciSpell );
    case U4_Zdown_Spell_CustomID:         return CastZDown( i_ciCaster, i_ciSpell );

    ///////////////
    // U3 Spells //
    ///////////////

    case U3_Alcort_Spell_CustomID:        return CastCure( i_ciCaster, ciTarget, i_ciSpell,
                                                           false /* does not affect virtue */ );
    case U3_Altair_Spell_CustomID:
    {
      // Note - the original spell Negates time, except such a thing can't really exist in an MMO. Altair means "The
      // Flyer" or "Flying Eagle" in Arabic, so it's quite possibly more related to Quickness than anything else as a
      // substitute. The quicken spell will speed up as many party members as the caster's level.
      local iNumTargets = i_ciCaster.GetProperty( U3_Level_PropertyID, 1 );
      iNumTargets = clamp( iNumTargets, 1, 8 );
      return CastQuickness( i_ciCaster, i_ciSpell, iNumTargets );
    }
    case U3_Anju_Sermani_Spell_CustomID:  return CastResurrect( i_ciCaster, eDir, null, U3_Ashes_WidgetID,
                                                                false /* does not affect virtue */ );
    case U3_Appar_Unem_Spell_CustomID:    return CastOpen( i_ciCaster, U3_Chest_WidgetID );
    case U3_Dag_Acron_Spell_CustomID:
    {
      // Note - the original spell would teleport the caster and party to a random overworld location, but blink is
      // far more aligned with the overall lore and a much more useful spell in general.
      return CastBlink( i_ciCaster, i_ciSpell, eDir );
    }
    case U3_Dag_Lorum_Spell_CustomID:     return CastLight( i_ciCaster, i_ciSpell );
    case U3_Dag_Mentar_Spell_CustomID:
    {
      // Damage as many targets as the caster's level (up to 8)
      local iNumTargets = i_ciCaster.GetProperty( U3_Level_PropertyID, 1 );
      iNumTargets = clamp( iNumTargets, 1, 8 );
      return CastRangedDamage( i_ciCaster, i_ciSpell, iNumTargets );
    }
    case U3_Decorp_Spell_CustomID:        return CastKill( i_ciCaster, ciTarget, i_ciSpell );
    case U3_Dor_Acron_Spell_CustomID:     return CastZDown( i_ciCaster, i_ciSpell );
    case U3_Excuun_Spell_CustomID:        return CastKill( i_ciCaster, ciTarget, i_ciSpell );
    case U3_Fal_Divi_Sequitu_Spell_CustomID:
    {
      // Note - the original was called Fal Divi meaning something like "False God" and would let any arcane class cast
      // any divine spell. That makes divine classes a little too overpowered, and defeats the purpose of having one or
      // the other as well as the hybrid classes. Removing that in favor of giving arcane classes the one additional
      // divine power of exiting a dungeon, since both have overlaps with moving up or down a single dungeon level.
      return CastXit( i_ciCaster, i_ciSpell );
    }
    case U3_Fulgar_Spell_CustomID:        return CastProjectile( i_ciCaster, ciTarget,
                                                                 U3_Fireball_Weapon_InventoryID );
    case U3_Lib_Rec_Spell_CustomID:
    {
      // Note - the original spell would teleport the caster and party to a random overworld location, but blink is
      // far more aligned with the overall lore and a much more useful spell in general.
      return CastBlink( i_ciCaster, i_ciSpell, eDir );
    }
    case U3_Lorum_Spell_CustomID:
    case U3_Luminae_Spell_CustomID:       return CastLight( i_ciCaster, i_ciSpell );
    case U3_Mentar_Spell_CustomID:
    {
      // Damage as many targets as the caster's level (up to 8)
      local iNumTargets = i_ciCaster.GetProperty( U3_Level_PropertyID, 1 );
      iNumTargets = clamp( iNumTargets, 1, 8 );
      return CastRangedDamage( i_ciCaster, i_ciSpell, iNumTargets );
    }
    case U3_Mittar_Spell_CustomID:        return CastProjectile( i_ciCaster, ciTarget,
                                                                 U3_Magic_Missile_Weapon_InventoryID );
    case U3_Necorp_Spell_CustomID:
    {
      // Damage as many targets as the caster's level (up to 8)
      local iNumTargets = i_ciCaster.GetProperty( U3_Level_PropertyID, 1 );
      iNumTargets = clamp( iNumTargets, 1, 8 );
      return CastRangedDamage( i_ciCaster, i_ciSpell, iNumTargets );
    }
    case U3_Noxum_Spell_CustomID:
    {
      // Damage as many targets as the caster's level (up to 8)
      local iNumTargets = i_ciCaster.GetProperty( U3_Level_PropertyID, 1 );
      iNumTargets = clamp( iNumTargets, 1, 8 );
      CastProjectileGroup( i_ciCaster, i_ciSpell, U3_Fireball_Weapon_InventoryID, iNumTargets );
    }
    case U3_Pontori_Spell_CustomID:
    {
      local iNumTargets = 8;
      return CastKillGroup( i_ciCaster, i_ciSpell, iNumTargets );
    }
    case U3_Rec_Du_Spell_CustomID:        return CastZDown( i_ciCaster, i_ciSpell );
    case U3_Rec_Su_Spell_CustomID:        return CastYUp( i_ciCaster, i_ciSpell );
    case U3_Repond_Spell_CustomID:
    {
      local iNumTargets = 8;
      return CastKillGroup( i_ciCaster, i_ciSpell, iNumTargets );
    }
    case U3_Sanctu_Spell_CustomID:
    {
      // Heal 200-399 Points
      local iAmount = rand()%200 + 200;
      return CastHeal( i_ciCaster, ciTarget, i_ciSpell, iAmount );
    }
    case U3_Sanctu_Mani_Spell_CustomID:   return CastHeal( i_ciCaster, ciTarget, i_ciSpell,
                                                           ciTarget.GetMaxHitpoints() );
    case U3_Sequitu_Spell_CustomID:       return CastXit( i_ciCaster, i_ciSpell );
    case U3_Sominae_Spell_CustomID:       return CastLight( i_ciCaster, i_ciSpell );
    case U3_Sur_Acron_Spell_CustomID:     return CastYUp( i_ciCaster );
    case U3_Surmandum_Spell_CustomID:     return CastResurrect( i_ciCaster, eDir, U3_Wisdom_PropertyID,
                                                                U4_Body_Dead_WidgetID );
    case U3_Unnamed_Spell_CustomID:
    {
      // Damage as many targets as the caster's level (up to 8)
      local iNumTargets = i_ciCaster.GetProperty( U3_Level_PropertyID, 1 );
      iNumTargets = clamp( iNumTargets, 1, 8 );
      return CastKillGroup( i_ciCaster, i_ciSpell, iNumTargets );
    }
    case U3_Vieda_Spell_CustomID:         return CastView( i_ciCaster );
    case U3_Zxkuqyb_Spell_CustomID:
    {
      // Damage as many targets as the caster's level (up to 8)
      local iNumTargets = i_ciCaster.GetProperty( U3_Level_PropertyID, 1 );
      iNumTargets = clamp( iNumTargets, 1, 8 );
      return CastKillGroup( i_ciCaster, i_ciSpell, iNumTargets );
    }

    ///////////////
    // U2 Spells //
    ///////////////

    // Note: The original spell would put the player at a random location in the dungeon. Meh. It's far more
    // interesting to let the player learn shortcuts and use the U4 style Blink spell to shortcut the dungeon.
    case U2_Blink_Spell_CustomID:         return CastBlink( i_ciCaster, i_ciSpell, eDir );
    case U2_Down_Spell_CustomID:          return CastZDown( i_ciCaster, i_ciSpell );
    case U2_Heal_Spell_CustomID:
    {
      // Heal 200-399 Points
      local iAmount = rand()%200 + 200;
      return CastHeal( i_ciCaster, ciTarget, i_ciSpell, iAmount );
    }
    case U2_Kill_Spell_CustomID:          return CastKill( i_ciCaster, ciTarget, i_ciSpell );
    case U2_Light_Spell_CustomID:         return CastLight( i_ciCaster, i_ciSpell );
    case U2_Magic_Missile_Spell_CustomID: return CastProjectile( i_ciCaster, ciTarget, U2_Magic_Missile_Spell_CustomID );
    case U2_Prayer_Spell_CustomID:        return CastPrayer( i_ciCaster, iRange, U2_Wisdom_PropertyID );
    case U2_Surface_Spell_CustomID:       return CastXit( i_ciCaster, i_ciSpell );
    case U2_Up_Spell_CustomID:            return CastYUp( i_ciCaster );

    ///////////////
    // U1 Spells //
    ///////////////

    // Note: The original spell would put the player at a random location in the dungeon. Meh. It's far more
    // interesting to let the player learn shortcuts and use the U4 style Blink spell to shortcut the dungeon.
    case U1_Blink_Spell_CustomID:         return CastBlink( i_ciCaster, i_ciSpell, eDir );
    case U1_Create_Spell_CustomID:
    {
      local ciPos = i_ciCaster.GetPosition();
      local fDuration = i_ciSpell.GetProperty( Spell_Duration_PropertyID, 10.0 );
      return CastEnergyField( i_ciCaster, ciPos, eDir, U1_Barrier_WidgetID, fDuration, true /* primary field */ );
    }
    case U1_Destroy_Spell_CustomID:       return CastDispell( i_ciCaster, eDir, U1_Barrier_Widget );
    case U1_Down_Spell_CustomID:          return CastZDown( i_ciCaster, i_ciSpell );
    case U1_Heal_Spell_CustomID:
    {
      // Heal 200-399 Points
      local iAmount = rand()%200 + 200;
      return CastHeal( i_ciCaster, ciTarget, i_ciSpell, iAmount );
    }
    case U1_Kill_Spell_CustomID:          return CastKill( i_ciCaster, ciTarget, i_ciSpell );
    case U1_Light_Spell_CustomID:         return CastLight( i_ciCaster, i_ciSpell );
    case U1_Magic_Missile_Spell_CustomID: return CastProjectile( i_ciCaster, ciTarget,
                                                                 U1_Magic_Missile_Weapon_InventoryID );
    case U1_Mind_Blaster_Spell_CustomID:  return CastNegateMagic( i_ciCaster, i_ciSpell ); // Original spell reduced stats
    case U1_Open_Spell_CustomID:          return CastOpen( i_ciCaster, U1_Chest_WidgetID );
    case U1_Prayer_Spell_CustomID:        return CastPrayer( i_ciCaster, iRange, U1_Wisdom_PropertyID );
    case U1_Psionic_Shock_Spell_CustomID: return CastRangedDamage( i_ciCaster, i_ciSpell, 1 );
    case U1_Surface_Spell_CustomID:       return CastXit( i_ciCaster, i_ciSpell );
    case U1_Up_Spell_CustomID:            return CastYUp( i_ciCaster );
  }

  return false;
}
