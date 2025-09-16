enum U3_CardType
{
  // Warning: These are serialized to players in the database! Do not change their order!
  Death    = 0,
  Love     = 1,
  Sol      = 2,
  Moons    = 3,
  NumCards = 4
}

enum U3_ExodusComponentState
{
  Functional,
  Destroyed
}

enum U3_LBTalkType
{
  Greet,
  Report,
  NeedExp,
  LevelUp,
  LevelCap,
  SeekMark,
  Return,
  Bye,
  Endgame
}

enum U3_MarkType
{
  // Warning: These are serialized to players in the database! Do not change their order!
  King     = 0,
  Force    = 1,
  Fire     = 2,
  Snake    = 3,
  NumMarks = 4
}

enum U3_ShrineType
{
  // Warning: These are serialized! Do not change their order!
  Strength,
  Dexterity,
  Intelligence,
  Wisdom
}
