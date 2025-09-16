class U1_Sun_Widget extends U1_Widget
{
  function AffectCreature( i_ciCreature )
  {
    // Kill the creature
    i_ciCreature.Kill( this, null, true );
  }


  function IsHarmful( i_ciCreature )
  {
    return true;
  }
}
