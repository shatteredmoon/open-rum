class U1_Door_Widget extends U1_Widget
{
  static s_fLockTime = 60.0;
  static s_fCloseTime = 5.0;


  function Close()
  {
    if( !IsVisible() )
    {
      // Close the door
      SetVisibility( true );
    }
  }


  function OnCollisionTest( i_ciObject )
  {
    if( ( i_ciObject instanceof NPC ) && i_ciObject.IsPathing() )
    {
      if( i_ciObject.GetProperty( Creature_Humanoid_PropertyID, false ) )
      {
        // Not considered a collision since the NPC is capable of opening the door
        return false;
      }
    }

    return true;
  }


  function Open( i_ciCreature )
  {
    // Hide the door
    SetVisibility( false );

    if( i_ciCreature instanceof Player )
    {
      i_ciCreature.ActionSuccess( msg_opened_client_StringID );
    }

    ::rumSchedule( this, Close, s_fCloseTime );
  }
}


class U1_Door_Locked_Widget extends U1_Door_Widget
{
  function Lock()
  {
    local bLocked = GetProperty( Locked_PropertyID, false );
    if( !bLocked )
    {
      // Lock the door
      SetProperty( Locked_PropertyID, true );
    }
  }


  function Open( i_ciCreature )
  {
    local bLocked = GetProperty( Locked_PropertyID, true );
    if( bLocked )
    {
      if( i_ciCreature instanceof Player )
      {
        i_ciCreature.ActionFailed( msg_locked_client_StringID );
      }
    }
    else
    {
      base.Open( i_ciCreature );
    }
  }


  function Unlock( i_ciPlayer )
  {
    local iKeys = i_ciPlayer.GetProperty( U1_Keys_PropertyID, 0 );
    if( iKeys > 0 )
    {
      local bLocked = GetProperty( Locked_PropertyID, true );
      if( bLocked )
      {
        SetProperty( Locked_PropertyID, false );
        ::rumSchedule( this, Lock, s_fLockTime );

        // Unlock successful, inform client
        i_ciPlayer.ActionSuccess( msg_unlocked_client_StringID );

        // Deduct a key
        i_ciPlayer.SetProperty( U1_Keys_PropertyID, iKeys - 1 );
      }
      else
      {
        // The door is already unlocked
        i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_no_keys_client_StringID );
    }
  }
}


class U1_Door_Princess_Widget extends U1_Door_Locked_Widget
{
  function Unlock( i_ciPlayer )
  {
    local iKeys = i_ciPlayer.GetProperty( U1_Keys_PropertyID, 0 );
    if( iKeys > 0 )
    {
      // Unlock successful, inform client
      i_ciPlayer.ActionSuccess( msg_unlocked_client_StringID );

      // Deduct a key
      i_ciPlayer.SetProperty( U1_Keys_PropertyID, iKeys - 1 );

      // Move the player into the princess' room
      local ciMap = GetMap();
      local ciPos = GetPosition();

      ciMap.MovePawn( i_ciPlayer, ciPos,
                      rumIgnoreTileCollisionMoveFlag | rumIgnorePawnCollisionMoveFlag |
                      rumIgnoreDistanceMoveFlag );

      i_ciPlayer.ActionSuccess( msg_found_princess_client_StringID );
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_no_keys_client_StringID );
    }
  }
}


class U2_Door_Widget extends U2_Widget
{
  static s_fLockTime = 60.0;
  static s_fCloseTime = 5.0;


  function Close()
  {
    if( !IsVisible() )
    {
      // Close the door
      SetVisibility( true );
    }
  }


  function OnCollisionTest( i_ciObject )
  {
    if( ( i_ciObject instanceof NPC ) && i_ciObject.IsPathing() )
    {
      if( i_ciObject.GetProperty( Creature_Humanoid_PropertyID, false ) )
      {
        // Not considered a collision since the NPC is capable of opening the door
        return false;
      }
    }

    return true;
  }


  function Open( i_ciCreature )
  {
    // Hide the door
    SetVisibility( false );

    if( i_ciCreature instanceof Player )
    {
      i_ciCreature.ActionSuccess( msg_opened_client_StringID );
    }

    ::rumSchedule( this, Close, s_fCloseTime );
  }
}


class U2_Door_Locked_Widget extends U2_Door_Widget
{
  function Lock()
  {
    local bLocked = GetProperty( Locked_PropertyID, false );
    if( !bLocked )
    {
      // Lock the door
      SetProperty( Locked_PropertyID, true );
    }
  }


  function Open( i_ciCreature )
  {
    local bLocked = GetProperty( Locked_PropertyID, true );
    if( bLocked )
    {
      if( i_ciCreature instanceof Player )
      {
        i_ciCreature.ActionFailed( msg_locked_client_StringID );
      }
    }
    else
    {
      base.Open( i_ciCreature );
    }
  }


