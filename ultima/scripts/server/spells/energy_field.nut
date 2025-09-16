function CastEnergyField( i_ciCaster, i_ciPos, i_eDir, i_eFieldType, i_fDuration, i_bPrimary )
{
  local ciField = ::rumCreate( i_eFieldType );
  if( null == ciField )
  {
    return;
  }

  local bPlaced = false;

  local ciMap = i_ciCaster.GetMap();
  local ciTargetPos = i_ciPos + GetDirectionVector( i_eDir );
  if( ciMap.MovePawn( ciField, ciTargetPos, rumIgnoreDistanceMoveFlag | rumTestMoveFlag ) == rumSuccessMoveResultType )
  {
    if( ciMap.AddPawn( ciField, ciTargetPos ) )
    {
      ::rumSchedule( ciField, ciField.Expire, i_fDuration );
      bPlaced = true;
    }
  }

  if( i_bPrimary )
  {
    // Create secondary fields
    local eDir = i_eDir - 1;
    if( eDir < Direction.Start )
    {
      eDir = Direction.End - 1;
    }

    CastEnergyField( i_ciCaster, i_ciPos, eDir, i_eFieldType, i_fDuration, false /* secondary */ );

    eDir = i_eDir + 1;
    if( eDir >= Direction.End )
    {
      eDir = Direction.Start;
    }

    CastEnergyField( i_ciCaster, i_ciPos, eDir, i_eFieldType, i_fDuration, false /* secondary */ );

    if( !bPlaced )
    {
      i_ciCaster.ActionFailed( msg_not_here_client_StringID );
    }
  }
}
