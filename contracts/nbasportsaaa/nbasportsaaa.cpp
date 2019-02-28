
#include "nbasportsaaa.hpp"

void NBASports::setconfig( GUESS::NBAConfig config )
{
    require_auth( get_self() );
    _config.set( config, get_self() );
}

void NBASports::transfer( name from, name to, asset quantity, string memo )
{
    if ( from == to || from == get_self() || to != get_self() || quantity.symbol != symbol("EOS", 4) )
    {
        ROLLBACK( "invalid transfer params" );
    }

    if ( memo.substr(0, strlen("create")) == "create" )
    {
        create( memo.substr(sizeof("create")) );
    }
    else if ( memo.substr(0, strlen("join")) == "join" )
    {
        join( memo.substr(sizeof("join")) );
    }
    else
    {
        ROLLBACK( "invalid memo" );
    }
}

void NBASports::create( string_view param )
{

}

void NBASports::join( string_view param )
{

}
