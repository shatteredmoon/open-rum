class U2_Thief_Creature extends U2_NPC
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

        switch( rand() % 7 )
        {
          case 0: ePropertyID = U2_Boots_PropertyID; break;
          case 1: ePropertyID = U2_Cloaks_PropertyID; break;
          case 2: ePropertyID = U2_Tools_PropertyID; break;
          case 3: ePropertyID = U2_Ankhs_PropertyID; break;
          case 4: ePropertyID = U2_Blue_Tassles_PropertyID; break;
          case 5: ePropertyID = U2_Brass_Buttons_PropertyID; break;
          case 6: ePropertyID = U2_Skull_Keys_PropertyID; break;
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
