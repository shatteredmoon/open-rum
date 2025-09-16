class U4_Ghost_Creature extends U4_NPC
{
  constructor()
  {
    base.constructor();

    // Ghosts are transparent
    SetTransparencyLevelFromFloat( 0.66 );
  }
}
