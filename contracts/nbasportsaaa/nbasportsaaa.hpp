
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

    void create( string &&param, name creator, asset value );

    void join( string &&param, name player, asset value );

    [[eosio::action]]
    void settle( uint64_t globalId, vector<uint64_t> followers );

private:
    vector<string> split( string &view, char s );

    template < int tid, typename _Type >
    tuple<bool, ORACLE::NBAData> findOracleByType( _Type type ) {
        auto i = _nbaOracle.end();
        if constexpr ( tid == 0 ) {
            i = find_if( _nbaOracle.begin(), _nbaOracle.end(), [&](auto &v) {
                return v.mid == type;
            });
        } else if constexpr ( tid == 1 ) {
            i = _nbaOracle.find( type );
        }
        if ( i == _nbaOracle.end() ) {
            return make_tuple( false, ORACLE::NBAData() );
        } else {
            return make_tuple( true, *i );
        }
    }

    template < int tid, typename _Type >
    tuple<bool, INDEX::NBAGuess::const_iterator> findGuessByType( _Type type )
    {
        auto i = _nbaGuess.end();
        if constexpr ( tid == 0 ) {
            i = find_if( _nbaGuess.begin(), _nbaGuess.end(), [&](auto &v) {
                return v.mid == type;
            });
        } else if constexpr ( tid == 1 ) {
            i = _nbaGuess.find( type );
        }
        return make_tuple( i != _nbaGuess.end(), i );
    }

    uint64_t pushGuess( GUESS::NBAGuess &&guess );

    name getWinner( ORACLE::NBAData &oracle, GUESS::NBAGuess &guess );
};
