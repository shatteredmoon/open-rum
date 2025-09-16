g_eCharismaPropertyVersionArray <-
[
  null,
  U1_Charisma_PropertyID,
  U2_Charisma_PropertyID,
  null,
  null
]

g_eCriminalPropertyVersionArray <-
[
  null,
  U1_Criminal_PropertyID,
  U2_Criminal_PropertyID,
  U3_Criminal_PropertyID,
  U4_Criminal_PropertyID,
]

g_eDexterityPropertyVersionArray <-
[
  null,
  U1_Agility_PropertyID,
  U2_Agility_PropertyID,
  U3_Dexterity_PropertyID,
  U4_Dexterity_PropertyID
]

g_eDirectionStringArray <-
[
  dir_west_shared_StringID,
  dir_northwest_shared_StringID,
  dir_north_shared_StringID,
  dir_northeast_shared_StringID,
  dir_east_shared_StringID,
  dir_southeast_shared_StringID,
  dir_south_shared_StringID,
  dir_southwest_shared_StringID,
  dir_center_shared_StringID
]

g_eFoodPropertyVersionArray <-
[
  null,
  U1_Food_PropertyID,
  U2_Food_PropertyID,
  U3_Food_PropertyID,
  U4_Food_PropertyID
]

g_eGoldPropertyVersionArray <-
[
  null,
  U1_Coin_PropertyID,
  U2_Gold_PropertyID,
  U3_Gold_PropertyID,
  U4_Gold_PropertyID
]

g_eHitpointsPropertyVersionArray <-
[
  null,
  U1_Hitpoints_PropertyID,
  U2_Hitpoints_PropertyID,
  U3_Hitpoints_PropertyID,
  U4_Hitpoints_PropertyID
]

g_eHorseDismountTimePropertyVersionArray <-
[
  null,
  U1_Horse_Dismount_Time_PropertyID,
  U2_Horse_Dismount_Time_PropertyID,
  U3_Horse_Dismount_Time_PropertyID,
  U4_Horse_Dismount_Time_PropertyID
]

g_eIntelligencePropertyVersionArray <-
[
  null,
  U1_Intelligence_PropertyID,
  U2_Intelligence_PropertyID,
  U3_Intelligence_PropertyID,
  U4_Intelligence_PropertyID
]

g_eKeysPropertyVersionArray <-
[
  null,
  U1_Keys_PropertyID,
  U2_Keys_PropertyID,
  U3_Keys_PropertyID,
  U4_Keys_PropertyID
]

g_eKingCreatureVersionArray <-
[
  null,
  U1_King_CreatureID,
  U2_King_CreatureID,
  U3_King_CreatureID,
  U4_King_CreatureID
]

g_eKingMapVersionArray <-
[
  null,
  U1_Castle_Lord_British_MapID,
  U2_Earth_AD_Castle_Lord_British_MapID,
  U3_Castle_Lord_British_MapID,
  U4_Castle_British_2_MapID
]

g_eManaPropertyVersionArray <-
[
  null,
  U1_Mana_PropertyID,
  U2_Mana_PropertyID,
  U3_Mana_PropertyID,
  U4_Mana_PropertyID
]

g_ePlayerClassPropertyVersionArray <-
[
  null,
  U1_PlayerClass_PropertyID,
  U2_PlayerClass_PropertyID,
  U3_PlayerClass_PropertyID,
  U4_PlayerClass_PropertyID
]

g_ePoisonedPropertyVersionArray <-
[
  null,
  null,
  U2_Poisoned_PropertyID,
  U3_Poisoned_PropertyID,
  U4_Poisoned_PropertyID
]

g_ePotionsPropertyVersionArray <-
[
  null,
  U1_Potions_PropertyID,
  U2_Potions_PropertyID,
  U3_Potions_PropertyID,
  U4_Potions_PropertyID
]

g_eSpiritBoundPropertyVersionArray <-
[
  null,
  U1_Spirit_Bound_PropertyID,
  U2_Spirit_Bound_PropertyID,
  U3_Spirit_Bound_PropertyID,
  U4_Spirit_Bound_PropertyID
]

g_eStaminaPropertyVersionArray <-
[
  null,
  U1_Stamina_PropertyID,
  U2_Stamina_PropertyID,
  null,
  null
]

g_eStrengthPropertyVersionArray <-
[
  null,
  U1_Strength_PropertyID,
  U2_Strength_PropertyID,
  U3_Strength_PropertyID,
  U4_Strength_PropertyID
]

g_eTransportIDPropertyVersionArray <-
[
  null,
  U1_Transport_ID_PropertyID,
  U2_Transport_ID_PropertyID,
  U3_Transport_ID_PropertyID,
  U4_Transport_ID_PropertyID
]

g_eTransportMapPropertyVersionArray <-
[
  null,
  U1_Transport_Map_PropertyID,
  U2_Transport_Map_PropertyID,
  U3_Transport_Map_PropertyID,
  U4_Transport_Map_PropertyID
]

g_eTransportWidgetPropertyVersionArray <-
[
  null,
  U1_Transport_Widget_PropertyID,
  U2_Transport_Widget_PropertyID,
  U3_Transport_Widget_PropertyID,
  U4_Transport_Widget_PropertyID
]

g_eWisdomPropertyVersionArray <-
[
  null,
  U1_Wisdom_PropertyID,
  U2_Wisdom_PropertyID,
  U3_Wisdom_PropertyID,
  null
]

// Color tags for string output
g_strColorTagArray <-
{
  Black   = "<C#000000>"
  White   = "<C#FFFFFF>"
  Red     = "<C#FF0000>"
  Green   = "<C#00FF00>"
  Blue    = "<C#4040FF>"
  Yellow  = "<C#FFFF00>"
  Magenta = "<C#FF00FF>"
  Cyan    = "<C#00FFFF>"
  Gray    = "<C#7F7F7F>"
}


function clamp( i_vValue, i_vLowerLimit, i_vUpperLimit )
{
  local vValue = i_vValue;

  if( vValue < i_vLowerLimit )
  {
    vValue = i_vLowerLimit;
  }
  else if( vValue > i_vUpperLimit )
  {
    vValue = i_vUpperLimit;
  }

  return vValue;
}


// Floating point random number (precision for thousandths only)
function frand( i_fVal )
{
  return ( rand() % ( i_fVal * 1000.0 ) ) / 1000.0;
}


// Returns a random number given a range of variance
// Example: 10.0 will return a number between -5.0 and 5.0
function frandVariance( i_fVar )
{
  return frand( i_fVar ) - ( i_fVar / 2.0 );
}


function GetDirectionFromVector( i_ciDir )
{
  local eDir = Direction.None;

  if( i_ciDir && i_ciDir instanceof rumVector )
  {
    if( i_ciDir.x == 0 )
    {
      if( i_ciDir.y == 1 )
      {
        eDir = Direction.South;
      }
      else if( i_ciDir.y == -1 )
      {
        eDir = Direction.North;
      }
    }
    else if( i_ciDir.x == 1 )
    {
      if( i_ciDir.y == 0 )
      {
        eDir = Direction.East;
      }
      else if( i_ciDir.y == 1 )
      {
        eDir = Direction.Southeast;
      }
      else if( i_ciDir.y == -1 )
      {
        eDir = Direction.Northeast;
      }
    }
    else if( i_ciDir.x == -1 )
    {
      if( i_ciDir.y == 0 )
      {
        eDir = Direction.West;
      }
      else if( i_ciDir.y == 1 )
      {
        eDir = Direction.Southwest;
      }
      else if( i_ciDir.y == -1 )
      {
        eDir = Direction.Northwest;
      }
    }
  }

  return eDir;
}


function GetDirectionVector( i_eDir )
{
  switch( i_eDir )
  {
    case Direction.West:      return rumVector( -1,  0 ); break;
    case Direction.Northwest: return rumVector( -1, -1 ); break;
    case Direction.North:     return rumVector(  0, -1 ); break;
    case Direction.Northeast: return rumVector(  1, -1 ); break;
    case Direction.East:      return rumVector(  1,  0 ); break;
    case Direction.Southeast: return rumVector(  1,  1 ); break;
    case Direction.South:     return rumVector(  0,  1 ); break;
    case Direction.Southwest: return rumVector( -1,  1 ); break;
    default:                  break;
  }

  return rumVector( 0, 0 );
}


function GetRandomDirection()
{
  return ( rand() % Direction.None );
}


function GetRandomDirectionVector()
{
  return GetDirectionVector( GetRandomDirection() );
}


function GetEnumNameFromValue( i_strEnum, i_eValue )
{
  local eConstTable = getconsttable()[i_strEnum];
  foreach( strKey, eVal in eConstTable )
  {
    if( eVal == i_eValue )
    {
      return strKey;
    }
  }

  return "";
}


