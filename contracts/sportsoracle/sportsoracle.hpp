
#include <tuple>
#include "../common/tables.hpp"

class [[eosio::contract("sportsoracle")]] NBAOracle
    : public contract
{
    SINGLE::Config      _config;
    INDEX::NBAData      _nbaData;
    INDEX::NBASubscribe _nbaSubscribe;

public:
    NBAOracle( name receiver, name code, datastream<const char*> ds )
        : contract( receiver, code, ds )
        , _config( get_self(), get_self().value )
        , _nbaData( get_self(), get_self().value )
        , _nbaSubscribe( get_self(), get_self().value )
    {}

    [[eosio::action]]
    void add( string mid, string homeTeam, string awayTeam, uint32_t startTime );

    [[eosio::action]]
    void close( string mid, uint8_t homeScore, uint8_t awayScore, uint32_t endTime );

    [[eosio::action]]
    void erase( string mid );

    [[eosio::action]]
    void auth( name auther, bool add );

    [[eosio::action]]
    void setguess( name guess );

    [[eosio::action]]
    void subscribe( string mid, uint64_t follower, bool follow );

private:
    tuple<INDEX::NBASubscribe::const_iterator, uint64_t> findFollower( string mid );

    void checkAuth();
};
