
#include <eosiolib/eosio.hpp>
#include <eosiolib/asset.hpp>
#include <eosiolib/singleton.hpp>
#include <string>
#include <set>
#include <vector>
#include <tuple>

using namespace eosio;
using namespace std;

inline void ROLLBACK( const char *m )
{
    eosio_assert( false, m );
}

inline void ROLLBACK( string &&m )
{
    eosio_assert( false, m.c_str() );
}

namespace ORACLE
{

struct [[eosio::table("authconfig"), eosio::contract("sportsoracle")]] OracleConfig
{
    set<name> authorization;
    name      guessContract;
};

struct [[eosio::table("nbadata"), eosio::contract("sportsoracle")]] NBAData
{
    uint64_t globalId;          // 全局唯一ID
    string   mid;               // 比赛ID（来自腾讯）
    string   homeTeam;          // 主队ID
    string   awayTeam;          // 客队ID
    uint32_t startTime;         // 开始时间（UNIX时间戳）
    uint8_t  homeScore;         // 主队分数
    uint8_t  awayScore;         // 客队分数
    uint32_t endTime;           // 结束时间（UINX时间戳）
    string   scoreDetail;       // 分数详情（待定）

    uint64_t primary_key() const {
        return globalId;
    }
};

struct [[eosio::table("nbasubscribe"), eosio::contract("sportsoracle")]] NBASubscribe
{
    uint64_t      globalId;      // 全局ID
    string        mid;           // 比赛ID
    set<uint64_t> followers;     // 订阅者

    uint64_t primary_key() const {
        return globalId;
    }
};

}

namespace GUESS
{

struct [[eosio::table("nbaconfig"), eosio::contract("nbasportsaaa")]] NBAConfig
{
    float failedFeeRate;
    float winnerFeeRate;
    name  oracleContract;

    struct OverCreate
    {
        float   startRate;
        float   perRate;
        uint8_t startCreate;
        uint8_t perCreate;
        asset   lowerFund;
        asset   stepFund;
    }
    overCreate;
};

struct [[eosio::table("nbaguess"), eosio::contract("nbasportsaaa")]] NBAGuess
{
    uint64_t globalId;       // 全局ID
    string   mid;            // 比赛ID
    uint8_t  type;           // 比赛类型
    uint8_t  bet;            // 竞猜方向
    float    score;          // 竞猜值(根据类型type的不同这个值的含义也不同)
    name     creator;        // 竞猜创建者
    name     player;         // 竞猜另一方玩家
    asset    tokenAmount;    // 竞猜下注金额
    name     winner;         // 竞猜胜利方

    uint64_t primary_key() const {
        return globalId;
    }
};

}

namespace INDEX
{

typedef multi_index<"nbadata"_n, ORACLE::NBAData>           NBAData;
typedef multi_index<"nbasubscribe"_n, ORACLE::NBASubscribe> NBASubscribe;
typedef multi_index<"nbaguess"_n, GUESS::NBAGuess>          NBAGuess;

}

namespace SINGLE
{

typedef singleton<"authconfig"_n, ORACLE::OracleConfig> Config;
typedef singleton<"nbaconfig"_n, GUESS::NBAConfig>      NBAConfig;

}
