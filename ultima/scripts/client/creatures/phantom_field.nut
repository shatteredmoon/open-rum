class U4_Phantom_Field_Creature extends U4_NPC
{
  constructor()
  {
    base.constructor();
    ::rumSchedule( this, Animate, rand() % 3 + 0.5 );
  }


  function Animate()
  {
    switch( rand() % 4 )
    {
      case 0: SetGraphic( U4_Field_Poison_GraphicID ); break;
      case 1: SetGraphic( U4_Field_Lightning_GraphicID ); break;
      case 2: SetGraphic( U4_Field_Fire_GraphicID ); break;
      case 3: SetGraphic( U4_Field_Sleep_GraphicID ); break;
    }

    ::rumSchedule( this, Animate, rand() % 3 + 0.5 );
  }
}
