// Received from client
class Player_Cast_Broadcast extends rumBroadcast
{
  var1 = 0; // Spell ID
  var2 = 0; // Direction
  var3 = 0; // Special type
  var4 = 0; // Target ID


  function OnRecv( i_iSocket, i_ciPlayer )
  {
    if( !( i_ciPlayer instanceof Player ) )
    {
      return ActionDelay.Short;
    }

    local eDelay = ActionDelay.Short;

    local ciPlayerClass = i_ciPlayer.GetPlayerClass();
    if( null == ciPlayerClass )
    {
      return ActionDelay.Short;
    }

    local bCanCast = ciPlayerClass.GetProperty( Class_Casts_Spells_PropertyID, false );
    if( bCanCast )
    {
      // Convert spell ID to spell
      local eSpellID = var1;
      local eDir = var2;
      local eType = var3;
      local ciTarget = null;

      local ciSpell = ::rumGetCustomAsset( eSpellID );

      local bTargetable = ciSpell.GetProperty( Spell_Targetable_PropertyID );
      if( bTargetable )
      {
        ciTarget = ::rumFetchPawn( var4 );
      }

      local iClassFlags = ciSpell.GetProperty( Spell_Class_Compatibility_Flags_PropertyID );
      local bCompatible = iClassFlags & ( 1 << ciPlayerClass.GetProperty( Class_ID_PropertyID, 0 ) );
      if( bCompatible )
      {
        i_ciPlayer.CastSpell( eSpellID, eDir, eType, ciTarget );
        eDelay = ActionDelay.Medium;
      }
      else
      {
        // The player cannot cast the specified spell because of a class restriction, which should've been caught by
        // the client
        i_ciPlayer.IncrementHackAttempts();
      }
    }
    else
    {
      // The player class cannot cast spells and should've been blocked by the client
      i_ciPlayer.IncrementHackAttempts();
    }

    local fDelay = i_ciPlayer.GetActionDelay( eDelay );
    ::rumSchedule( i_ciPlayer, i_ciPlayer.ProcessNextPacket, fDelay );
  }
}
