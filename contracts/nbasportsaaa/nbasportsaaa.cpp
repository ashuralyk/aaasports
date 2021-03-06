
#include <functional>
#include "nbasportsaaa.hpp"

void NBASports::setconfig( GUESS::NBAConfig config, bool del )
{
    require_auth( get_self() );
    if ( del )
    {
        _config.remove();
    }
    else
    {
        eosio_assert( config.failedFeeRate        > 0.f, "the 'failedFeeRate' must be greater than 0" );
        eosio_assert( config.winnerFeeRate        > 0.f, "the 'winnerFeeRate' must be greater than 0" );
        eosio_assert( config.overCreate.startRate > 0,   "the 'startRate' in overCreate must be greater than 0" );
        eosio_assert( config.overCreate.perRate   > 0,   "the 'perRate' in overCreate must be greater than 0" );
        eosio_assert( config.overCreate.perCreate > 0,   "the 'perCreate' in overCreate must be greater than 0" );
        _config.set( config, get_self() );
    }
}

void NBASports::transfer( name from, name to, asset quantity, string memo )
{
    if ( from == get_self() || to != get_self() )
    {
        return;
    }

    if ( from == to || quantity.symbol != symbol("EOS", 4) )
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
        ROLLBACK( "invalid memo: must go with 'create' or 'join' as the fist word" );
    }
}

void NBASports::create( string &&param, name creator, asset value )
{
    vector<string> params = split( param, '|' );
    if ( params.size() != 4 || stoul(params[1]) > 1 || stoul(params[2]) > 2 )
    {
        ROLLBACK( "invalid 'create' memo: create|mid|bet[=0,1]|type[=0,1,2]|score|" );
    }

    print( params[0], ", ", params[1], ", ", params[2], ", ", params[3] );

    auto [hasMid, nbaData] = findOracleByType<0>( params[0] );

    if ( false == hasMid )
    {
        ROLLBACK( "the 'mid' from the memo doesn't exist in the data set" );
    }

    if ( nbaData.endTime > 0 || nbaData.homeScore > 0 || nbaData.awayScore > 0 )
    {
        ROLLBACK( "the game represented by this 'mid' from the memo has closed yet" );
    }

    if ( now() > nbaData.startTime )
    {
        ROLLBACK( "the onchained-time is already behind the beginning time, create or join are both closed." );
    }

    auto [pushed, globalId] = pushGuess({
        .mid         = params[0],
        .bet         = static_cast<uint8_t>( stoul(params[1]) ),
        .type        = static_cast<uint8_t>( stoul(params[2]) ),
        .score       = stof(params[3]),
        .creator     = creator,
        .tokenAmount = value
    });

    if ( false == pushed )
    {
        ROLLBACK( "the 'mid' with this guess type is already created by " + creator.to_string() );
    }

    action( 
        permission_level{ get_self(), "active"_n },
        requireOracleAccount(),
        "subscribe"_n,
        make_tuple( params[0], globalId, true )
    )
    .send();
}

void NBASports::join( string &&param, name player, asset value )
{
    vector<string> params = split( param, '|' );
    if ( params.size() != 2 )
    {
        ROLLBACK( "invalid 'join' memo: join|mid|creator|" );
    }

    // 检查数据集合
    auto [hasMid, nbaData] = findOracleByType<0>( params[0] );

    if ( false == hasMid )
    {
        ROLLBACK( "the 'mid' from the memo doesn't exist in the data set" );
    }

    if ( now() > nbaData.startTime )
    {
        ROLLBACK( "the onchained-time is already behind the beginning time, create or join are both closed." );
    }

    // 检查竞猜集合
    struct T
    {
        string mid;
        name   creator;
    };

    auto [find, i] = findGuessByType<0>( T{
        .mid     = params[0],
        .creator = name( params[1] )
    });

    if ( false == find )
    {
        ROLLBACK( "the 'mid' doesn't exist in the guess set" );
    }

    if ( (*i).creator != name(params[1]) )
    {
        ROLLBACK( "this guess isn't created by " + (*i).creator.to_string() );
    }

    if ( (*i).player != name() )
    {
        ROLLBACK( "this guess is already on playing, and the player is " + (*i).player.to_string() );
    }

    if ( (*i).tokenAmount != value )
    {
        ROLLBACK( "the imported token amount doesn't match this guess's" );
    }

    if ( (*i).creator == player )
    {
        ROLLBACK( "you can't play with yourself" );
    }

    if ( (*i).winner != name() )
    {
        ROLLBACK( "this guess already has a winner who is " + (*i).winner.to_string() + ", so refuse to join" );
    }

    _nbaGuess.modify( i, get_self(), [&](auto &v) {
        v.player = player;
    });
}

