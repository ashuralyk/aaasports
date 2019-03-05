
#include <algorithm>
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

    void create( string &&param, name creator, asset value );

    void join( string &&param, name player, asset value );

    [[eosio::action]]
    void settle( uint64_t globalId, vector<uint64_t> followers );

    [[eosio::action]]
    void erase( string mid, name creator );

private:
    vector<string> split( string &view, char s );

    template < int tid, typename _Type >
    tuple<bool, ORACLE::NBAData> findOracleByType( _Type type ) {
        name o = requireOracleAccount();
        INDEX::NBAData nbaOracle( o, o.value );
        auto i = nbaOracle.end();
        if constexpr ( tid == 0 ) {
            i = find_if( nbaOracle.begin(), nbaOracle.end(), [&](auto &v) {
                return v.mid == type;
            });
        } else if constexpr ( tid == 1 ) {
            i = nbaOracle.find( type );
        }
        if ( i == nbaOracle.end() ) {
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

    tuple<bool, uint64_t> pushGuess( GUESS::NBAGuess &&guess );

    name getWinner( ORACLE::NBAData &oracle, GUESS::NBAGuess &guess );

    name requireOracleAccount() {
        auto c = _config.get_or_default( {} );
        if ( c.oracleContract == name() )
        {
            ROLLBACK( "please set oracle account in the config at first" );
        }
        return c.oracleContract;
    }

    template < int _FeeTy >
    float getFeeRate( name creator = name() )
    {
        auto c = _config.get_or_default( {} );
        if constexpr ( _FeeTy == 0 ) return c.failedFeeRate;
        if constexpr ( _FeeTy == 1 ) return c.winnerFeeRate;
        if constexpr ( _FeeTy == 2 ) {
            if ( creator != name() ) {
                auto quantity = count_if( _nbaGuess.begin(), _nbaGuess.end(), [&](auto &v) {
                    return v.creator == creator;
                });
                auto o = c.overCreate;
                if ( auto d = quantity - o.startCreate; d >= 0 ) {
                    return o.startRate + o.perRate * (d % o.perCreate);
                }
            }
        }
        return 0.f;
    }
};
