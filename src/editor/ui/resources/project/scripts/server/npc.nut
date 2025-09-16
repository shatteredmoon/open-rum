class NPC extends Creature
{
  function onSerialize(ciArchiver)
  {
    local iVersion = 1;
    iVersion = ciArchiver.serialize(iVersion);
  }
}
