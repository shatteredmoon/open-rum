// Defined in the main project CMakeLists.txt
#ifdef USE_GLFW

#ifndef _C_INPUT_GLFW_H_
#define _C_INPUT_GLFW_H_

#include <c_input.h>

#include <GLFW/glfw3.h>


class rumInput : public rumInputBase
{
public:

  rumPoint GetMousePosition() const override;

  bool IsKeyPressed( int32_t i_iKey ) const override;

  static int32_t Init();

  static void ScriptBind();

  static void SetWindow( GLFWwindow* i_pcWindow );

  static void Shutdown();

  static void Update();

private:

  static void KeyPressCallback( GLFWwindow* i_pcWindow, int32_t i_iKey, int32_t i_iScancode, int32_t i_iAction,
                                int32_t i_iMods );
  static void MouseButtonCallback( GLFWwindow* i_pcWindow, int32_t i_iButton, int32_t i_iAction, int32_t i_iMods );
  static void MousePositionCallback( GLFWwindow* i_pcWindow, double i_fXPos, double i_fYPos );
  static void MouseScrollCallback( GLFWwindow* i_pcWindow, double i_fXOffset, double i_fYOffset );

  static GLFWwindow* s_pcWindow;

  static GLFWcursorposfun s_funcPrevUserCallbackCursorPos;
  static GLFWcursorenterfun s_funcPrevUserCallbackCursorEnter;
  static GLFWmousebuttonfun s_funcPrevUserCallbackMousebutton;
  static GLFWscrollfun s_funcPrevUserCallbackScroll;
  static GLFWkeyfun s_funcPrevUserCallbackKey;
  static GLFWcharfun s_funcPrevUserCallbackChar;

  typedef rumInputBase super;
};


class rumKeypress : public rumKeypressBase
{
public:

  virtual void SetValue( int32_t i_iKey, int32_t i_iAscii = 0, int32_t i_iScancode = 0, int32_t i_iMod = 0 ) override;

  static void ScriptBind();

private:

  static int32_t KeyA() { return GLFW_KEY_A; }
  static int32_t KeyB() { return GLFW_KEY_B; }
  static int32_t KeyC() { return GLFW_KEY_C; }
  static int32_t KeyD() { return GLFW_KEY_D; }
  static int32_t KeyE() { return GLFW_KEY_E; }
  static int32_t KeyF() { return GLFW_KEY_F; }
  static int32_t KeyG() { return GLFW_KEY_G; }
  static int32_t KeyH() { return GLFW_KEY_H; }
  static int32_t KeyI() { return GLFW_KEY_I; }
  static int32_t KeyJ() { return GLFW_KEY_J; }
  static int32_t KeyK() { return GLFW_KEY_K; }
  static int32_t KeyL() { return GLFW_KEY_L; }
  static int32_t KeyM() { return GLFW_KEY_M; }
  static int32_t KeyN() { return GLFW_KEY_N; }
  static int32_t KeyO() { return GLFW_KEY_O; }
  static int32_t KeyP() { return GLFW_KEY_P; }
  static int32_t KeyQ() { return GLFW_KEY_Q; }
  static int32_t KeyR() { return GLFW_KEY_R; }
  static int32_t KeyS() { return GLFW_KEY_S; }
  static int32_t KeyT() { return GLFW_KEY_T; }
  static int32_t KeyU() { return GLFW_KEY_U; }
  static int32_t KeyV() { return GLFW_KEY_V; }
  static int32_t KeyW() { return GLFW_KEY_W; }
  static int32_t KeyX() { return GLFW_KEY_X; }
  static int32_t KeyY() { return GLFW_KEY_Y; }
  static int32_t KeyZ() { return GLFW_KEY_Z; }

  static int32_t Key0() { return GLFW_KEY_0; }
  static int32_t Key1() { return GLFW_KEY_1; }
  static int32_t Key2() { return GLFW_KEY_2; }
  static int32_t Key3() { return GLFW_KEY_3; }
  static int32_t Key4() { return GLFW_KEY_4; }
  static int32_t Key5() { return GLFW_KEY_5; }
  static int32_t Key6() { return GLFW_KEY_6; }
  static int32_t Key7() { return GLFW_KEY_7; }
  static int32_t Key8() { return GLFW_KEY_8; }
  static int32_t Key9() { return GLFW_KEY_9; }

  static int32_t KeyPad0() { return GLFW_KEY_KP_0; }
  static int32_t KeyPad1() { return GLFW_KEY_KP_1; }
  static int32_t KeyPad2() { return GLFW_KEY_KP_2; }
  static int32_t KeyPad3() { return GLFW_KEY_KP_3; }
  static int32_t KeyPad4() { return GLFW_KEY_KP_4; }
  static int32_t KeyPad5() { return GLFW_KEY_KP_5; }
  static int32_t KeyPad6() { return GLFW_KEY_KP_6; }
  static int32_t KeyPad7() { return GLFW_KEY_KP_7; }
  static int32_t KeyPad8() { return GLFW_KEY_KP_8; }
  static int32_t KeyPad9() { return GLFW_KEY_KP_9; }

