class Property extends rumProperty
{
    // sent to all players
    static global = false;

    // stored to database on change
    static persistent = false;

    // sent only to the object owning the property
    static private = false;

    // sent to any players on the current map only
    static regional = false;
}
