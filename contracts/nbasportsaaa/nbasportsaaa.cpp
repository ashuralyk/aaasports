
#include <algorithm>
#include <functional>
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
        create( memo.substr(sizeof("create")), from, quantity );
    }
    else if ( memo.substr(0, strlen("join")) == "join" )
    {
        join( memo.substr(sizeof("join")), from, quantity );
    }
    else
    {
        ROLLBACK( "invalid memo: must go with 'create' or 'join' as fist word" );
    }
}

void NBASports::create( string &&param, name creator, asset value )
{
    vector<string> params = split( param, '|' );
    if ( params.size() != 4 )
    {
        ROLLBACK( "invalid 'create' memo: create|mid|bet|type|score|" );
    }

    bool hasMid;
    ORACLE::NBAData nbaData;

    tie(hasMid, nbaData) = findOracleByType<0>( params[0] );

    if ( false == hasMid )
    {
        ROLLBACK( "the 'mid' from the memo doesn't exist" );
    }

    uint64_t globalId = pushGuess({
        .mid         = params[0],
        .bet         = static_cast<uint8_t>( stoul(params[1]) ),
        .type        = static_cast<uint8_t>( stoul(params[2]) ),
        .score       = static_cast<uint8_t>( stoul(params[3]) ),
        .creator     = creator,
        .tokenAmount = value
    });

    if ( 0 == globalId )
    {
        ROLLBACK( "the 'mid' is already created by you" );
    }

    action( 
        permission_level{ "nbasportsaaa"_n, "active"_n },
        "sportsoracle"_n,
        "subscribe"_n,
        make_tuple( params[0], globalId )
    )
    .send();
}

void NBASports::join( string &&param, name player, asset value )
{
    vector<string> params = split( param, '|' );
    if ( params.size() != 1 )
    {
        ROLLBACK( "invalid 'create' memo: join|mid|creator|" );
    }

    bool find = false;
    auto i = _nbaGuess.end();
    tie(find, i) = findGuessByType<0>( params[0] );

    if ( false == find )
    {
        ROLLBACK( "the 'mid' doesn't exist" );
    }

    if ( (*i).creator == name(params[1]) )
    {
        ROLLBACK( "this guess isn't created by this creator" );
    }

    if ( (*i).player != name() )
    {
        ROLLBACK( "this guess is already on playing" );
    }

    if ( (*i).tokenAmount != value )
    {
        ROLLBACK( "the token doesn't match this guess" );
    }

    _nbaGuess.modify( i, get_self(), [&](auto &v) {
        v.player = player;
    });
}

void NBASports::settle( uint64_t globalId, vector<uint64_t> followers )
{
    require_auth( "sportsoracle"_n );
    
    bool find;
    ORACLE::NBAData oracle;
    tie(find, oracle) = findOracleByType<1>( globalId );

    if ( false == find )
    {
        ROLLBACK( "the 'globalId' from oracle doesn't exist" );
    }

    if ( false == oracle.isEnd || oracle.homeScore == 0 || oracle.awayScore == 0 )
    {
        ROLLBACK( "the flags 'isEnd', 'homeScore', 'awayScore' aren't set yet" );
    }

    for ( uint64_t follower : followers )
    {
        auto i = _nbaGuess.end();
        tie(find, i) = findGuessByType<1>( follower );

        if ( false == find )
        {
            ROLLBACK( "follower " + to_string(follower) + " doesn't exist in the guess set" );
        }

        if ( (*i).winner != name() )
        {
            ROLLBACK( "this guess already has a winner" );
        }

        _nbaGuess.modify( i, get_self(), [&](auto &v) {
            v.winner = getWinner( oracle, v );
        });
    }
}

vector<string> NBASports::split( string &view, char s )
{
    vector<string> params;
    for ( uint32_t i = view.find(s), j = 0; i != string::npos; j = i + 1, i = view.find(s, j) )
    {
        params.push_back( view.substr(j, i) );
    }
    return params;
}

uint64_t NBASports::pushGuess( GUESS::NBAGuess &&guess )
{
    uint64_t globalId = 0;
    if ( guess.type < 3 || guess.bet < 2 )
    {
        for ( auto i = _nbaGuess.begin(); i != _nbaGuess.end(); ++i )
        {
            auto nba = (*i);
            globalId = nba.globalId;
            if ( nba.mid == guess.mid && nba.creator == guess.creator )
            {
                globalId = 0;
                break;
            }
        }

        if ( globalId > 0 )
        {
            _nbaGuess.emplace( get_self(), [&](auto &v) {
                v = guess;
                v.globalId = globalId + 1;
            });
        }
    }

    return globalId;
}

name NBASports::getWinner( ORACLE::NBAData &oracle, GUESS::NBAGuess &guess )
{
    function<bool()> creatorWin;
    switch ( guess.type )
    {
        // 独赢
        case 0:
        {
            creatorWin = [&]() {
                if ( guess.bet == 0 ) {
                    return oracle.homeScore > oracle.awayScore;
                } else {
                    return oracle.homeScore < oracle.awayScore;
                }
            };
        }
        break;
        // 总分
        case 1:
        {
            creatorWin = [&]() {
                if ( guess.bet == 0 ) {
                    return static_cast<float>(oracle.homeScore + oracle.awayScore) > (guess.score / 10.f);
                } else {
                    return static_cast<float>(oracle.homeScore + oracle.awayScore) < (guess.score / 10.f);
                }
            };
        }
        break;
        // 分差
        case 2:
        {
            creatorWin = [&]() {
                if ( guess.bet == 0 ) {
                    return static_cast<float>(oracle.homeScore - oracle.awayScore) > (guess.score / 10.f);
                } else {
                    return static_cast<float>(oracle.homeScore - oracle.awayScore) < (guess.score / 10.f);
                }
            };
        }
        break;
    }

    return creatorWin() ? guess.creator : guess.player;
}

extern "C"
{

void apply( uint64_t receiver, uint64_t code, uint64_t action )
{
    if( code == receiver )
    {
        switch( action )
        {
            EOSIO_DISPATCH_HELPER( NBASports, (setconfig)(settle) )
        }
    }
    else
    {
        if ( code == ("eosio.token"_n).value && action == "transfer"_n.value )
        {
            execute_action( name(receiver), name(code), &NBASports::transfer );
        }
    }
}

}