void NBASports::settle( uint64_t globalId, vector<uint64_t> followers )
{
    require_auth( requireOracleAccount() );
    
    bool find;
    ORACLE::NBAData oracle;
    tie(find, oracle) = findOracleByType<1>( globalId );

    if ( false == find )
    {
        ROLLBACK( "the 'globalId' from oracle doesn't exist" );
    }

    if ( oracle.endTime == 0 || oracle.homeScore == 0 || oracle.awayScore == 0 )
    {
        ROLLBACK( "the flags 'endTime', 'homeScore', 'awayScore' aren't set yet" );
    }

    for ( uint64_t follower : followers )
    {
        auto [find, i] = findGuessByType<1>( follower );

        if ( false == find )
        {
            ROLLBACK( "follower " + to_string(follower) + " doesn't exist in the guess set" );
        }

        if ( (*i).winner != name() )
        {
            ROLLBACK( "this guess already has a winner who is " + (*i).winner.to_string() );
        }

        _nbaGuess.modify( i, get_self(), [&](auto &v) {
            // 结束的时候还没有人参加
            if ( v.player == name() )
            {
                v.winner = v.creator;
                print( "FAIL: old = ", v.tokenAmount );
                asset payback = v.tokenAmount - getFee<0>( v.tokenAmount );
                print( ", new = ", payback );
                action(
                    permission_level{ get_self(), "active"_n },
                    "eosio.token"_n,
                    "transfer"_n,
                    make_tuple( get_self(), v.creator, payback, "您在AAASports上的竞猜{" + v.mid + "}已结束，返还无人参加情况下的EOS(已扣除手续费)" )
                ).send();
            }
            // 正常结束竞猜
            else
            {
                v.winner = getWinner( oracle, v );
                print( "WIN: old = ", v.tokenAmount * 2 );
                asset payback = 2 * (v.tokenAmount - getFee<1>( v.tokenAmount ));
                print( ", new = ", payback );
                action(
                    permission_level{ get_self(), "active"_n },
                    "eosio.token"_n,
                    "transfer"_n,
                    make_tuple( get_self(), v.winner, payback, "恭喜，您在AAASports上赢得竞猜{" + v.mid + "}，发放EOS(已扣除手续费)" )
                ).send();
            }
        });
    }
}

void NBASports::erase( string mid, name creator )
{
    if (! has_auth(requireOracleAccount()) )
    {
        require_auth( get_self() );
    }

    auto i = find_if( _nbaGuess.begin(), _nbaGuess.end(), [&](auto &v) {
        return v.mid == mid && v.creator == creator;
    });

    if ( i == _nbaGuess.end() )
    {
        ROLLBACK( "there isn't {" + mid + "} in the guess set" );
    }

    auto nba = (*i);
    print( nba.creator, ", ", nba.player, ", ", nba.tokenAmount );
    _nbaGuess.erase( i );

    if ( nba.winner == name() )
    {
        // 返回创建者游戏币
        action(
            permission_level{ get_self(), "active"_n },
            "eosio.token"_n,
            "transfer"_n,
            make_tuple( get_self(), nba.creator, nba.tokenAmount, "您在AAASports上创建的竞猜{" + mid + "}已被删除，退还抵押的EOS" )
        ).send();

        // 返回加入者游戏币
        if ( nba.player != name() )
        {
            action(
                permission_level{ get_self(), "active"_n },
                "eosio.token"_n,
                "transfer"_n,
                make_tuple( get_self(), nba.player, nba.tokenAmount, "您在AAASports上加入的竞猜{" + mid + "}已被删除，退还抵押的EOS" )
            ).send();
        }
    }

    // 注销在预言机中的订阅服务
    action(
        permission_level{ get_self(), "active"_n },
        requireOracleAccount(),
        "subscribe"_n,
        make_tuple( nba.mid, nba.globalId, false )
    ).send();
}

