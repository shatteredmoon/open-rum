function CastDispell( i_ciCaster, i_eDir, i_cTarget )
{
  local bFound = false;

  local ciPos = i_ciCaster.GetPosition() + GetDirectionVector( i_eDir );
  local ciMap = i_ciCaster.GetMap();

  local eResult = ciMap.MovePawn( i_ciCaster, ciPos, rumTestMoveFlag );
  if( eResult != rumOffMapMoveResultType )
  {
    local bDispelled = false;

    local ciField;
    local ciPosData = ciMap.GetPositionData( ciPos );
    while( ciField = ciPosData.GetNext( rumWidgetPawnType ) )
    {
      if( ciField.IsVisible() && ( ciField instanceof i_cTarget ) )
      {
        bFound = true;
        bDispelled = ciField.Dispell();
        if( !bDispelled )
        {
          i_ciCaster.ActionFailed( msg_failed_client_StringID );
        }
      }
    }
  }

  if( !bFound )
  {
    i_ciCaster.ActionFailed( msg_not_here_client_StringID );
  }
}
