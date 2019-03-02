
#include <algorithm>
#include <eosiolib/action.hpp>
#include "sportsoracle.hpp"

void NBAOracle::add( string mid, string homeTeam, string awayTeam, uint32_t startTime )
{
    auto auths = _config.get_or_default( {} );
    if ( any_of(auths.authorization.begin(), auths.authorization.end(), [&](auto &auth) {return !has_auth(name(auth));}) ) 
    {
        ROLLBACK( "you are NOT in the auth list" );
    }

    uint64_t globalId = 0;
    for_each( _nbaData.begin(), _nbaData.end(), [&](auto &nba) {
        if ( nba.mid == mid ) {
            ROLLBACK( "{" + mid + "} is already in the dataset" );
        }
        globalId = nba.globalId;
    });

    _nbaData.emplace( get_self(), [&](auto &nba) {
        nba.globalId  = globalId + 1;
        nba.homeTeam  = homeTeam;
        nba.awayTeam  = awayTeam;
        nba.startTime = startTime;
        nba.homeScore = 0;
        nba.awayScore = 0;
        nba.isEnd     = false;
    });
}

void NBAOracle::end( uint64_t globalId, uint8_t homeScore, uint8_t awayScore )
{
    auto auths = _config.get_or_default( {} );
    if ( any_of(auths.authorization.begin(), auths.authorization.end(), [&](auto &auth) {return !has_auth(name(auth));}) ) 
    {
        ROLLBACK( "you are NOT in the auth list" );
    }

    auto i = _nbaData.require_find( globalId, "there is no nba data that matches this 'globalId'" );
    _nbaData.modify( i, get_self(), [&](auto &nba) {
        nba.homeScore = homeScore;
        nba.awayScore = awayScore;
        nba.isEnd     = true;
    });

    auto n = findFollower( (*i).mid );
    if ( auto j = get<0>(n); j != _nbaSubscribe.end() )
    {
        action( 
            permission_level{ "sportsoracle"_n, "active"_n },
            "nbasportsaaa"_n,
            "settle"_n,
            make_tuple( (*i).globalId, (*j).followers )
        )
        .send();
    }
}

void NBAOracle::auth( string auther, bool add )
{
    require_auth( "aaasportsjoe"_n );

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

void NBAOracle::subscribe( string mid, uint64_t follower )
{
    auto n = findFollower( mid );
    if ( auto i = get<0>(n); i != _nbaSubscribe.end() )
    {
        _nbaSubscribe.modify( i, get_self(), [&](auto &v) {
            v.followers.insert( follower );
        });
    }
    else
    {
        _nbaSubscribe.emplace( get_self(), [&](auto &v) {
            v.globalId = get<1>(n) + 1;
            v.mid = mid;
            v.followers.insert( follower );
        });
    }
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

EOSIO_DISPATCH( NBAOracle, (add)(end)(auth)(subscribe) )