  function Unlock( i_ciPlayer )
  {
    local iKeys = i_ciPlayer.GetProperty( U2_Keys_PropertyID, 0 );
    if( iKeys > 0 )
    {
      local bLocked = GetProperty( Locked_PropertyID, true );
      if( bLocked )
      {
        SetProperty( Locked_PropertyID, false );
        ::rumSchedule( this, Lock, s_fLockTime );

        // Unlock successful, inform client
        i_ciPlayer.ActionSuccess( msg_unlocked_client_StringID );

        // Deduct a key
        i_ciPlayer.SetProperty( U2_Keys_PropertyID, iKeys - 1 );
      }
      else
      {
        // The door is already unlocked
        i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_no_keys_client_StringID );
    }
  }
}


class U3_Door_Widget extends U3_Widget
{
  static s_fLockTime = 60.0;
  static s_fCloseTime = 5.0;


  function Close()
  {
    if( !IsVisible() )
    {
      // Close the door
      SetVisibility( true );
    }
  }


  function OnCollisionTest( i_ciObject )
  {
    if( ( i_ciObject instanceof NPC ) && i_ciObject.IsPathing() )
    {
      if( i_ciObject.GetProperty( Creature_Humanoid_PropertyID, false ) )
      {
        // Not considered a collision since the NPC is capable of opening the door
        return false;
      }
    }

    return true;
  }


  function Open( i_ciCreature )
  {
    // Hide the door
    SetVisibility( false );

    if( i_ciCreature instanceof Player )
    {
      i_ciCreature.ActionSuccess( msg_opened_client_StringID );
    }

    ::rumSchedule( this, Close, s_fCloseTime );
  }
}


class U3_Door_Locked_Widget extends U3_Door_Widget
{
  function Lock()
  {
    local bLocked = GetProperty( Locked_PropertyID, false );
    if( !bLocked )
    {
      // Lock the door
      SetProperty( Locked_PropertyID, true );
    }
  }


  function Open( i_ciCreature )
  {
    local bLocked = GetProperty( Locked_PropertyID, true );
    if( bLocked )
    {
      if( i_ciCreature instanceof Player )
      {
        i_ciCreature.ActionFailed( msg_locked_client_StringID );
      }
    }
    else
    {
      base.Open( i_ciCreature );
    }
  }


  function Unlock( i_ciPlayer )
  {
    local iKeys = i_ciPlayer.GetProperty( U3_Keys_PropertyID, 0 );
    if( iKeys > 0 )
    {
      local bLocked = GetProperty( Locked_PropertyID, true );
      if( bLocked )
      {
        SetProperty( Locked_PropertyID, false );
        ::rumSchedule( this, Lock, s_fLockTime );

        // Unlock successful, inform client
        i_ciPlayer.ActionSuccess( msg_unlocked_client_StringID );

        // Deduct a key
        i_ciPlayer.SetProperty( U3_Keys_PropertyID, iKeys - 1 );
      }
      else
      {
        // The door is already unlocked
        i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_no_keys_client_StringID );
    }
  }
}


class U4_Door_Widget extends U4_Widget
{
  static s_fLockTime = 60.0;
  static s_fCloseTime = 5.0;


  function Close()
  {
    if( !IsVisible() )
    {
      // Close the door
      SetVisibility( true );
    }
  }


  function OnCollisionTest( i_ciObject )
  {
    if( ( i_ciObject instanceof NPC ) && i_ciObject.IsPathing() )
    {
      if( i_ciObject.GetProperty( Creature_Humanoid_PropertyID, false ) )
      {
        // Not considered a collision since the NPC is capable of opening the door
        return false;
      }
    }

    return true;
  }


  function Open( i_ciCreature )
  {
    // Hide the door
    SetVisibility( false );

    if( i_ciCreature instanceof Player )
    {
      i_ciCreature.ActionSuccess( msg_opened_client_StringID );
    }

    ::rumSchedule( this, Close, s_fCloseTime );
  }
}


class U4_Door_Locked_Widget extends U4_Door_Widget
{
  function Lock()
  {
    local bLocked = GetProperty( Locked_PropertyID, false );
    if( !bLocked )
    {
      // Lock the door
      SetProperty( Locked_PropertyID, true );
    }
  }


  function Open( i_ciCreature )
  {
    local bLocked = GetProperty( Locked_PropertyID, true );
    if( bLocked )
    {
      if( i_ciCreature instanceof Player )
      {
        i_ciCreature.ActionFailed( msg_locked_client_StringID );
      }
    }
    else
    {
      base.Open( i_ciCreature );
    }
  }


  function Unlock( i_ciPlayer )
  {
    local iKeys = i_ciPlayer.GetProperty( U4_Keys_PropertyID, 0 );
    if( iKeys > 0 )
    {
      local bLocked = GetProperty( Locked_PropertyID, true );
      if( bLocked )
      {
        SetProperty( Locked_PropertyID, false );
        ::rumSchedule( this, Lock, s_fLockTime );

        // Unlock successful, inform client
        i_ciPlayer.ActionSuccess( msg_unlocked_client_StringID );

        // Deduct a key
        i_ciPlayer.SetProperty( U4_Keys_PropertyID, iKeys - 1 );
      }
      else
      {
        // The door is already unlocked
        i_ciPlayer.ActionFailed( msg_not_here_client_StringID );
      }
    }
    else
    {
      i_ciPlayer.ActionFailed( msg_no_keys_client_StringID );
    }
  }
}
