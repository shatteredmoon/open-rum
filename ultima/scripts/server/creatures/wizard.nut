class U2_Wizard_Creature extends U2_NPC
{
  function OnDeath( i_ciSource, i_bKillCredit = true )
  {
    base.OnDeath( i_ciSource, i_bKillCredit );

    // There is a small chance of giving items to players
    if( i_bKillCredit && ( i_ciSource instanceof Player ) )
    {
      if( rand() % 100 < 5 )
      {
        local ePropertyID = rumInvalidAssetID;

        switch( rand() % 3 )
        {
          case 0: ePropertyID = U2_Wands_PropertyID; break;
          case 1: ePropertyID = U2_Staffs_PropertyID; break;
          case 2: ePropertyID = U2_Green_Idols_PropertyID; break;
          case 3: ePropertyID = U2_Strange_Coin_PropertyID; break;
        }

        if( ePropertyID != rumInvalidAssetID )
        {
          local iNewAmount = i_ciSource.GetProperty( ePropertyID, 0 ) + 1;
          i_ciSource.SetProperty( ePropertyID, iNewAmount );
        }
      }
    }
  }
}
