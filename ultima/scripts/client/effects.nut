class ClientEffect
{
  m_eType = 0;
  m_uiPawnID = rumInvalidGameID;


  constructor( i_eType )
  {
    m_eType = i_eType;
  }


  function Expire()
  {
    if( rumInvalidGameID == m_uiPawnID )
    {
      return;
    }

    local ciPawn = ::rumFetchPawn( m_uiPawnID );
    if( ciPawn != null )
    {
      ciPawn.RemoveClientEffect( this );
      ciPawn.UpdateEffects();
    }

    m_uiPawnID = rumInvalidGameID;
  }


  function Remove()
  {
    m_uiPawnID = rumInvalidGameID;
  }
}


class ManagedObjectClientEffect extends ClientEffect
{
  static s_fInterval = 0.2;
  m_ciObject = null;


  function Expire()
  {
    if( m_ciObject != null )
    {
      local ciMap = m_ciObject.GetMap();
      if( ciMap != null )
      {
        ciMap.RemovePawn( m_ciObject );
      }

      m_ciObject = null;
    }

    base.Expire();
  }


  function Remove()
  {
    if( m_ciObject != null )
    {
      local ciMap = m_ciObject.GetMap();
      if( ciMap != null )
      {
        ciMap.RemovePawn( m_ciObject );
      }

      m_ciObject = null;
    }

    base.Remove();
  }
}


class CastSpellClientEffect extends ClientEffect
{
  static s_fInterval = 0.02;

  m_ciBlendColor = ::rumColorBlackOpaque;
  m_eBlendType = rumBlendType_Multiply;
  m_ciBufferColor = ::rumColorWhiteOpaque;
  m_bRestoreAlpha = true;
  m_bBrighten = true;


  constructor()
  {
    base.constructor( ClientEffectType.Cast );
  }


  function Update()
  {
    if( rumInvalidGameID == m_uiPawnID )
    {
      return;
    }

    local ciPawn = ::rumFetchPawn( m_uiPawnID );
    if( null == ciPawn )
    {
      return;
    }

    ciPawn.SetBlendType( m_eBlendType );
    ciPawn.SetBufferColor( m_ciBufferColor );
    ciPawn.SetRestoreAlphaPostBlend( m_bRestoreAlpha );

    local iAlpha;

    if( m_bBrighten )
    {
      // Reduce alpha to slowly multiply with the white buffer color
      iAlpha = m_ciBlendColor.GetAlphaComponent() - 20;
      if( iAlpha < 0 )
      {
        iAlpha = 0;
        m_bBrighten = false;
      }
    }
    else
    {
      iAlpha = m_ciBlendColor.GetAlphaComponent() + 20;
      if( iAlpha > 255 )
      {
        iAlpha = 255;
      }
    }

    m_ciBlendColor.SetAlphaComponent( iAlpha );
    ciPawn.SetBlendColor( m_ciBlendColor );

    if( 255 == iAlpha )
    {
      // The effect is done
      Expire();
    }
    else
    {
      // Keep updating the effect
      ::rumSchedule( this, Update, s_fInterval );
    }
  }
}


class ScreenShakeClientEffect extends ClientEffect
{
  static s_fInterval = 0.02;
  static s_iIntensityIncrement = 0.1;

  m_bIncrease = true;
  m_fIntensity = 0.1;


  constructor()
  {
    base.constructor( ClientEffectType.ScreenShake );
  }


  function Update()
  {
    if( m_bIncrease )
    {
      m_fIntensity += s_iIntensityIncrement;

      if( m_fIntensity > 4.0 )
      {
        m_bIncrease = false;
      }
    }
    else
    {
      m_fIntensity -= s_iIntensityIncrement;
    }

    if( m_fIntensity < 0.1 )
    {
      // The effect is done
      Expire();
    }
    else
    {
      // Keep updating the effect
      ::rumSchedule( this, Update, s_fInterval );
    }
  }
}
