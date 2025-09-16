// Received from client
class Player_Mix_Broadcast extends rumBroadcast
{
  var1 = 0; // Spell to mix
  var2 = 0; // Reagent array
  var3 = 0; // Num mixtures


  function ConsumeReagents( i_ciPlayer, i_eReagentArray, i_iAmount )
  {
    foreach( eReagent in i_eReagentArray )
    {
      local ePropertyID = g_eU4ReagentPropertyArray[eReagent];
      local iNumReagents = i_ciPlayer.GetProperty( ePropertyID, 0 ) - i_iAmount;
      i_ciPlayer.SetProperty( ePropertyID, iNumReagents );
    }
  }


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return;
    }

    local eDelay = ActionDelay.Short;

    local ePlayerClassID = i_ciPlayer.GetProperty( U4_PlayerClass_PropertyID, U4_Mage_Class_CustomID );
    local ciPlayerClass = ::rumGetCustomAsset( ePlayerClassID );

    local bCastsSpells = ciPlayerClass.GetProperty( Class_Casts_Spells_PropertyID, false );
    if( !bCastsSpells )
    {
      // The player class cannot mix reagents and should've been blocked by the client
      i_ciPlayer.IncrementHackAttempts();
      return;
    }

    local bSuccess = false;

    local eSpellID = var1;
    if( ValueInContainer( eSpellID, g_eU4SpellArray ) )
    {
      local iPlayerMixFlags = var2;
      local iNumMixtures = var3;
      local iMaxMixtures = 99;

      local ciSpell = ::rumGetCustomAsset( eSpellID );
      local iSpellMixFlags = ciSpell.GetProperty( Spell_Mix_Component_Flags_PropertyID );

      // Create a reagent array of the player's mixture
      local eReagentArray = [];
      for( local i = 0; i < Reagents.NumReagents; ++i )
      {
        if( iPlayerMixFlags & ( 1 << i ) )
        {
          eReagentArray.push( i );
        }
      }

      // Success if the provided mix flags matches the required mix flags from the spell
      if( iPlayerMixFlags == iSpellMixFlags )
      {
        // Get the maximum amount of mixtures this player can make
        foreach( eReagent in eReagentArray )
        {
          local ePropertyID = g_eU4ReagentPropertyArray[eReagent];
          local iReagents = i_ciPlayer.GetProperty( ePropertyID, 0 );
          iMaxMixtures = min( iReagents, iMaxMixtures );
        }

        if( iMaxMixtures > 0 )
        {
          local ePropertyID = ciSpell.GetProperty( Spell_ID_PropertyID, 0 );
          local iExistingMixtures = i_ciPlayer.GetProperty( ePropertyID, 0 );

          // Clamp to the maximum amount of mixtures a player can hold
          iMaxMixtures = min( 99 - iExistingMixtures, iMaxMixtures );

          // Can the player mix as many as requested?
          if( iNumMixtures > iMaxMixtures )
          {
            // TODO: Warn player that they can't mix as many as they requested?
            iNumMixtures = iMaxMixtures;
          }

          if( iNumMixtures > 0 )
          {
            // Give the player the requested amount of mixtures
            i_ciPlayer.SetProperty( ePropertyID, iExistingMixtures + iNumMixtures );

            ConsumeReagents( i_ciPlayer, eReagentArray, iNumMixtures );
          }
        }

        i_ciPlayer.ActionSuccess( msg_success_client_StringID );

        // Success even if 0 mixtures were made
        bSuccess = true;

        eDelay = ActionDelay.Long;
      }
      else
      {
        // Consume the reagents from the single fizzled mixture
        ConsumeReagents( i_ciPlayer, eReagentArray, 1 );
      }
    }

    if( !bSuccess )
    {
      i_ciPlayer.ActionFailed( msg_mix_fizzles_client_StringID );
    }

    local fDelay = i_ciPlayer.GetActionDelay( eDelay );
    ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
  }
}
