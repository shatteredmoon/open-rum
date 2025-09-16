class U1_Signpost_Widget extends U1_Widget
{
  static s_iStatReward = 5;


  function Use( i_ciPlayer )
  {
    if( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() )
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      return;
    }

    local eSignpostType = GetProperty( U1_Signpost_Type_PropertyID, -1 );
    if( -1 == eSignpostType )
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      return;
    }

    local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), U1_SignPostUsageType.Description,
                                     eSignpostType );
    i_ciPlayer.SendBroadcast( ciBroadcast );

    local eQuestPropertyID = g_tblU1SignQuestArray[eSignpostType].ePropertyID;
    local eQuestState = g_tblU1SignQuestArray[eSignpostType].eState;

    // Is the player on this quest?
    local ePlayerQuestState = i_ciPlayer.GetProperty( eQuestPropertyID, U1_KingQuestState.Unbestowed1 );
    if( ePlayerQuestState == eQuestState )
    {
      // The rewards are very different from the original U1. For one, property rewards are always 5 points, the
      // Eastern Signpost gives a random stat increase, and the Grave of the Lost Soul gives strength instead of
      // stamina. Weapons received from the Pillars of the Argonauts are always random, and more valuable weapons
      // are less likely to be received.

      local strDesc = format( "u1_sign_%d_server_StringID", eSignpostType );
      strDesc = ::rumGetStringByName( strDesc, i_ciPlayer.m_iLanguageID );

      ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), U1_SignPostUsageType.QuestComplete, strDesc );
      i_ciPlayer.SendBroadcast( ciBroadcast );

      // Give sign reward
      local vReward = g_tblU1SignQuestArray[eSignpostType].vReward;

      if( null == vReward )
      {
        // Increase a random stat
        local ePropertyID = g_eU1StatPropertyArray[rand() % g_eU1StatPropertyArray.len()];
        i_ciPlayer.SetProperty( ePropertyID, i_ciPlayer.GetProperty( ePropertyID, 0 ) + s_iStatReward );

        ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), U1_SignPostUsageType.RewardProperty,
                                   ePropertyID, s_iStatReward );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else if( vReward == g_eU1WeaponInventoryArray )
      {
        local eWeaponType = U1_Mace_Weapon_InventoryID;

        // Give a random weapon
        local iRoll = rand() % 210;
        if( iRoll >= 208 )
        {
          eWeaponType = U1_Blaster_Weapon_InventoryID;
        }
        else if( iRoll >= 204 )
        {
          eWeaponType = U1_Phazor_Weapon_InventoryID;
        }
        else if( iRoll >= 192 )
        {
          eWeaponType = U1_Light_Sword_Weapon_InventoryID;
        }
        else if( iRoll >= 180 )
        {
          eWeaponType = U1_Pistol_Weapon_InventoryID;
        }
        else if( iRoll >= 168 )
        {
          eWeaponType = U1_Triangle_Weapon_InventoryID;
        }
        else if( iRoll >= 154 )
        {
          eWeaponType = U1_Staff_Weapon_InventoryID;
        }
        else if( iRoll >= 138 )
        {
          eWeaponType = U1_Wand_Weapon_InventoryID;
        }
        else if( iRoll >= 120 )
        {
          eWeaponType = U1_Amulet_Weapon_InventoryID;
        }
        else if( iRoll >= 100 )
        {
          eWeaponType = U1_Bow_Weapon_InventoryID;
        }
        else if( iRoll >= 78 )
        {
          eWeaponType = U1_Great_Sword_Weapon_InventoryID;
        }
        else if( iRoll >= 54 )
        {
          eWeaponType = U1_Sword_Weapon_InventoryID;
        }
        else if( iRoll >= 28 )
        {
          eWeaponType = U1_Axe_Weapon_InventoryID;
        }

        // Inform the client of the reward
        ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), U1_SignPostUsageType.RewardWeapon,
                                   eWeaponType );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }
      else
      {
        // Pre-determined stat increased, based on the quest/sign type
        i_ciPlayer.SetProperty( vReward, i_ciPlayer.GetProperty( vReward, 0 ) + s_iStatReward );

        ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), U1_SignPostUsageType.RewardProperty,
                                   vReward, s_iStatReward );
        i_ciPlayer.SendBroadcast( ciBroadcast );
      }

      // Advance quest
      i_ciPlayer.SetProperty( eQuestPropertyID, ePlayerQuestState + 1 );
      i_ciPlayer.ActionSuccess( u1_king_quest_completed_client_StringID );
    }
  }
}


class U2_Signpost_Widget extends U2_Widget
{
  function Use( i_ciPlayer )
  {
    if( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() )
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      return;
    }

    local eSignpostType = GetProperty( U2_Signpost_Type_PropertyID, -1 );
    if( -1 == eSignpostType )
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      return;
    }

    // Send the sign type to the client so that it can display the sign details
    local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), eSignpostType );
    i_ciPlayer.SendBroadcast( ciBroadcast );

    // Does the player know what period this is?
    local iFlags = i_ciPlayer.GetProperty( U2_TimePeriods_PropertyID, 0 );
    if( !::rumBitOn( iFlags, eSignpostType ) )
    {
      // Remember that the player now knows the period
      iFlags = ::rumBitSet( iFlags, eSignpostType );
      i_ciPlayer.SetProperty( U2_TimePeriods_PropertyID, iFlags );
    }
  }
}


class U3_Signpost_Widget extends U3_Widget
{
  function Use( i_ciPlayer )
  {
    if( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() )
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      return;
    }

    local iMessageID = GetProperty( U3_Signpost_Index_PropertyID, -1 );
    if( -1 == iMessageID )
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      return;
    }

    local strDesc = format( "u3_sign_%d_server_StringID", iMessageID );
    strDesc = ::rumGetStringByName( strDesc, i_ciPlayer.m_iLanguageID );

    local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetAssetID(), strDesc );
    i_ciPlayer.SendBroadcast( ciBroadcast );
  }
}
