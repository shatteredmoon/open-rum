class Inventory extends rumInventory
{
    // sent to all players
    static global = false;

    // stored to database on change
    static persistent = true;

    // sent only to the object owning the property
    static private = true;

    // sent to any players on the current map only
    static regional = false;
}
