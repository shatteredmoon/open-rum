class U2_Fighter_Creature extends U2_NPC
{
  function OnDeath( i_ciSource, i_bKillCredit = true )
  {
    base.OnDeath( i_ciSource, i_bKillCredit );

    // There is a small chance of giving items to players
    if( i_bKillCredit && ( i_ciSource instanceof Player ) )
    {
      if( rand() % 100 < 10 )
      {
        local ePropertyID = runInvalidAssetID;

        switch( rand() % 4 )
        {
          case 0: ePropertyID = U2_Helms_PropertyID; break;
          case 1: // fall through
          case 2: // fall through
          case 3: ePropertyID = U2_Torches_PropertyID; break;
        }

        if( ePropertyID != runInvalidAssetID )
        {
          local iNewAmount = i_ciSource.GetProperty( ePropertyID, 0 ) + 1;
          i_ciSource.SetProperty( ePropertyID, iNewAmount );
        }
      }
    }
  }
}
