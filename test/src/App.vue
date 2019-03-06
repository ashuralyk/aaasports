<template>
  <div id="app">
    <button @click="addgame()" type="button" style="width: 100px; height: 50px">加入一场游戏</button>
  </div>
</template>

<script>
import {ConnectTo, AddGame} from './scatter'

export default {
  name: 'app',
  data () {
    return {
      msg: 'Welcome to Your Vue.js App'
    }
  },
  methods: {
    async addgame() {
      let con = await ConnectTo(0, 'AAASports')
      if (! con.result) {
          console.error(con.error)
      } else {
        let feedback = await AddGame({
          mid:       'VVNvsOVN',
          homeTeam:  'VVN',
          awayTeam:  'OVN',
          startTime: Math.floor(Date.now() / 1000)
        })
        if (! feedback.result) {
            console.error(feedback.error)
        } else {
            console.log(feedback.json)
        }
      }
    }
  }
}
</script>

<style>
</style>