  static int32_t KeyF1()  { return GLFW_KEY_F1;  }
  static int32_t KeyF2()  { return GLFW_KEY_F2;  }
  static int32_t KeyF3()  { return GLFW_KEY_F3;  }
  static int32_t KeyF4()  { return GLFW_KEY_F4;  }
  static int32_t KeyF5()  { return GLFW_KEY_F5;  }
  static int32_t KeyF6()  { return GLFW_KEY_F6;  }
  static int32_t KeyF7()  { return GLFW_KEY_F7;  }
  static int32_t KeyF8()  { return GLFW_KEY_F8;  }
  static int32_t KeyF9()  { return GLFW_KEY_F9;  }
  static int32_t KeyF10() { return GLFW_KEY_F10; }
  static int32_t KeyF11() { return GLFW_KEY_F11; }
  static int32_t KeyF12() { return GLFW_KEY_F12; }

  static int32_t KeyLeftAlt()      { return GLFW_KEY_LEFT_ALT;      }
  static int32_t KeyRightAlt()     { return GLFW_KEY_RIGHT_ALT;     }
  static int32_t KeyLeftControl()  { return GLFW_KEY_LEFT_CONTROL;  }
  static int32_t KeyRightControl() { return GLFW_KEY_RIGHT_CONTROL; }
  static int32_t KeyLeftShift()    { return GLFW_KEY_LEFT_SHIFT;    }
  static int32_t KeyRightShift()   { return GLFW_KEY_RIGHT_SHIFT;   }
  static int32_t KeyEscape()       { return GLFW_KEY_ESCAPE;        }
  static int32_t KeyDash()         { return GLFW_KEY_MINUS;         }
  static int32_t KeyEquals()       { return GLFW_KEY_EQUAL;         }
  static int32_t KeyBackspace()    { return GLFW_KEY_BACKSPACE;     }
  static int32_t KeyTab()          { return GLFW_KEY_TAB;           }
  static int32_t KeyOpenBracket()  { return GLFW_KEY_LEFT_BRACKET;  }
  static int32_t KeyCloseBracket() { return GLFW_KEY_RIGHT_BRACKET; }
  static int32_t KeyEnter()        { return GLFW_KEY_ENTER;         }
  static int32_t KeyBackslash()    { return GLFW_KEY_BACKSLASH;     }
  static int32_t KeyComma()        { return GLFW_KEY_COMMA;         }
  static int32_t KeyPeriod()       { return GLFW_KEY_PERIOD;        }
  static int32_t KeySlash()        { return GLFW_KEY_SLASH;         }
  static int32_t KeySpace()        { return GLFW_KEY_SPACE;         }
  static int32_t KeyInsert()       { return GLFW_KEY_INSERT;        }
  static int32_t KeyDelete()       { return GLFW_KEY_DELETE;        }
  static int32_t KeyHome()         { return GLFW_KEY_HOME;          }
  static int32_t KeyEnd()          { return GLFW_KEY_END;           }
  static int32_t KeyPageUp()       { return GLFW_KEY_PAGE_UP;       }
  static int32_t KeyPageDown()     { return GLFW_KEY_PAGE_DOWN;     }
  static int32_t KeyLeft()         { return GLFW_KEY_LEFT;          }
  static int32_t KeyRight()        { return GLFW_KEY_RIGHT;         }
  static int32_t KeyUp()           { return GLFW_KEY_UP;            }
  static int32_t KeyDown()         { return GLFW_KEY_DOWN;          }
  static int32_t KeyPadDivide()    { return GLFW_KEY_KP_DIVIDE;     }
  static int32_t KeyPadMultiply()  { return GLFW_KEY_KP_MULTIPLY;   }
  static int32_t KeyPadSubtract()  { return GLFW_KEY_KP_SUBTRACT;   }
  static int32_t KeyPadAdd()       { return GLFW_KEY_KP_ADD;        }
  static int32_t KeyPadDecimal()   { return GLFW_KEY_KP_DECIMAL;    }
  static int32_t KeyPadEnter()     { return GLFW_KEY_KP_ENTER;      }
  static int32_t KeyPrintScreen()  { return GLFW_KEY_PRINT_SCREEN;  }
  static int32_t KeyPause()        { return GLFW_KEY_PAUSE;         }
  static int32_t KeyPadEquals()    { return GLFW_KEY_KP_EQUAL;      }
  static int32_t KeySemicolon()    { return GLFW_KEY_SEMICOLON;     }
  static int32_t KeyApostrophe()   { return GLFW_KEY_APOSTROPHE;    }
  static int32_t KeyNumLock()      { return GLFW_KEY_NUM_LOCK;      }
  static int32_t KeyBacktick()     { return GLFW_KEY_GRAVE_ACCENT;  }

  typedef rumKeypressBase super;
};


#endif // _C_INPUT_GLFW_H_

#endif // USE_GLFW
