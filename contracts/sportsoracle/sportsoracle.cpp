
#include <algorithm>
#include <eosiolib/action.hpp>
#include "sportsoracle.hpp"

void NBAOracle::add( string mid, string homeTeam, string awayTeam, uint32_t startTime )
{
    checkAuth();

    uint64_t globalId = 0;
    for_each( _nbaData.begin(), _nbaData.end(), [&](auto &nba) {
        if ( nba.mid == mid ) {
            ROLLBACK( "{" + mid + "} is already in the dataset" );
        }
        globalId = nba.globalId;
    });

    _nbaData.emplace( get_self(), [&](auto &nba) {
        nba.globalId  = globalId + 1;
        nba.mid       = mid;
        nba.homeTeam  = homeTeam;
        nba.awayTeam  = awayTeam;
        nba.startTime = startTime;
        nba.homeScore = 0;
        nba.awayScore = 0;
        nba.endTime   = 0;
    });
}

void NBAOracle::close( string mid, uint8_t homeScore, uint8_t awayScore, uint32_t endTime )
{
    checkAuth();

    if ( homeScore == awayScore )
    {
        ROLLBACK( "'homeScore' can't be equal to 'awayScore'" );
    }

    auto i = find_if( _nbaData.begin(), _nbaData.end(), [&](auto &v){return v.mid == mid;} );
    if ( i == _nbaData.end() )
    {
        ROLLBACK( "there is no nba data that matches {" + mid + "}" );
    }

    _nbaData.modify( i, get_self(), [&](auto &nba) {
        nba.homeScore = homeScore;
        nba.awayScore = awayScore;
        nba.endTime   = endTime;
    });

    auto n = findFollower( (*i).mid );
    if ( auto j = get<0>(n); j != _nbaSubscribe.end() )
    {
        auto gc = _config.get_or_default( {} );
        if ( gc.guessContract != name() )
        {
            action( 
                permission_level{ get_self(), "active"_n },
                gc.guessContract,
                "settle"_n,
                make_tuple( (*i).globalId, (*j).followers )
            )
            .send();
            print( "send successfully" );
        }
    }
}

void NBAOracle::erase( string mid, bool del )
{
    checkAuth();

    if ( auto i = find_if(_nbaData.begin(), _nbaData.end(), [&](auto &v) {return v.mid == mid;}); i == _nbaData.end() )
    {
        ROLLBACK( "{" + mid + "} doesn't exist" );
    }
    else
    {
        print( (*i).mid );
        _nbaData.erase( i );

        auto n = findFollower( mid );
        if ( auto j = get<0>(n); j != _nbaSubscribe.end() )
        {
            if ( del )
            {
                eraseGuess( mid, (*j).followers );
            }
            _nbaSubscribe.erase( j );
            print( " find subscribe, delete it" );
        }
    }
}

void NBAOracle::auth( name auther, bool add )
{
    require_auth( get_self() );

    auto auths = _config.get_or_default( {} );
    if ( add )
    {
        auths.authorization.insert( auther );
    }
    else
    {
        auths.authorization.erase( auther );
    }
    _config.set( auths, get_self() );
}

void NBAOracle::setguess( name guess )
{
    require_auth( get_self() );

    if (! is_account(guess) )
    {
        ROLLBACK( "please make sure the existence of the guess contract" );
    }

    auto gc = _config.get_or_default( {} );
    gc.guessContract = guess;
    _config.set( gc, get_self() );
}

void NBAOracle::subscribe( string mid, uint64_t follower, bool follow )
{
    auto gc = _config.get_or_default( {} );
    if ( gc.guessContract != name() )
    {
        require_auth( gc.guessContract );
    }
    else
    {
        ROLLBACK( "please set guess contract at first" );
    }
    
    auto n = findFollower( mid );
    if ( follow )
    {
        if ( auto i = get<0>(n); i != _nbaSubscribe.end() )
        {
            _nbaSubscribe.modify( i, get_self(), [&](auto &v) {
                v.followers.insert( follower );
            });
            print( "find it.. to modify" );
        }
        else
        {
            _nbaSubscribe.emplace( get_self(), [&](auto &v) {
                v.globalId = get<1>(n) + 1;
                v.mid = mid;
                v.followers.insert( follower );
            });
            print( "not find it.. to emplace" );
        }
    }
    else
    {
        if ( auto i = get<0>(n); i != _nbaSubscribe.end() )
        {
            bool empty;
            _nbaSubscribe.modify( i, get_self(), [&](auto &v) {
                v.followers.erase( follower );
                empty = v.followers.empty();
            });
            print( "find it.. to modify" );

            if ( empty )
            {
                _nbaSubscribe.erase( i );
            }
        }
        else
        {
            print( "not find it.. nothing happened" );
        }
    }
}

void NBAOracle::clear()
{
    require_auth( get_self() );

    _config.remove();
    while( _nbaData.begin() != _nbaData.end() )
        _nbaData.erase( _nbaData.begin() );

    while( _nbaSubscribe.begin() != _nbaSubscribe.end() )
        _nbaSubscribe.erase( _nbaSubscribe.begin() );
}

tuple<INDEX::NBASubscribe::const_iterator, uint64_t> NBAOracle::findFollower( string mid )
{
    uint64_t globalId = 0;
    for ( auto i = _nbaSubscribe.begin(); i != _nbaSubscribe.end(); ++i )
    {
        globalId = (*i).globalId;
        if ( (*i).mid == mid )
        {
            return make_tuple( i, globalId );
        }
    }

    return make_tuple( _nbaSubscribe.end(), globalId );
}

void NBAOracle::checkAuth()
{
    auto auths = _config.get_or_default( {} );
    if ( false == any_of(auths.authorization.begin(), auths.authorization.end(), [&](auto &auth) {return has_auth(auth);}) ) 
    {
        ROLLBACK( "you are NOT in the auth list" );
    }
}

void NBAOracle::eraseGuess( string mid, const set<uint64_t> &guessGlobalIds )
{
    auto gc = _config.get_or_default( {} );
    if ( gc.guessContract != name() )
    {
        INDEX::NBAGuess nbaGuess( gc.guessContract, gc.guessContract.value );
        for ( uint64_t id : guessGlobalIds )
        {
            if ( auto i = nbaGuess.find(id); i != nbaGuess.end() )
            {
                action( 
                    permission_level{ get_self(), "active"_n },
                    gc.guessContract,
                    "erase"_n,
                    make_tuple( mid, (*i).creator )
                )
                .send();
            }
        }
    }
}

EOSIO_DISPATCH( NBAOracle, (add)(close)(erase)(auth)(setguess)(subscribe)(clear) )
