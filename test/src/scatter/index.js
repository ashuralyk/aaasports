
import ScatterJs from 'scatterjs-core'
import EosPlugin from 'scatterjs-plugin-eosjs'
import Eos from 'eosjs'
import Config from './config'

ScatterJs.plugins(new EosPlugin())

var networks = [
  // local 测试网
  {
    blockchain: 'eos',
    protocol: 'http',
    host: '192.168.100.169',
    port: 8888,
    chainId: 'cf057bbfb72640471fd910bcb67639c22df9f92470936cddc1ade0e2f2e7dc4f'
  },
  // jungle 测试网
  {
    blockchain: 'eos',
    protocol: 'http',
    host: 'jungle2.cryptolions.io',
    port: 80,
    chainId: 'e70aaab8997e1dfce58fbfac80cbbb8fecec7b99cf982a9444273cbc64c41473'
  },
  // eos 主网
  {
    blockchain: 'eos',
    protocol: 'https',
    host: 'api-mainnet.starteos.io',
    port: '443',
    chainId: 'aca376f206b8fc25a6ed44dbdc66547c36c6c33e3a119ffbeaef943642f0e906'
  }
]

var runtime = {
  connected: false,
  account: null,
  eos: null
}

function fail(msg) {
  return {
    result: false,
    error: msg
  }
}

function succeed(json = null) {
  if (json !== null) {
    return {
      result: true,
      error: '',
      json: json
    }
  } else {
    return {
      result: true,
      error: ''
    }
  }
}

export async function ConnectTo(networkId, appName) {
  if (! runtime.connected) {
    let con = await ScatterJs.scatter.connect(appName)
    if (! con) {
      return fail('please install scatter at first')
    }

    let network = networks[networkId]
    let login = await ScatterJs.scatter.login({
      accounts: [network]
    })

    runtime.account = login.accounts.find(a => a.blockchain === 'eos')
    if (null === runtime.account) {
      return fail('occur erros while getting scatter identity')
    }

    // 设置runtime
    runtime.connected = true
    runtime.eos = ScatterJs.scatter.eos(network, Eos)
  }

  return succeed()
}

// 获取用户登录账户名
export function GetLoginName() {
  return runtime.account ? runtime.account.name : ''
}

// 获取用户登录权限
export function GetLoginAuth() {
  return runtime.account ? runtime.account.authority : 'active'
}

// 获取用户登录公钥
export function GetPublicKey() {
  return runtime.account ? runtime.account.publicKey : ''
}

//============================================
// 和预言机账户交互
//============================================

export async function AddGame(params) {
  try {
    let data = {
      actions: [
        {
          account: Config.oracleAccount,
          name: 'add',
          authorization: [{
            actor: GetLoginName(),
            permission: GetLoginAuth()
          }],
          data: params
        }
      ]
    }
    let txJson = await runtime.eos.transaction(data)
    return succeed(txJson);
  } catch (e) {
    return fail(e)
  }
}

export async function CloseGame(params) {
  try {
    let data = {
      actions: [
        {
          account: Config.oracleAccount,
          name: 'close',
          authorization: [{
            actor: GetLoginName(),
            permission: GetLoginAuth()
          }],
          data: params
        }
      ]
    }
    let txJson = await runtime.eos.transaction(data)
    return succeed(txJson);
  } catch (e) {
    return fail(e)
  }
}

export async function EraseGame(params) {
  try {
    let data = {
      actions: [
        {
          account: Config.oracleAccount,
          name: 'erase',
          authorization: [{
            actor: GetLoginName(),
            permission: GetLoginAuth()
          }],
          data: params
        }
      ]
    }
    let txJson = await runtime.eos.transaction(data)
    return succeed(txJson);
  } catch (e) {
    return fail(e)
  }
}

export async function GetOracle() {
  try {
    let rows = await runtime.eos.getTableRows({
      json: true,
      code: Config.oracleAccount,
      scope: Config.oracleAccount,
      table: 'nbadata'
    })
    return succeed(rows)
  } catch (e) {
    return fail(e)
  }
}

//============================================
// 和NBA账户交互
//============================================

export async function Transfer(params) {
  try {
    let data = {
      actions: [
        {
          account: 'eosio.token',
          name: 'transfer',
          authorization: [{
            actor: GetLoginName(),
            permission: GetLoginAuth()
          }],
          data: params
        }
      ]
    }
    let txJson = await runtime.eos.transaction(data)
    return succeed(txJson);
  } catch (e) {
    return fail(e)
  }
}

export async function GetGuess() {
  try {
    let rows = await runtime.eos.getTableRows({
      json: true,
      code: Config.nbaAccount,
      scope: Config.nbaAccount,
      table: 'nbaguess'
    })
    return succeed(rows)
  } catch (e) {
    return fail(e)
  }
}
