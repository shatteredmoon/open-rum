class Block_NPC_Widget extends Widget
{
  function OnCollisionTest( i_ciObject )
  {
    return i_ciObject instanceof NPC;
  }
}
