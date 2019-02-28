
#include "../common/tables.hpp"

class [[eosio::contract("nbasportsaaa")]] NBASports
    : public contract
{
    SINGLE::NBAConfig _config;
    INDEX::NBAGuess   _nbaGuess;

public:
    NBASports( name receiver, name code, datastream<const char*> ds )
        : contract( receiver, code, ds )
        , _config( get_self(), get_self().value )
        , _nbaGuess( get_self(), get_self().value )
    {}

    [[eosio::action]]
    void setconfig( GUESS::NBAConfig config );

    void transfer( name from, name to, asset quantity, string memo );

    void create( string_view param );

    void join( string_view param );

    [[eosio::action]]
    void settle();
}:
