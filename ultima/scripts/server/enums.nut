enum AttackReturnType
{
  Success,
  Failed_InvalidTarget,
  Failed_OutOfRange,
  Failed_LOS,
  Failed_Collision
}

enum ChestType
{
  // Warning: These are serialized to maps!
  Treasure = 0,
  Guarded  = 1,
  Player   = 2,
  NPC      = 3
}

enum FountainType
{
  // Warning: These are serialized to maps!
  Health = 0,
  Poison = 1,
  Cure   = 2,
  Mana   = 3,
  Acid   = 4
}

enum MerchantType
{
  // Warning: These are serialized to maps!
  Weapons   = 0,
  Armour    = 1,
  Inn       = 2,
  Healer    = 3,
  Rations   = 4,
  Guild     = 5,
  Tavern    = 6,
  Transport = 7,
  Magic     = 8,
  Stable    = 9,
  Reagents  = 10,
  Oracle    = 11
}

enum NPCSpecialType
{
  // Warning: These are serialized to maps!
  U4_LordBritish  = 0,
  U4_SeerHawkwind = 1,
  U4_ShrineAnkh   = 2,
  U3_LordBritish  = 3,
  U2_LordBritish  = 4,
  U1_King         = 5,
  U1_Princess     = 6,
  U3_Timelord     = 7,
  U2_HotelClerk   = 8,
  U2_Oracle       = 9,
  U2_OldMan       = 10,
  U2_Sentri       = 11
}

enum NPCType
{
  // Warning: These are serialized to maps!
  Standard,
  Merchant,
  Special
}

enum PropertyUserFlags
{
  None         = 0x0,
  Transferable = 0x1
}

enum TrapType
{
  // Warning: These are serialized to maps!
  Rocks,
  Winds,
  Pit
}

enum UltimaCompletions
{
  // Warning: These are serialized on the player!
  KilledMondain   = 0,
  KilledMinax     = 1,
  DestroyedExodus = 2,
  CodexTest       = 3,
  NumFlags        = 4
}