void NBASports::clear()
{
    require_auth( get_self() );

    _config.remove();
    while( _nbaGuess.begin() != _nbaGuess.end() )
        _nbaGuess.erase( _nbaGuess.begin() );
}

float NBASports::stof( string &view )
{
    const char *s = view.c_str();
    float rez = 0, fact = 1;
    if (*s == '-') {
        s++;
        fact = -1;
    }

    for (int point_seen = 0; *s; s++) {
        if (*s == '.') {
            point_seen = 1; 
            continue;
        }

        int d = *s - '0';
        if (d >= 0 && d <= 9) {
            if (point_seen) fact /= 10.0f;
            rez = rez * 10.0f + (float)d;
        }
    }

    return rez * fact;
}

vector<string> NBASports::split( string &view, char s )
{
    vector<string> params;
    for ( uint32_t i = view.find(s), j = 0; i != string::npos; j = i + 1, i = view.find(s, j) )
    {
        params.push_back( view.substr(j, i - j) );
    }
    return params;
}

tuple<bool, uint64_t> NBASports::pushGuess( GUESS::NBAGuess &&guess )
{
    uint64_t globalId = 0;

    if ( guess.type < 3 || guess.bet < 2 )
    {
        for ( auto i = _nbaGuess.begin(); i != _nbaGuess.end(); ++i )
        {
            auto nba = (*i);
            globalId = nba.globalId;
            if ( nba.mid == guess.mid && nba.creator == guess.creator && nba.type == guess.type )
            {
                return make_tuple( false, 0 );
            }
        }

        // 扣除创建过多情况下的手续费
        print( " | CREATE: old = ", guess.tokenAmount );
        guess.tokenAmount -= getFee<2>( guess.tokenAmount, guess.creator );
        print( ", new = ", guess.tokenAmount );

        if ( auto lowerBound = getLowerBoundAsset(guess.creator); guess.tokenAmount < lowerBound )
        {
            ROLLBACK( "your asset doesn't match our starting foud of yours, please increase and try again (at least " + lowerBound.to_string() + ", current is " + guess.tokenAmount.to_string() + ")" );
        }

        // 加入竞猜
        _nbaGuess.emplace( get_self(), [&](auto &v) {
            v = guess;
            v.globalId = ++globalId;
        });
    }

    return make_tuple( true, globalId );
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
                    return static_cast<float>(oracle.homeScore + oracle.awayScore) > guess.score;
                } else {
                    return static_cast<float>(oracle.homeScore + oracle.awayScore) < guess.score;
                }
            };
        }
        break;
        // 分差
        case 2:
        {
            creatorWin = [&]() {
                if ( guess.bet == 0 ) {
                    return static_cast<float>(oracle.homeScore) - static_cast<float>(oracle.awayScore) > guess.score;
                } else {
                    return static_cast<float>(oracle.homeScore) - static_cast<float>(oracle.awayScore) < guess.score;
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
            EOSIO_DISPATCH_HELPER( NBASports, (setconfig)(settle)(erase)(clear) )
        }
    }
    else
    {
        if ( code == "eosio.token"_n.value && action == "transfer"_n.value )
        {
            execute_action( name(receiver), name(code), &NBASports::transfer );
        }
    }
}

}
