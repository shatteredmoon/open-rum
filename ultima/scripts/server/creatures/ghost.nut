class U4_Ghost_Creature extends U4_NPC
{
  function OnAddedToMap()
  {
    base.OnAddedToMap();

    local ciMap = GetMap();
    local eMapID = ciMap.GetAssetID();
    if( U4_Towne_Skara_Brae_MapID == eMapID )
    {
      // Isaac is invisible until a player stays at the inn
      SetVisibility( false );
    }
  }


  function Hide()
  {
    SetVisibility( false );
  }
}
