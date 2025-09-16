class U4_Horse_Widget extends Transport_Widget
{
  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Terrestrial );

    SetProperty( State_PropertyID, HorseStateType.FacingLeft );
  }


  function Board( i_ciPlayer, i_bForce = false )
  {
    // TODO - handle horse ownership/theft of a player's horse

    local uiCommanderID = GetCommanderID();
    if( rumInvalidGameID == uiCommanderID )
    {
      if( !i_bForce )
      {
        i_ciPlayer.ActionSuccess( msg_mount_horse_client_StringID );
      }

      return base.Board( i_ciPlayer, i_bForce );
    }

    i_ciPlayer.ActionFailed( msg_no_room_client_StringID );

    return false;
  }


  function Exit( i_ciPlayer, i_ciPos, i_eMoveType )
  {
    if( base.Exit( i_ciPlayer, i_ciPos, i_eMoveType ) )
    {
      // Stamp the exit time on the player
      i_ciPlayer.SetVersionedProperty( g_eHorseDismountTimePropertyVersionArray, ::rumGetSecondsSinceEpoch() );

      return true;
    }

    return false;
  }


  function TryMove( i_ciPlayer, i_ciPos, i_eDir )
  {
    local eResult = base.TryMove( i_ciPlayer, i_ciPos, i_eDir );
    if( rumSuccessMoveResultType == eResult )
    {
      local eState = GetProperty( State_PropertyID, HorseStateType.FacingLeft );

      // State_PropertyID drives client animation
      if( eState != HorseStateType.FacingRight &&
          ( i_eDir >= Direction.Northeast && i_eDir <= Direction.Southeast ) )
      {
        // Replicates to clients
        SetProperty( State_PropertyID, HorseStateType.FacingRight );
      }
      else if( eState != HorseStateType.FacingLeft &&
               ( ( Direction.West == i_eDir )      ||
                 ( Direction.Northwest == i_eDir ) ||
                 ( Direction.Southwest == i_eDir ) ) )
      {
        // Replicates to clients
        SetProperty( State_PropertyID, HorseStateType.FacingLeft );
      }
    }

    return eResult;
  }
}


class U3_Horse_Widget extends Transport_Widget
{
  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Terrestrial );

    SetProperty( State_PropertyID, HorseStateType.FacingLeft );
  }


  function Board( i_ciPlayer, i_bForce = false )
  {
    // TODO - handle horse ownership/theft of a player's horse

    local uiCommanderID = GetCommanderID();
    if( rumInvalidGameID == uiCommanderID )
    {
      if( !i_bForce )
      {
        i_ciPlayer.ActionSuccess( msg_mount_horse_client_StringID );
      }

      return base.Board( i_ciPlayer, i_bForce );
    }

    i_ciPlayer.ActionFailed( msg_no_room_client_StringID );

    return false;
  }


  function Exit( i_ciPlayer, i_ciPos, i_eMoveType )
  {
    if( base.Exit( i_ciPlayer, i_ciPos, i_eMoveType ) )
    {
      // Stamp the exit time on the player
      i_ciPlayer.SetVersionedProperty( g_eHorseDismountTimePropertyVersionArray, ::rumGetSecondsSinceEpoch() );

      return true;
    }

    return false;
  }


  function TryMove( i_ciPlayer, i_ciPos, i_eDir )
  {
    local eResult = base.TryMove( i_ciPlayer, i_ciPos, i_eDir );
    if( rumSuccessMoveResultType == eResult )
    {
      local eState = GetProperty( State_PropertyID, HorseStateType.FacingLeft );

      // State_PropertyID drives client animation
      if( eState != HorseStateType.FacingRight &&
          ( i_eDir >= Direction.Northeast && i_eDir <= Direction.Southeast ) )
      {
        // Replicates to clients
        SetProperty( State_PropertyID, HorseStateType.FacingRight );
      }
      else if( eState != HorseStateType.FacingLeft &&
               ( ( Direction.West == i_eDir )      ||
                 ( Direction.Northwest == i_eDir ) ||
                 ( Direction.Southwest == i_eDir ) ) )
      {
        // Replicates to clients
        SetProperty( State_PropertyID, HorseStateType.FacingLeft );
      }
    }

    return eResult;
  }
}


