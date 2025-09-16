enum U2_ClerkTalkType
{
  Greet,
  RequestRoom,
  RequestRoomTip,
  Bye
}

enum U2_LBTalkType
{
  Greet,
  HitpointTribute,
  StatTribute,
  Return,
  Bye,
  Endgame
}

enum U2_OracleTalkType
{
  Greet,
  Pay,
  Clue,
  Bye
}

// Warning: These are serialized to the player and maps
enum QuickswordMaterialIndex
{
  Blade  = 0,
  Pommel = 1,
  Hilt   = 2
}

enum U2_TimegatePeriods
{
  // Warning: These are serialized to maps!
  Pangea     = 0,
  BC1423     = 1,
  AD1990     = 2,
  AD2112     = 3,
  Legends    = 4,
  NumPeriods = 5
}
