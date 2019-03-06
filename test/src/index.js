
import Scatter from './scatter'

Scatter.ConnectTo(0, 'AAASports').then(con => {
    if (! con.result) {
        console.error(con.error)
    } else {
        Scatter.AddGame({
            mid:       'VVNvsOVN',
            homeTeam:  'VVN',
            awayTeam:  'OVN',
            startTime: Math.floor(Date.now() / 1000)
        }).then(feedback => {
            if (! feedback.result) {
                console.error(feedback.error)
            } else {
                console.log(feedback.json)
            }
        })
    }
})