class U2_Horse_Widget extends Transport_Widget
{
  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Terrestrial );

    SetProperty( State_PropertyID, HorseStateType.FacingLeft );
  }


  function Board( i_ciPlayer, i_bForce = false )
  {
    // TODO - handle horse ownership/theft of a player's horse

    local uiCommanderID = GetCommanderID();
    if( rumInvalidGameID == uiCommanderID )
    {
      if( !i_bForce )
      {
        i_ciPlayer.ActionSuccess( msg_mount_horse_client_StringID );
      }

      return base.Board( i_ciPlayer, i_bForce );
    }

    i_ciPlayer.ActionFailed( msg_no_room_client_StringID );

    return false;
  }


  function Exit( i_ciPlayer, i_ciPos, i_eMoveType )
  {
    if( base.Exit( i_ciPlayer, i_ciPos, i_eMoveType ) )
    {
      // Stamp the exit time on the player
      i_ciPlayer.SetVersionedProperty( g_eHorseDismountTimePropertyVersionArray, ::rumGetSecondsSinceEpoch() );

      return true;
    }

    return false;
  }


  function TryMove( i_ciPlayer, i_ciPos, i_eDir )
  {
    local eResult = base.TryMove( i_ciPlayer, i_ciPos, i_eDir );
    if( rumSuccessMoveResultType == eResult )
    {
      local eState = GetProperty( State_PropertyID, HorseStateType.FacingLeft );

      // State_PropertyID drives client animation
      if( eState != HorseStateType.FacingRight &&
          ( i_eDir >= Direction.Northeast && i_eDir <= Direction.Southeast ) )
      {
        // Replicates to clients
        SetProperty( State_PropertyID, HorseStateType.FacingRight );
      }
      else if( eState != HorseStateType.FacingLeft &&
               ( ( Direction.West == i_eDir )      ||
                 ( Direction.Northwest == i_eDir ) ||
                 ( Direction.Southwest == i_eDir ) ) )
      {
        // Replicates to clients
        SetProperty( State_PropertyID, HorseStateType.FacingLeft );
      }
    }

    return eResult;
  }
}


class U1_Horse_Widget extends Transport_Widget
{
  constructor()
  {
    base.constructor();
    SetMoveType( MoveType.Terrestrial );

    SetProperty( State_PropertyID, HorseStateType.FacingLeft );
  }


  function Board( i_ciPlayer, i_bForce = false )
  {
    // TODO - handle horse ownership/theft of a player's horse

    local uiCommanderID = GetCommanderID();
    if( rumInvalidGameID == uiCommanderID )
    {
      if( !i_bForce )
      {
        i_ciPlayer.ActionSuccess( msg_mount_horse_client_StringID );
      }

      return base.Board( i_ciPlayer, i_bForce );
    }

    i_ciPlayer.ActionFailed( msg_no_room_client_StringID );

    return false;
  }


  function Exit( i_ciPlayer, i_ciPos, i_eMoveType )
  {
    if( base.Exit( i_ciPlayer, i_ciPos, i_eMoveType ) )
    {
      // Stamp the exit time on the player
      i_ciPlayer.SetVersionedProperty( g_eHorseDismountTimePropertyVersionArray, ::rumGetSecondsSinceEpoch() );

      return true;
    }

    return false;
  }


  function TryMove( i_ciPlayer, i_ciPos, i_eDir )
  {
    local eResult = base.TryMove( i_ciPlayer, i_ciPos, i_eDir );
    if( rumSuccessMoveResultType == eResult )
    {
      local eState = GetProperty( State_PropertyID, HorseStateType.FacingLeft );

      // State_PropertyID drives client animation
      if( eState != HorseStateType.FacingRight &&
          ( i_eDir >= Direction.Northeast && i_eDir <= Direction.Southeast ) )
      {
        // Replicates to clients
        SetProperty( State_PropertyID, HorseStateType.FacingRight );
      }
      else if( eState != HorseStateType.FacingLeft &&
               ( ( Direction.West == i_eDir )      ||
                 ( Direction.Northwest == i_eDir ) ||
                 ( Direction.Southwest == i_eDir ) ) )
      {
        // Replicates to clients
        SetProperty( State_PropertyID, HorseStateType.FacingLeft );
      }
    }

    return eResult;
  }
}
