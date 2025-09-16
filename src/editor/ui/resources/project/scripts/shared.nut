
DIRECTION <-
{
   NONE = 0,
   WEST = 1,
   NORTHWEST = 2,
   NORTH = 3,
   NORTHEAST = 4,
   EAST = 5,
   SOUTHEAST = 6,
   SOUTH = 7,
   SOUTHWEST = 8,
   NUM = 9
}

direction <-
[
    "dir_center",
    "dir_west",
    "dir_northwest",
    "dir_north",
    "dir_northeast",
    "dir_east",
    "dir_southeast",
    "dir_south",
    "dir_southwest"
]


// Returns a table of name/id pairs of registered classes
function GetRegisteredInfo(className)
{
    // The table built and returned by this function
    local classTable = {};

    local root = getroottable();
    foreach(idx, val in root)
    {
        // First off, does is this item a script class
        if ((typeof val) == "class")
        {
            local baseClass = val.getbase();
            try
            {
                // Does a base class exist and if so, does it match the requested class name?
                while ((typeof baseClass) == "class" && baseClass.__name != className)
                {
                    // Keep recursing base classes until the base is no longer a script class or the base class name
                    // matches the requested class name
                    baseClass = baseClass.getbase();
                }

                if (baseClass.__name == className)
                {
                    // This class is derived from a class matching the requested name, store its information
                    classTable.rawset(val.__name, val.__sid);
                }
            }
            catch(e)
            {
                // Doesn't have an embedded name/id
            }
        }
    }

    return classTable;
}


// This replaces squirrel keyword 'in', since it doesn't seem to work for all
// containers. Returns true if 'value' exists in 'container'
function valueInContainer(value, container)
{
    foreach (item in container)
    {
        if (item == value)
        {
            //print("Value found in container\n");
            return true;
        }
    }

    //print("Value not found in container\n");
    return false;
}


function printContainer(container)
{
    print("Contents of container: " + container + " (" + container.len() + " elements)\n");
    foreach (item in container)
    {
        print(item.__name + " " + item + "\n");
    }
}


function printSimpleContainer(container)
{
    print("Contents of container: " + container + " (" + container.len() + " elements)\n");
    foreach (index, item in container)
    {
        print("Index " + index + ", value " + item + "\n");
    }
}


function max(val1, val2)
{
    if (val1 > val2)
    {
        return val1;
    }

    return val2;
}


function min(val1, val2)
{
    if (val1 < val2)
    {
        return val1;
    }

    return val2;
}


function GetDirectionVector(dir)
{
    switch (dir)
    {
        case DIRECTION.WEST:      return rumVector(-1, 0);  break;
        case DIRECTION.NORTHWEST: return rumVector(-1, -1); break;
        case DIRECTION.NORTH:     return rumVector(0, -1);  break;
        case DIRECTION.NORTHEAST: return rumVector(1, -1);  break;
        case DIRECTION.EAST:      return rumVector(1, 0);   break;
        case DIRECTION.SOUTHEAST: return rumVector(1, 1);   break;
        case DIRECTION.SOUTH:     return rumVector(0, 1);   break;
        case DIRECTION.SOUTHWEST: return rumVector(-1, 1);  break;
        default:                  break;
    }

    return rumVector(0, 0);
}


function GetDirectionFromInput(key)
{
    local dir = DIRECTION.NONE;

    switch(key)
    {
    case rumKEY_LEFT:
    case rumKEY_4_PAD:
        dir = DIRECTION.WEST; break;
    case rumKEY_RIGHT:
    case rumKEY_6_PAD:
        dir = DIRECTION.EAST; break;
    case rumKEY_UP:
    case rumKEY_8_PAD:
        dir = DIRECTION.NORTH; break;
    case rumKEY_DOWN:
    case rumKEY_2_PAD:
        dir = DIRECTION.SOUTH; break;
    case rumKEY_7_PAD:
        dir = DIRECTION.NORTHWEST; break;
    case rumKEY_9_PAD:
        dir = DIRECTION.NORTHEAST; break;
    case rumKEY_3_PAD:
        dir = DIRECTION.SOUTHEAST; break;
    case rumKEY_1_PAD:
        dir = DIRECTION.SOUTHWEST; break;
    }

    return dir;
}


function GetRandomDirection()
{
    return (rand()%DIRECTION.SOUTHWEST + 1);
}


function GetRandomDirectionVector()
{
    return GetDirectionVector(GetRandomDirection());
}
