<template>
  <div id="app">
    <p>login: {{ login }}</p>
    <button @click="addgame()"   type="button" style="width: 100px; height: 50px">预言机：加入一场游戏</button>
    <button @click="closegame()" type="button" style="width: 100px; height: 50px">预言机：关闭一场游戏</button>
    <button @click="erasegame()" type="button" style="width: 100px; height: 50px">预言机：删除一场游戏</button>
    <button @click="getoracle()" type="button" style="width: 100px; height: 50px">预言机：获得比赛列表</button>
    <br />
    <button @click="creategame()" type="button" style="width: 100px; height: 50px">NBA：创建一场游戏</button>
    <button @click="joingame()"   type="button" style="width: 100px; height: 50px">NBA：加入一场游戏</button>
    <button @click="getguess()"   type="button" style="width: 100px; height: 50px">NBA：获得竞猜列表</button>
    <p>{{ msg }}</p>
    <button @click="msg=''" type="button" style="width: 100px; height: 50px">清空</button>
  </div>
</template>

<script>
import {ConnectTo, GetLoginName, AddGame, CloseGame, EraseGame, GetOracle, Transfer, GetGuess} from './scatter'
import Config from './scatter/config'

export default {
  name: 'app',
  data () {
    return {
      login: '',
      msg: ''
    }
  },
  methods: {
    //----------------------------------------------
    // 预言机合约相关
    //----------------------------------------------
    async addgame() {
      let con = await ConnectTo(1, 'AAASports')
      if (! con.result) {
          this.msg = JSON.stringify(con.error)
      } else {
        let feedback = await AddGame({
          mid:       'VVNvsOVN',
          homeTeam:  'VVN',
          awayTeam:  'OVN',
          startTime: Math.floor(Date.now() / 1000)
        })
        if (! feedback.result) {
            this.msg = JSON.stringify(feedback.error)
        } else {
            this.msg = JSON.stringify(feedback.json)
        }
      }
      this.login = GetLoginName()
    },
    async closegame() {
      let con = await ConnectTo(1, 'AAASports')
      if (! con.result) {
          this.msg = JSON.stringify(con.error)
      } else {
        let feedback = await CloseGame({
          mid:      'VVNvsOVN',
          homeScore: 75,
          awayScore: 56,
          endTime:   Math.floor(Date.now() / 1000)
        })
        if (! feedback.result) {
            this.msg = JSON.stringify(feedback.error)
        } else {
            this.msg = JSON.stringify(feedback.json)
        }
      }
      this.login = GetLoginName()
    },
    async erasegame() {
      let con = await ConnectTo(1, 'AAASports')
      if (! con.result) {
          this.msg = JSON.stringify(con.error)
      } else {
        let feedback = await EraseGame({
          mid: 'VVNvsOVN'
        })
        if (! feedback.result) {
            this.msg = JSON.stringify(feedback.error)
        } else {
            this.msg = JSON.stringify(feedback.json)
        }
      }
      this.login = GetLoginName()
    },
    async getoracle() {
      let con = await ConnectTo(1, 'AAASports')
      if (! con.result) {
          this.msg = JSON.stringify(con.error)
      } else {
        let feedback = await GetOracle()
        if (! feedback.result) {
            this.msg = JSON.stringify(feedback.error)
        } else {
            this.msg = JSON.stringify(feedback.json)
        }
      }
      this.login = GetLoginName()
    },
    //----------------------------------------------
    // NBA合约相关
    //----------------------------------------------
    async creategame() {
      let con = await ConnectTo(1, 'AAASports')
      if (! con.result) {
          this.msg = JSON.stringify(con.error)
      } else {
        let feedback = await Transfer({
          from:     GetLoginName(),
          to:       Config.nbaAccount,
          quantity: '1.0000 EOS',
          memo:     'create|VVNvsOVN|0|1|255|'
        })
        if (! feedback.result) {
            this.msg = JSON.stringify(feedback.error)
        } else {
            this.msg = JSON.stringify(feedback.json)
        }
      }
      this.login = GetLoginName()
    },
    async joingame(creator) {
      let con = await ConnectTo(1, 'AAASports')
      if (! con.result) {
          this.msg = JSON.stringify(con.error)
      } else {
        let feedback = await Transfer({
          from:     GetLoginName(),
          to:       Config.nbaAccount,
          quantity: '1.0000 EOS',
          memo:     'join|VVNvsOVN|' + creator + '|'
        })
        if (! feedback.result) {
            this.msg = JSON.stringify(feedback.error)
        } else {
            this.msg = JSON.stringify(feedback.json)
        }
      }
      this.login = GetLoginName()
    },
    async getguess() {
      let con = await ConnectTo(1, 'AAASports')
      if (! con.result) {
          this.msg = JSON.stringify(con.error)
      } else {
        let feedback = await GetGuess()
        if (! feedback.result) {
            this.msg = JSON.stringify(feedback.error)
        } else {
            this.msg = JSON.stringify(feedback.json)
        }
      }
      this.login = GetLoginName()
    }
  }
}
</script>
