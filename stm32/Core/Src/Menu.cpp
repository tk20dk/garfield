#include <Garfield.h>


struct
{
  char Name[ 8 ];
  uint32_t Length;
  void (TGarfield::*Func)( char const* );
} DataHandler[] =
{
  { "log ", 4, &TGarfield::LogHandler },
  { "help", 4, &TGarfield::HelpHandler }
};


void TGarfield::UartLoop()
{
  char const* Line = UartGetLine();
  if( Line == nullptr )
  {
    return;
  }

  if( *Line == '\0' )
  {
    UartPrintf( "garfield:> " );
    return;
  }

  for( auto &Handler : DataHandler )
  {
    if( memcmp( Line, Handler.Name, Handler.Length ) == 0 )
    {
      Line += Handler.Length;
      SkipSpace( Line );
      (this->*Handler.Func)( Line );
      UartPrintf( "garfield:> " );
      return;
    }
  }

  UartPrintf( "Unknown command\n" );
  UartPrintf( "garfield:> " );
}

void TGarfield::LogHandler( char const* const Line )
{
  if( strcmp( Line, "on" ) == 0 )
  {
    LogMode = true;
  }
  else if( strcmp( Line, "off" ) == 0 )
  {
    LogMode = false;
  }
  else
  {
    UartPrintf( "Bad log arguments\n" );
  }
}

void TGarfield::HelpHandler( char const* const Line )
{
  UartPrintf( "usage:\n" );
  UartPrintf( "  help\n" );
  UartPrintf( "  log on|off\n" );
}

void TGarfield::SkipSpace( char const* &Line )
{
  while(( *Line != '\0' ) && ( *Line == ' ' ))
  {
    Line++;
  }
}

