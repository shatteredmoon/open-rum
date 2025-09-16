// Warning: These are serialized to the player
enum QuickswordMaterialFlags
{
  Blade  = 0x1,
  Pommel = 0x2,
  Hilt   = 0x4,
  All    = 0x7
}

// Warning: These are serialized to the player
enum QuickswordQuestState
{
  NotStarted = 0,
  Started    = 1,
  Finished   = 2
}

// Warning: These are serialized to the player
enum RingQuestState
{
  Started               = 0
  ReceivedAntosBlessing = 1,
  ReceivedDirections    = 2
}

enum U2_SignpostType
{
  Pangea            = 0,
  EarthBC           = 1,
  EarthAD           = 2,
  EarthAftermath    = 3,
  Legends           = 4,
  Uranus            = 5
}
