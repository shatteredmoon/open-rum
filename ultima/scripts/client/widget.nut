class Widget extends rumWidget
{
  m_ciEffectsTable = null;


  constructor()
  {
    base.constructor();

    m_ciEffectsTable = {};
  }


  function AddClientEffect( i_ciClientEffect )
  {
    // A weakref is not used here because this is the only thing that keeps the object from being garbage collected
    m_ciEffectsTable[i_ciClientEffect] <- i_ciClientEffect;
  }


  function GetDescription()
  {
    return ::rumGetStringByName( GetName() + "_Widget_client_StringID" );
  }


  function Look()
  {
    // Only send if this widget exists on the server
    if( GetOriginType() == rumServerOriginType )
    {
      local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetID() );
      ::rumSendBroadcast( ciBroadcast );
    }
  }


  function LookCallback()
  {
    g_ciUI.m_ciGameInputTextBox.Clear();

    // Push the player's selection to output
    local strResponse = g_ciUI.m_ciYesNoListView.GetCurrentEntry();
    ShowString( strResponse + "<b>" );

    local eResponse = g_ciUI.m_ciYesNoListView.GetSelectedKey();
    if( YesNoResponse.Yes == eResponse )
    {
      // Only send if this widget exists on the server
      if( GetOriginType() == rumServerOriginType )
      {
        local ciBroadcast = ::rumCreate( Player_Look_BroadcastID, GetID() );
        ::rumSendBroadcast( ciBroadcast );
      }
    }

    Ultima_ListSelectionEnd();
  }


  function OnAnimate( i_uiCurrentFrame )
  {
    return ( rand()%2 == 0 ? i_uiCurrentFrame + 1 : i_uiCurrentFrame );
  }


  function OnObjectReleased()
  {
    // Release all affects since they potentially hold a reference to the widget
    foreach( ciClientEffect in m_ciEffectsTable )
    {
      ciClientEffect.Remove();
    }

    m_ciEffectsTable.clear();
  }


  function OnPawnRemoved()
  {
    // Release all affects since they potentially hold a reference to the widget
    foreach( ciClientEffect in m_ciEffectsTable )
    {
      ciClientEffect.Remove();
    }

    m_ciEffectsTable.clear();
  }


  function OnPositionUpdated( i_ciNewPos, i_ciOldPos )
  {
    // Update any client effects that would be affected by the widget's move
    local ciMap = GetMap();
    if( ciMap != null )
    {
      foreach( idx, ciClientEffect in m_ciEffectsTable )
      {
        if( ciClientEffect && ( ClientEffectType.Damage == ciClientEffect.m_eType ) )
        {
          ciMap.MovePawn( ciClientEffect.m_ciObject, i_ciNewPos,
                          rumIgnoreTileCollisionMoveFlag | rumIgnorePawnCollisionMoveFlag |
                          rumIgnoreDistanceMoveFlag );
        }
      }
    }
  }


  function OnPropertyUpdated( i_ePropertyID, i_vValue )
  {}


  function OnStateUpdated( i_iState )
  {
    UseAnimationSet( i_iState );
  }


  function RemoveClientEffect( i_ciClientEffect )
  {
    delete m_ciEffectsTable[i_ciClientEffect];
  }


  function UpdateEffects()
  {}
}


class U1_Widget extends Widget
{}


class U2_Widget extends Widget
{}


class U3_Widget extends Widget
{}


class U4_Widget extends Widget
{}


class U4_Magic_Field_Widget extends U4_Widget
{}
