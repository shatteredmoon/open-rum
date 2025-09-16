class Inventory extends rumInventory
{
  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    if( i_ePropertyID == Inventory_Quantity_PropertyID )
    {
      Ultima_Stat_Update();
    }
  }
}
