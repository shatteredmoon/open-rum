class Portal extends rumPortal
{
  destMap = 0;
  destPos = null;

  constructor()
  {
    base.constructor();
    destPos = rumPos();
  }

  function isCollision( eMoveType )
  {
    return false;
  }

  function onSerialize( ciArchiver )
  {
    local iVersion = 1;
    iVersion = ciArchiver.serialize( iVersion );
    destMap = ciArchiver.serialize( destMap );
    destPos.x = ciArchiver.serialize( destPos.x );
    destPos.y = ciArchiver.serialize( destPos.y );
  }

  function onEdit()
  {
    local classTable = GetRegisteredInfo( "Map" );

    local arr = ["Destination Map", destMap, classTable,
                 "Destination Position X", destPos.x, null,
                 "Destination Position Y", destPos.y, null];

    arr = rumModalDialog( arr );
    if( arr != null && ( ( typeof arr ) == "array" ) && ( arr.len() == 3 ) )
    {
      destMap = arr[0];
      destPos.x = arr[1];
      destPos.y = arr[2];
    }
  }
}
