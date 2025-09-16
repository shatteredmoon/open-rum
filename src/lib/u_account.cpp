#include <u_account.h>

// Static initializers
rumAccount* rumAccount::s_pcInstance{ nullptr };

// TODO - compile time assert that AccountCreateResultType's greatest value < 256
// It is sent in a single byte back to the client on creation failure
const char* rumAccount::s_aAccountCreateResultTypeStrings[] =
{
  "",
  "rum_account_create_error",
  "rum_account_create_name_exists",
  "rum_account_create_name_invalid",
  "rum_account_create_password_invalid",
  "rum_account_create_password_mismatch",
  "rum_account_create_timeout"
};

// TODO - compile time assert that AccountLoginResulType's greatest value < 256
// It is sent in a single byte back to the client on login failure
const char* rumAccount::s_aAccountLoginResultTypeStrings[] =
{
  "",
  "rum_account_login_error",
  "rum_account_create_name_invalid",     // purposefully reused
  "rum_account_create_password_invalid", // purposefully reused
  "rum_account_login_not_found",
  "rum_account_login_authentication_failed",
  "rum_account_login_already_active",
};


// static
rumAccount& rumAccount::GetInstance()
{
  if( !s_pcInstance )
  {
    s_pcInstance = new rumAccount;
  }

  return *s_pcInstance;
}


// static
bool rumAccount::IsSignedIn_VM()
{
  return GetInstance().IsSignedIn();
}


// static
void rumAccount::ScriptBind()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  Sqrat::RootTable( pcVM )
    .Func( "rumIsAccountLoggedIn", IsSignedIn_VM )
    .Func( "rumGetAccountCharacters", GetCharacters_VM )
    .Func( "rumGetNumAccountCharacters", GetNumCharacters_VM );
}


// static
Sqrat::Object rumAccount::GetCharacters_VM()
{
  HSQUIRRELVM pcVM{ Sqrat::DefaultVM::Get() };

  // It's okay to have zero length squirrel arrays
  Sqrat::Array sqArray( pcVM, (int32_t)GetInstance().m_listCharacters.size() );

  uint32_t uiIndex{ 0 };
  for( const auto& iter : GetInstance().m_listCharacters )
  {
    // Add the character name
    sqArray.SetValue( uiIndex++, iter.c_str() );
  }

  return sqArray;
}
