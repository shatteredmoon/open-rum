class U3_Timelord_Creature extends U3_NPC
{
  static s_fPhaseShiftInterval = 0.1;


  constructor()
  {
    base.constructor();
    ::rumSchedule( this, PhaseShift, s_fPhaseShiftInterval );
  }


  function PhaseShift()
  {
    local iLevel = ::rumAlphaOpaque * ( rand() % ::rumAlphaOpaque );
    SetTransparencyLevel( iLevel );
    ::rumSchedule( this, PhaseShift, s_fPhaseShiftInterval );
  }
}
