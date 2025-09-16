class U4_Altar_Widget extends U4_Widget
{
  function Use( i_ciPlayer, i_vValue )
  {
    if( i_ciPlayer.IsDead() || i_ciPlayer.IsIncapacitated() )
    {
      i_ciPlayer.ActionFailed( msg_cant_client_StringID );
      return;
    }

    local eAltarType = GetProperty( Widget_Altar_Type_PropertyID, 0 );
    if( AltarType.Dungeon == eAltarType )
    {
      local iStoneIndicesArray = i_vValue;
      local bSuccess = false;

      if( iStoneIndicesArray.len() == 4 )
      {
        local iFlags = i_ciPlayer.GetProperty( U4_Item_Three_Part_Key_PropertyID, 0 );
        local ciMap = GetMap();
        local eMapID = ciMap.GetAssetID();
        if( U4_Altar_Courage_MapID == eMapID )
        {
          // Player should not already have this part of the Three Part Key
          if( !::rumBitOn( iFlags, PrincipleType.Courage ) &&
              iStoneIndicesArray[0] == VirtueType.Valor && iStoneIndicesArray[1] == VirtueType.Sacrifice &&
              iStoneIndicesArray[2] == VirtueType.Honor && iStoneIndicesArray[3] == VirtueType.Spirituality )
          {
            iFlags = ::rumBitSet( iFlags, PrincipleType.Courage );
            i_ciPlayer.SetProperty( U4_Item_Three_Part_Key_PropertyID, iFlags );
            bSuccess = true;
          }
        }
        else if( U4_Altar_Love_MapID == eMapID )
        {
          // Player should not already have this part of the Three Part Key
          if( !::rumBitOn( iFlags, PrincipleType.Love ) &&
              iStoneIndicesArray[0] == VirtueType.Compassion && iStoneIndicesArray[1] == VirtueType.Justice &&
              iStoneIndicesArray[2] == VirtueType.Sacrifice && iStoneIndicesArray[3] == VirtueType.Spirituality )
          {
            iFlags = ::rumBitSet( iFlags, PrincipleType.Love );
            i_ciPlayer.SetProperty( U4_Item_Three_Part_Key_PropertyID, iFlags );
            bSuccess = true;
          }
        }
        else if( U4_Altar_Truth_MapID == eMapID )
        {
          // Player should not already have this part of the Three Part Key
          if( !::rumBitOn( iFlags, PrincipleType.Truth ) &&
              iStoneIndicesArray[0] == VirtueType.Honesty && iStoneIndicesArray[1] == VirtueType.Justice &&
              iStoneIndicesArray[2] == VirtueType.Honor && iStoneIndicesArray[3] == VirtueType.Spirituality )
          {
            iFlags = ::rumBitSet( iFlags, PrincipleType.Truth );
            i_ciPlayer.SetProperty( U4_Item_Three_Part_Key_PropertyID, iFlags );
            bSuccess = true;
          }
        }
      }

      if( bSuccess )
      {
        i_ciPlayer.ActionSuccess( msg_found_key_client_StringID );
        i_ciPlayer.AffectVirtue( VirtueType.Honor, 2, true, true );
      }
      else
      {
        i_ciPlayer.ActionFailed( msg_hmmm_no_effect_client_StringID );
      }
    }
    else if( AltarType.Abyss == eAltarType )
    {
      local ciMap = GetMap();
      local ciBroadcast = ::rumCreate( Abyss_Altar_Test_BroadcastID, U4_AbyssAltarPhaseType.Virtue );
      i_ciPlayer.SendBroadcast( ciBroadcast );
    }
  }
}
