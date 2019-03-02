
#include <algorithm>
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
        ROLLBACK( "invalid memo: must go with 'create' or 'join' as fist word" );
    }
}

void NBASports::create( string_view param, name creator, asset value )
{
    vector<string_view> params = split( param, '|' );
    if ( params.size() != 4 )
    {
        ROLLBACK( "invalid 'create' memo: create|mid|team|type|score|" );
    }

    bool hasMid;
    ORACLE::NBAData nbaData;

    tie(hasMid, nbaData) = findByMid( params[0] );

    if ( false == hasMid )
    {
        ROLLBACK( "the 'mid' from the memo doesn't exist" );
    }

    
}

void NBASports::join( string_view param, name player, asset value )
{

}

vector<string_view> NBASports::split( string_view view, char s )
{
    vector<string_view> params;
    for ( auto i = view.find(s), j = 0; i != string::npos; j = i + 1, i = view.find(s, j) )
    {
        params.push_back( view.substr(j, i) );
    }
    return params;
}

tuple<bool, ORACLE::NBAData> NBASports::findByMid( string_view mid )
{
    auto i = find( _nbaOracle.begin(), _nbaOracle.end(), [&](const auto &v) {
        return v.mid == mid
    });

    if ( i == _nbaOracle.end() )
    {
        return make_tuple( false, ORACLE::NBAData() );
    }
    else
    {
        return make_tuple( true, *i );
    }
}
