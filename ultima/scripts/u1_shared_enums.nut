enum U1_CoinType
{
  Copper,
  Silver,
  Gold
}

// Warning: These are serialized to players in the database! Do not change their order!
enum U1_GemType
{
  Red     = 0,
  Green   = 1,
  Blue    = 2,
  White   = 3,
  NumGems = 4
}

enum U1_KingQuestRewardType
{
  Stat,
  Hint,
  Gem
}

// Warning: These are serialized to players in the database! Do not change their order!
enum U1_KingQuestState
{
  Unbestowed1 = 0,
  Bestowed1   = 1,
  Completed1  = 2,
  Unbestowed2 = 3,
  Bestowed2   = 4,
  Completed2  = 5
}

// Warning: These are serialized to players in the database! Do not change their order!
enum U1_KingQuestType
{
  LostKing    = 0, // Gelatinous Cube / Daemon
  Rondorin    = 1, // CarrionCreeper / Invisible Seeker
  BlackDragon = 2, // Lich / Zorn
  Shamino     = 3, // Balron / Mind Whipper
  LordBritish = 4, // TowerKnowledge / Eastern Sign Post
  Barataria   = 5, // PillarOzymandias / The Sign Post
  WhiteDragon = 6, // GraveLostSoul / Pillar of the Argonauts
  Olympus     = 7  // SouthernSignPost / Pillars of Protection
}

enum U1_KingTalkType
{
  Greet,
  Pence,
  Service,
  Return,
  HPGranted,
  QuestBestowed,
  QuestAlreadyBestowed,
  QuestReward,
  Bye
}

enum U1_PondResultType
{
  Hitpoints,
  Food,
  Spell,
  Weapon,
  Stat
}

enum U1_PrincessTalkType
{
  Greet,
  TimeMachine
}

enum U1_PrincessType
{
  Marsha    = 0,
  Beth      = 1,
  Julia     = 2,
  Cassandra = 3,
  Kristen   = 4,
  Donna     = 5,
  Dianne    = 6,
  Lori      = 7
}

enum U1_SignPostUsageType
{
  Description,
  QuestComplete,
  RewardProperty,
  RewardWeapon
}
