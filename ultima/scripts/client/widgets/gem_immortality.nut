class U1_Gem_Immortality_Widget extends U1_Widget
{
  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {
    base.OnPropertyUpdated( i_ePropertyID, i_vValue );

    if( ( State_PropertyID == i_ePropertyID ) && ( GemImmortalityState.Vulnerable == i_vValue ) )
    {
      SetAnimationType( rumStandardOnceAnimation );
      UseAnimationSet( GemImmortalityState.Vulnerable );
      ::rumSchedule( this, ResetAnimation, GetNumAnimationFrames() * GetAnimationInterval() );
    }
  }


  function ResetAnimation()
  {
    SetAnimationType( rumCustomAnimation );
    UseAnimationSet( GemImmortalityState.Invulnerable );
  }
}
