#ifndef _U_ASSERT_H_
#define _U_ASSERT_H_

#ifdef _DEBUG

#include <sstream>

void AssertDlg( const char* i_strText, const char* i_strCaption );

// TODO: Use these when std::format lands...

/*#define rumAssert( exp ) \
  do { \
    if( !( exp ) ) { \
      AssertDlg( std::format( "{}:{} {} failed {}", __FILE__, __LINE__, __func__, #exp ).c_str(),\
                 "Assertion Failure" ); \
    } } while( false )

#define rumAssertMsg( exp, msg ) \
  do { \
    if( !( exp ) ) { \
      AssertDlg( std::format( "{}:{} {} failed {}\n\n{}", __FILE__, __LINE__, __func__, #exp, #msg ).c_str(),\
                 "Assertion Failure" ); \
    } } while( false )
#else*/

#define rumAssert( exp ) \
  do { \
    if( !( exp ) ) { \
      std::stringstream ss; \
      ss << "Failed \"" << #exp << "\" in " __FILE__ << " " << __func__ << " line " << __LINE__; \
      AssertDlg( ss.str().c_str(), "Assertion Failure" ); \
    } } while( false )

#define rumAssertMsg( exp, msg ) \
  do { \
    if( !( exp ) ) { \
      std::stringstream ss; \
      ss << "Failed \"" << #exp << "\" in " __FILE__ << " " << __func__ << " line " << __LINE__ << "\n\n" << #msg; \
      AssertDlg( ss.str().c_str(), "Assertion Failure" ); \
    } } while( false )

#define rumAssertArgs( exp, ... ) \
  do { \
    if( !( exp ) ) { \
      std::stringstream ss; \
      ss << "Failed \"" << #exp << "\" in " __FILE__ << " " << __func__ << " line " << __LINE__ << "\n\n" << __VA_ARGS__; \
      AssertDlg( ss.str().c_str(), "Assertion Failure" ); \
    } } while( false )

#else
  #define rumAssert( exp ) ( (void)0 )
  #define rumAssertMsg( exp, msg ) ( (void)0 )
  #define rumAssertArgs( exp, ... ) ( (void)0 )
#endif // _DEBUG

#endif // _U_ASSERT_H_
