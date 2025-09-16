class U2_Cleric_Creature extends U2_NPC
{
  function OnDeath( i_ciSource, i_bKillCredit = true )
  {
    base.OnDeath( i_ciSource, i_bKillCredit );

    // There is a small chance of giving items to players
    if( i_bKillCredit && ( i_ciSource instanceof Player ) )
    {
      if( rand() % 100 < 5 )
      {
        local iNewAmount = i_ciSource.GetProperty( U2_Ankhs_PropertyID, 0 ) + 1;
        i_ciSource.SetProperty( U2_Ankhs_PropertyID, iNewAmount );
      }
    }
  }
}
