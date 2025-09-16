function CastOpen( i_ciCaster, i_eChestType )
{
  local ciChest;
  local ciMap = i_ciCaster.GetMap();
  local bFound = false;
  local ciPosData = ciMap.GetPositionData( i_ciCaster.GetPosition() );
  while( ciChest = ciPosData.GetNext( rumWidgetPawnType, i_eChestType ) )
  {
    if( ciChest.IsVisible() )
    {
      // Open the chest using magic
      bFound = true;
      ciChest.Open( i_ciCaster, true );
      ciPosData.Stop();
    }
  }

  if( !bFound )
  {
    i_ciCaster.ActionFailed( msg_not_here_client_StringID );
  }
}
