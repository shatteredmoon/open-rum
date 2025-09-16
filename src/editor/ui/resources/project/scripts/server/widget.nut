class Widget extends rumWidget
{
  function isCollision( eMoveType )
  {
    return true;
  }

  function onSerialize( ciArchiver )
  {
    local iVersion = 1;
    iVersion = ciArchiver.serialize( iVersion );
  }
}


class Widget_Rock extends Widget
{}


class Widget_Bush extends Widget
{
  function isCollision( eMoveType )
  {
    return false;
  }
}
