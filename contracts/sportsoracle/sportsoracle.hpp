
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
    void end( uint64_t globalId, uint8_t homeScore, uint8_t awayScore );

    [[eosio::action]]
    void auth( string auther, bool add );

    [[eosio::action]]
    void subscribe( string mid, uint64_t follower );

private:
    tuple<INDEX::NBASubscribe::const_iterator, uint64_t> findFollower( string mid );
};
