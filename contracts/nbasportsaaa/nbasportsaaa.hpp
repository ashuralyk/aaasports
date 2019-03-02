
#include "../common/tables.hpp"

class [[eosio::contract("nbasportsaaa")]] NBASports
    : public contract
{
    SINGLE::NBAConfig _config;
    INDEX::NBAGuess   _nbaGuess;
    INDEX::NBAData    _nbaOracle;

public:
    NBASports( name receiver, name code, datastream<const char*> ds )
        : contract( receiver, code, ds )
        , _config( get_self(), get_self().value )
        , _nbaGuess( get_self(), get_self().value )
        , _nbaOracle( "sportsoracle"_n, ("sportsoracle"_n).value )
    {}

    [[eosio::action]]
    void setconfig( GUESS::NBAConfig config );

    void transfer( name from, name to, asset quantity, string memo );

    void create( string_view param, name creator, asset value );

    void join( string_view param, name player, asset value );

    [[eosio::action]]
    void settle();

private:
    vector<string_view> split( string_view view, char s );

    tuple<bool, ORACLE::NBAData> findByMid( string_view mid );
};