function GetEnumValueFromName( i_strEnum, i_strName )
{
  local eConstTable = getconsttable()[i_strEnum];
  foreach( strKey, eVal in eConstTable )
  {
    if( i_strName == strKey )
    {
      return eVal;
    }
  }

  return "";
}


function IsSignTile( i_ciTile )
{
  return i_ciTile.GetProperty( Tile_Sign_PropertyID, false );
}


function IsSpaceTile( i_ciTile )
{
  return i_ciTile.GetProperty( Tile_Space_PropertyID, false );
}


function IsWaterTile( i_ciTile )
{
  return i_ciTile.GetProperty( Tile_Water_PropertyID, false );
}


function MapRequiresLight( i_ciMap )
{
  if( null == i_ciMap )
  {
    return false;
  }

  local eMapType = i_ciMap.GetProperty( Map_Type_PropertyID, MapType.Invalid );
  return ( MapType.Dungeon == eMapType ) ||
         ( MapType.Abyss == eMapType )   ||
         ( MapType.Altar == eMapType )   ||
         ( MapType.Cave == eMapType )    ||
         ( MapType.Codex == eMapType );
}


function max( i_vValue1, i_vValue2 )
{
  return i_vValue1 > i_vValue2 ? i_vValue1 : i_vValue2;
}


function min( i_vValue1, i_vValue2 )
{
  return i_vValue1 < i_vValue2 ? i_vValue1 : i_vValue2;
}


function OnPropertyDesc( i_ciProperty, i_sqValue )
{
  local strDesc = "";

  if( i_ciProperty.IsAssetRef() )
  {
    local ciAsset = ::rumGetAsset( i_sqValue );
    if( ciAsset != null )
    {
      strDesc = ciAsset.GetName();
    }
  }
  else
  {
    local strEnumName = i_ciProperty.GetEnumName();
    if( strEnumName != "" )
    {
      strDesc = GetEnumNameFromValue( strEnumName, i_sqValue );
    }
  }

  return strDesc;
}


function PopCount( i_iBitFlags )
{
  local iCount = 0;

  // Until all bits are zero
  while( i_iBitFlags > 0 )
  {
    // Check lower bit
    if( ( i_iBitFlags & 1 ) == 1 )
    {
      ++iCount;
    }

    // Shift bits, removing lower bit
    i_iBitFlags = ( i_iBitFlags >> 1 );
  }

  return iCount;
}


function PrintContainer( i_ciContainer )
{
  print( "Contents of container: " + i_ciContainer + " (" + i_ciContainer.len() + " elements)\n" );
  foreach( iIndex, i_vValue in i_ciContainer )
  {
    print( "Index " + iIndex + ", value " + i_vValue + "\n" );
  }
}


function ShuffleArray( i_ciArray )
{
  local iLength = i_ciArray.len();
  for( local iLeftIndex = 0; iLeftIndex < iLength; ++iLeftIndex )
  {
    local iRightIndex = iLength - iLeftIndex;
    local iRandomIndex = rand()%iRightIndex;
    local vTemp = i_ciArray[iRandomIndex];
    i_ciArray[iRandomIndex] = i_ciArray[--iRightIndex];
    i_ciArray[iRightIndex] = vTemp;
  }
}


// This will strip any embedded tags from strings and return the stripped portion. Embedded tags typically start with
// "<" and end with ">", though those token can be provided
function StripTags( i_strInput, i_strStartToken = "<", i_strEndToken = ">" )
{
  local iIndex = 0;

  iIndex = i_strInput.find( i_strStartToken );
  if( iIndex != null )
  {
    local iOffset = 0;
    local strNew = "";

    // Loop while a function tag is found
    do
    {
      // Find the next end of the function tag
      strNew += i_strInput.slice( iOffset, iIndex );
      iOffset = i_strInput.find( i_strEndToken, iIndex ) + 1;
    } while( iIndex = i_strInput.find( i_strStartToken, iOffset ) );

    // Append the leftover portion of the string
    strNew += i_strInput.slice( iOffset );

    return strNew;
  }

  return i_strInput;
}


// This replaces squirrel keyword 'in', since it doesn't seem to work for all containers
// Returns true if 'value' exists in 'container'
function ValueInContainer( i_vValue, i_ciContainer )
{
  foreach( vValue in i_ciContainer )
  {
    if( vValue == i_vValue )
    {
      return true;
    }
  }

  return false;
}
